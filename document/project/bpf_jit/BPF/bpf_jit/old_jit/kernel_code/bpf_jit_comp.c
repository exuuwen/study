/* bpf_jit_comp.c : BPF JIT compiler
 *
 * Copyright (C) 2011 Eric Dumazet (eric.dumazet@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */
#include <linux/moduleloader.h>
#include <asm/cacheflush.h>
#include <linux/netdevice.h>
#include <linux/filter.h>

/*
 * Conventions :
 *  EAX : BPF A accumulator
 *  EBX : BPF X accumulator
 *  RDI : pointer to skb   (first argument given to JIT function)
 *  RBP : frame pointer (even if CONFIG_FRAME_POINTER=n)
 *  ECX,EDX,ESI : scratch registers
 *  r9d : skb->len - skb->data_len (headlen)
 *  r8  : skb->data
 * -8(RBP) : saved RBX value
 * -16(RBP)..-80(RBP) : BPF_MEMWORDS values
 */
int bpf_jit_enable __read_mostly;

/*
 * assembly code in arch/x86/net/bpf_jit.S
 */
extern u8 sk_load_word[], sk_load_half[], sk_load_byte[], sk_load_byte_msh[];
//extern u8 sk_load_word_ind[], sk_load_half_ind[], sk_load_byte_ind[];

static inline u8 *emit_code(u8 *ptr, u32 bytes, unsigned int len)
{
	if (len == 1)
		*ptr = bytes;
	else if (len == 2)
		*(u16 *)ptr = bytes;
	else {
		*(u32 *)ptr = bytes;
		barrier();
	}
	return ptr + len;
}

#define EMIT(bytes, len)	do { prog = emit_code(prog, bytes, len); } while (0)

#define EMIT1(b1)		EMIT(b1, 1)
#define EMIT2(b1, b2)		EMIT((b1) + ((b2) << 8), 2)
#define EMIT3(b1, b2, b3)	EMIT((b1) + ((b2) << 8) + ((b3) << 16), 3)
#define EMIT4(b1, b2, b3, b4)   EMIT((b1) + ((b2) << 8) + ((b3) << 16) + ((b4) << 24), 4)
#define EMIT1_off32(b1, off)	do { EMIT1(b1); EMIT(off, 4);} while (0)

#define CLEAR_A() EMIT2(0x31, 0xc0) /* xor %eax,%eax */
#define CLEAR_X() EMIT3(0x4d, 0x31, 0xc9) /* xor %r9,%r9 */

static inline bool is_imm8(int value)
{
	return value <= 127 && value >= -128;
}

static inline bool is_near(int offset)
{
	return offset <= 127 && offset >= -128;
}

#define EMIT_JMP(offset)						\
do {									\
	if (offset) {							\
		if (is_near(offset))					\
			EMIT2(0xeb, offset); /* jmp .+off8 */		\
		else							\
			EMIT1_off32(0xe9, offset); /* jmp .+off32 */	\
	}								\
} while (0)

/* list of x86 cond jumps opcodes (. + s8)
 * Add 0x10 (and an extra 0x0f) to generate far jumps (. + s32)
 */
#define X86_JB  0x72
#define X86_JAE 0x73
#define X86_JE  0x74
#define X86_JNE 0x75
#define X86_JBE 0x76
#define X86_JA  0x77
#define X86_JS  0x78
#define X86_JLE 0x7e
#define X86_JG  0x7f

#define EMIT_COND_JMP(op, offset)				\
do {								\
	if (is_near(offset))					\
		EMIT2(op, offset); /* jxx .+off8 */		\
	else {							\
		EMIT2(0x0f, op + 0x10);				\
		EMIT(offset, 4); /* jxx .+off32 */		\
	}							\
} while (0)

#define COND_SEL(CODE, TOP, FOP)	\
	case CODE:			\
		t_op = TOP;		\
		f_op = FOP;		\
		goto cond_branch

#define COND_JMP_SEL(CODE, TOP, FOP)	\
		case CODE:			\
			t_op = TOP; 	\
			f_op = FOP; 	\
			goto cond_jmp_branch


#define SEEN_DATAREF 1 /* might call external helpers */
#define SEEN_XREG    2 /* ebx is used */
#define SEEN_MEM     4 /* use mem[] for temporary storage */

static inline void bpf_flush_icache(void *start, void *end)
{
	mm_segment_t old_fs = get_fs();

	set_fs(KERNEL_DS);
	smp_wmb();
	flush_icache_range((unsigned long)start, (unsigned long)end);
	set_fs(old_fs);
}

int new_bpf_jit_compile(struct sk_filter *fp, u8* code, u32 len)
{
	u8* image = NULL;

	if (!bpf_jit_enable)
		return -EPERM;
	image = module_alloc(max_t(unsigned int, len, sizeof(struct work_struct)));
	if (!image)
		return -ENOMEM;
	memcpy(image, code, len);

	if (bpf_jit_enable > 1)
	{
		pr_err("proglen=%u  image=%p\n", len, image);
		print_hex_dump(KERN_ERR, "JIT code: ", DUMP_PREFIX_ADDRESS,
				       16, 1, image, len, false);
	}

	bpf_flush_icache(image, image + len);	
	fp->bpf_func = (void*)image;

	return 0;
}


void bpf_jit_compile(struct sk_filter *fp/*struct sock_fprog* fprog, unsigned int *len*/)
{
	u8 temp[128];
	u8 *prog;
	unsigned int proglen, oldproglen = 0;
	int ilen, i;
	int t_offset, f_offset;
	u8 t_op, f_op, seen = 0, pass;
	u8 *image = NULL;
	u8 *func;
	int pc_ret0 = -1; /* bpf index of first RET #0 instruction (if any) */
	unsigned int cleanup_addr; /* epilogue code offset */
	unsigned int *addrs;
	const struct sock_filter *filter = fp->insns;
	int flen = fp->len;
	//unsigned int max_offset;
	unsigned char fast_jump_off[flen];

	unsigned char fast_path_len[flen];
	unsigned char slow_path_len[flen];
	
	if (!bpf_jit_enable)
		return;

	addrs = kmalloc(flen * sizeof(*addrs), GFP_KERNEL);
	if (addrs == NULL)
		return;

	/* Before first pass, make a rough estimation of addrs[]
	 * each bpf instruction is translated to less than 64 bytes
	 */
	for (proglen = 0, i = 0; i < flen; i++) {
		proglen += 128;
		addrs[i] = proglen;
	}
	cleanup_addr = proglen; /* epilogue address */

	for (pass = 0; pass < 10; pass++) {
		/* no prologue/epilogue for trivial filters (RET something) */
		proglen = 0;
		prog = temp;
		//max_offset = 0;
		

		if (seen) {
			EMIT4(0x55, 0x48, 0x89, 0xe5); /* push %rbp; mov %rsp,%rbp */
			EMIT4(0x48, 0x83, 0xec, 96);	/* subq  $96,%rsp	*/
			/* note : must save %rbx in case bpf_error is hit */
			//if (seen & (SEEN_XREG | SEEN_DATAREF))
				//EMIT4(0x48, 0x89, 0x5d, 0xf8); /* mov %rbx, -8(%rbp) */
			if (seen & SEEN_XREG)
				CLEAR_X(); /* make sure we dont leek kernel memory */

			/*
			 * If this filter needs to access skb data,
			 * loads r9 and r8 with :
			 *  r9 = skb->len - skb->data_len
			 *  r8 = skb->data
			 */
		
			/*if (seen & SEEN_DATAREF) {
				if (offsetof(struct sk_buff, len) <= 127)
					//mov    off8(%rdi),%r9d 
					EMIT4(0x44, 0x8b, 0x4f, offsetof(struct sk_buff, len));
				else {
					// mov    off32(%rdi),%r9d 
					EMIT3(0x44, 0x8b, 0x8f);
					EMIT(offsetof(struct sk_buff, len), 4);
				}
				if (is_imm8(offsetof(struct sk_buff, data_len)))
					// sub    off8(%rdi),%r9d 
					EMIT4(0x44, 0x2b, 0x4f, offsetof(struct sk_buff, data_len));
				else {
					EMIT3(0x44, 0x2b, 0x8f);
					EMIT(offsetof(struct sk_buff, data_len), 4);
				}

				if (is_imm8(offsetof(struct sk_buff, data)))
					// mov off8(%rdi),%r8 
					EMIT4(0x4c, 0x8b, 0x47, offsetof(struct sk_buff, data));
				else {
					// mov off32(%rdi),%r8 
					EMIT3(0x4c, 0x8b, 0x87);
					EMIT(offsetof(struct sk_buff, data), 4);
				}
			}*/
		}

		switch (filter[0].code) {
		case BPF_S_RET_K:
		case BPF_S_LD_W_LEN:
		case BPF_S_ANC_PROTOCOL:
		case BPF_S_ANC_IFINDEX:
		case BPF_S_ANC_MARK:
		case BPF_S_ANC_RXHASH:
		case BPF_S_ANC_CPU:
		case BPF_S_ANC_QUEUE:
		case BPF_S_LD_W_ABS:
		case BPF_S_LD_H_ABS:
		case BPF_S_LD_B_ABS:
			/* first instruction sets A register (or is RET 'constant') */
			break;
		default:
			/* make sure we dont leak kernel information to user */
			CLEAR_A(); /* A = 0 */
		}

		for (i = 0; i < flen; i++) {
			unsigned int K = filter[i].k;


			switch (filter[i].code) {
			case BPF_S_ALU_ADD_X: /* A += X; */
				seen |= SEEN_XREG;
				//EMIT2(0x01, 0xd8);		/* add %ebx,%eax */
				EMIT3(0x44, 0x01, 0xc8);		/* add %r9d,%eax */
				break;
			case BPF_S_ALU_ADD_K: /* A += K; */
				if (!K)
					break;
				if (is_imm8(K))
					EMIT3(0x83, 0xc0, K);	/* add imm8,%eax */
				else
					EMIT1_off32(0x05, K);	/* add imm32,%eax */
				break;
			case BPF_S_ALU_SUB_X: /* A -= X; */
				seen |= SEEN_XREG;
				//EMIT2(0x29, 0xd8);		/* sub    %ebx,%eax */
				EMIT3(0x44, 0x29, 0xc8);		/* sub   %r9d,%eax */
				break;
			case BPF_S_ALU_SUB_K: /* A -= K */
				if (!K)
					break;
				if (is_imm8(K))
					EMIT3(0x83, 0xe8, K); /* sub imm8,%eax */
				else
					EMIT1_off32(0x2d, K); /* sub imm32,%eax */
				break;
			case BPF_S_ALU_MUL_X: /* A *= X; */
				seen |= SEEN_XREG;
				//EMIT3(0x0f, 0xaf, 0xc3);	/* imul %ebx,%eax */
				EMIT4(0x41, 0x0f, 0xaf, 0xc1);  /* imul %r9d,%eax */
				break;
			case BPF_S_ALU_MUL_K: /* A *= K */
				if (is_imm8(K))
					EMIT3(0x6b, 0xc0, K); /* imul imm8,%eax,%eax */
				else {
					EMIT2(0x69, 0xc0);		/* imul imm32,%eax */
					EMIT(K, 4);
				}
				break;
			case BPF_S_ALU_DIV_X: /* A /= X; */
				seen |= SEEN_XREG;
				//EMIT2(0x85, 0xdb);	/* test %ebx,%ebx */
				EMIT3(0x45, 0x85, 0xc9); /* test %r9d,%r9d */
				if (pc_ret0 != -1)
					EMIT_COND_JMP(X86_JE, addrs[pc_ret0] - (addrs[i] - 4));
				else {
					EMIT_COND_JMP(X86_JNE, 2 + 5);
					CLEAR_A();
					EMIT1_off32(0xe9, cleanup_addr - (addrs[i] - 4)); /* jmp .+off32 */
				}
				//EMIT4(0x31, 0xd2, 0xf7, 0xf3); /* xor %edx,%edx; div %ebx */
				EMIT2(0x31, 0xd2); /* xor %edx,%edx*/
				EMIT3(0x41, 0xf7, 0xf1); /* div %r9d */
				break;
			case BPF_S_ALU_DIV_K: /* A = reciprocal_divide(A, K); */
				EMIT3(0x48, 0x69, 0xc0); /* imul imm32,%rax,%rax */
				EMIT(K, 4);
				EMIT4(0x48, 0xc1, 0xe8, 0x20); /* shr $0x20,%rax */
				break;
			case BPF_S_ALU_AND_X:
				seen |= SEEN_XREG;
				//EMIT2(0x21, 0xd8);		/* and %ebx,%eax */
				EMIT3(0x44, 0x21, 0xc8);	/* and %r9d,%eax */
				break;
			case BPF_S_ALU_AND_K:
				if (K >= 0xFFFFFF00) {
					EMIT2(0x24, K & 0xFF); /* and imm8,%al */
				} else if (K >= 0xFFFF0000) {
					EMIT2(0x66, 0x25);	/* and imm16,%ax */
					EMIT2(K, 2);
				} else {
					EMIT1_off32(0x25, K);	/* and imm32,%eax */
				}
				break;
			case BPF_S_ALU_OR_X:
				seen |= SEEN_XREG;
				//EMIT2(0x09, 0xd8);		/* or %ebx,%eax */
				EMIT3(0x44, 0x09, 0xc8);	/* or %r9d,%eax */
				break;
			case BPF_S_ALU_OR_K:
				if (is_imm8(K))
					EMIT3(0x83, 0xc8, K); /* or imm8,%eax */
				else
					EMIT1_off32(0x0d, K);	/* or imm32,%eax */
				break;
			case BPF_S_ALU_LSH_X: /* A <<= X; */
				seen |= SEEN_XREG;
				//EMIT4(0x89, 0xd9, 0xd3, 0xe0);	/* mov %ebx,%ecx; shl %cl,%eax */
				EMIT3(0x44, 0x89, 0xc9);
				EMIT2(0xd3, 0xe0);			/* mov %r9d,%ecx; shl %cl,%eax */
				break;
			case BPF_S_ALU_LSH_K:
				if (K == 0)
					break;
				else if (K == 1)
					EMIT2(0xd1, 0xe0); /* shl %eax */
				else
					EMIT3(0xc1, 0xe0, K);
				break;
			case BPF_S_ALU_RSH_X: /* A >>= X; */
				seen |= SEEN_XREG;
				//EMIT4(0x89, 0xd9, 0xd3, 0xe8);	/* mov %ebx,%ecx; shr %cl,%eax */
				EMIT3(0x44, 0x89, 0xc9);
				EMIT2(0xd3, 0xe8);			/* mov %r9d,%ecx; shr %cl,%eax */
				break;
			case BPF_S_ALU_RSH_K: /* A >>= K; */
				if (K == 0)
					break;
				else if (K == 1)
					EMIT2(0xd1, 0xe8); /* shr %eax */
				else
					EMIT3(0xc1, 0xe8, K);
				break;
			case BPF_S_ALU_NEG:
				EMIT2(0xf7, 0xd8);		/* neg %eax */
				break;
			case BPF_S_RET_K:
				if (!K) {
					if (pc_ret0 == -1)
						pc_ret0 = i;
					CLEAR_A();
				} else {
					EMIT1_off32(0xb8, K);	/* mov $imm32,%eax */
				}
				/* fallinto */
			case BPF_S_RET_A:
				if (seen) {
					if (i != flen - 1) {
						EMIT_JMP(cleanup_addr - addrs[i]);
						break;
					}
					//if (seen & SEEN_XREG)
						//EMIT4(0x48, 0x8b, 0x5d, 0xf8);  /* mov  -8(%rbp),%rbx */
					EMIT1(0xc9);		/* leaveq */
				}
				EMIT1(0xc3);		/* ret */
				break;
			case BPF_S_MISC_TAX: /* X = A */
				seen |= SEEN_XREG;
				//EMIT2(0x89, 0xc3);	/* mov    %eax,%ebx */
				EMIT3(0x44, 0x89, 0xc1); /* mov    %eax,%r9d */
				break;
			case BPF_S_MISC_TXA: /* A = X */
				seen |= SEEN_XREG;
				//EMIT2(0x89, 0xd8);	/* mov    %ebx,%eax */
				EMIT3(0x44, 0x89, 0xc8); /* mov    %r9d,%eax */
				break;
			case BPF_S_LD_IMM: /* A = K */
				if (!K)
					CLEAR_A();
				else
					EMIT1_off32(0xb8, K); /* mov $imm32,%eax */
				break;
			case BPF_S_LDX_IMM: /* X = K */
				seen |= SEEN_XREG;
				if (!K)
					CLEAR_X();
				else
				{
					//EMIT1_off32(0xbb, K); /* mov $imm32,%ebx */
					EMIT1(0x41);
					EMIT1_off32(0xb9, K); /* mov $imm32,%r9d */				
				}
				break;
			case BPF_S_LD_MEM: /* A = mem[K] : mov off8(%rbp),%eax */
				seen |= SEEN_MEM;
				EMIT3(0x8b, 0x45, 0xf0 - K*4);
				break;
			case BPF_S_LDX_MEM: /* X = mem[K] : mov off8(%rbp),%ebx */
				seen |= SEEN_XREG | SEEN_MEM;
				//EMIT3(0x8b, 0x5d, 0xf0 - K*4);
				EMIT4(0x44, 0x8b, 0x4d, 0xf0 - K*4); /* X = mem[K] : mov off8(%rbp),%r9d */
				break;
			case BPF_S_ST: /* mem[K] = A : mov %eax,off8(%rbp) */
				seen |= SEEN_MEM;
				EMIT3(0x89, 0x45, 0xf0 - K*4);
				break;
			case BPF_S_STX: /* mem[K] = X : mov %ebx,off8(%rbp) */
				seen |= SEEN_XREG | SEEN_MEM;
				//EMIT3(0x89, 0x5d, 0xf0 - K*4);
				EMIT4(0x44, 0x89, 0x4d, 0xf0 - K*4); /* mem[K] = X : mov %r9d,off8(%rbp) */
				break;
			case BPF_S_LD_W_LEN: /*	A = skb->len; */
				BUILD_BUG_ON(FIELD_SIZEOF(struct sk_buff, len) != 4);
				if (is_imm8(offsetof(struct sk_buff, len)))
					/* mov    off8(%rdi),%eax */
					EMIT3(0x8b, 0x47, offsetof(struct sk_buff, len));
				else {
					EMIT2(0x8b, 0x87);
					EMIT(offsetof(struct sk_buff, len), 4);
				}
				break;
			case BPF_S_LDX_W_LEN: /* X = skb->len; */
				seen |= SEEN_XREG;
				if (is_imm8(offsetof(struct sk_buff, len)))
					/* mov off8(%rdi),%ebx */
					EMIT3(0x8b, 0x5f, offsetof(struct sk_buff, len));
				else {
					EMIT2(0x8b, 0x9f);
					EMIT(offsetof(struct sk_buff, len), 4);
				}
				break;
			case BPF_S_ANC_PROTOCOL: /* A = ntohs(skb->protocol); */
				BUILD_BUG_ON(FIELD_SIZEOF(struct sk_buff, protocol) != 2);
				if (is_imm8(offsetof(struct sk_buff, protocol))) {
					/* movzwl off8(%rdi),%eax */
					EMIT4(0x0f, 0xb7, 0x47, offsetof(struct sk_buff, protocol));
				} else {
					EMIT3(0x0f, 0xb7, 0x87); /* movzwl off32(%rdi),%eax */
					EMIT(offsetof(struct sk_buff, protocol), 4);
				}
				EMIT2(0x86, 0xc4); /* ntohs() : xchg   %al,%ah */
				break;
			case BPF_S_ANC_IFINDEX:
				if (is_imm8(offsetof(struct sk_buff, dev))) {
					/* movq off8(%rdi),%rax */
					EMIT4(0x48, 0x8b, 0x47, offsetof(struct sk_buff, dev));
				} else {
					EMIT3(0x48, 0x8b, 0x87); /* movq off32(%rdi),%rax */
					EMIT(offsetof(struct sk_buff, dev), 4);
				}
				EMIT3(0x48, 0x85, 0xc0);	/* test %rax,%rax */
				EMIT_COND_JMP(X86_JE, cleanup_addr - (addrs[i] - 6));
				BUILD_BUG_ON(FIELD_SIZEOF(struct net_device, ifindex) != 4);
				EMIT2(0x8b, 0x80);	/* mov off32(%rax),%eax */
				EMIT(offsetof(struct net_device, ifindex), 4);
				break;
			case BPF_S_ANC_MARK:
				BUILD_BUG_ON(FIELD_SIZEOF(struct sk_buff, mark) != 4);
				if (is_imm8(offsetof(struct sk_buff, mark))) {
					/* mov off8(%rdi),%eax */
					EMIT3(0x8b, 0x47, offsetof(struct sk_buff, mark));
				} else {
					EMIT2(0x8b, 0x87);
					EMIT(offsetof(struct sk_buff, mark), 4);
				}
				break;
			case BPF_S_ANC_RXHASH:
				BUILD_BUG_ON(FIELD_SIZEOF(struct sk_buff, rxhash) != 4);
				if (is_imm8(offsetof(struct sk_buff, rxhash))) {
					/* mov off8(%rdi),%eax */
					EMIT3(0x8b, 0x47, offsetof(struct sk_buff, rxhash));
				} else {
					EMIT2(0x8b, 0x87);
					EMIT(offsetof(struct sk_buff, rxhash), 4);
				}
				break;
			case BPF_S_ANC_QUEUE:
				BUILD_BUG_ON(FIELD_SIZEOF(struct sk_buff, queue_mapping) != 2);
				if (is_imm8(offsetof(struct sk_buff, queue_mapping))) {
					/* movzwl off8(%rdi),%eax */
					EMIT4(0x0f, 0xb7, 0x47, offsetof(struct sk_buff, queue_mapping));
				} else {
					EMIT3(0x0f, 0xb7, 0x87); /* movzwl off32(%rdi),%eax */
					EMIT(offsetof(struct sk_buff, queue_mapping), 4);
				}
				break;
			case BPF_S_ANC_CPU:
#ifdef CONFIG_SMP
				EMIT4(0x65, 0x8b, 0x04, 0x25); /* mov %gs:off32,%eax */
				EMIT((u32)(unsigned long)&cpu_number, 4); /* A = smp_processor_id(); */
#else
				CLEAR_A();
#endif
				break;
			case BPF_S_LD_W_ABS: 
				// JITd code performance enhancement is performed on all the LD load 
				// instructions. LD instruction is used to load memory into register, which
				// is a performance bottleneck.
				// First is to inline the callq, and then consolidate two mov instructions
				// into one mov instruction. By these change, JITd code performance is 
				// doubled. The following LD_ instructions change are same with this one.
				
				//func = sk_load_word;
//common_load:			
				seen |= SEEN_DATAREF;
				if ((int)K < 0)
					goto out;
				//t_offset = func - (image + addrs[i]);
				//EMIT1_off32(0xbe, K); /* mov imm32,%esi */
				//EMIT1_off32(0xe8, t_offset); /* call */

				// inline the call above
				//EMIT1_off32(0xbe, K); /* mov imm32,%esi */
				//EMIT4(0x41, 0x8b, 0x04, 0x30);  /*mov    (%r8,%rsi,1),%eax*/

				// Consolidate the above two mov into one
				
				//EMIT3(0x41, 0x8b, 0x80);
				//EMIT4(K, 0x00, 0x00, 0x00);  	/*mov   K(%r8),%eax*/ 
				
				////EMIT3(0x48, 0xc7, 0xc1);/*mov K %rcx*/ //hlen - offset
				////EMIT(K, 4);
				////EMIT4(0x48, 0x83, 0xc1, 0x03);  /*add    $0x3,%rcx*/
				////EMIT3(0x48, 0x39, 0xca); /*cmp    %rcx,%rdx*/
				//if(K > max_offset)
				//{
					EMIT2(0x8d, 0x8a); /*lea    -k(%rdx),%ecx*/
					EMIT((~K) + 1, 4);
					EMIT3(0x83, 0xf9, 0x03); /*cmp    $0x3,%ecx*/
				
					EMIT_COND_JMP(X86_JLE, fast_path_len[i]);	/*jle  xx*/

					//max_offset = K;				
				//}
				if((filter[i+1].code == BPF_S_JMP_JGT_K) || (filter[i+1].code == BPF_S_JMP_JGE_K) || (filter[i+1].code == BPF_S_JMP_JEQ_K))
					goto bpf_fast_path;
				
				EMIT2(0x8b, 0x86);
				EMIT(K, 4);  	/*mov   K(%rsi),%eax*/
				EMIT2(0x0f, 0xc8);		/*bswap  %eax*/
				EMIT_JMP(slow_path_len[i]);
				fast_path_len[i] = is_near(slow_path_len[i])? 10 : 14;

				goto slowpath_load;

				break;
			case BPF_S_LD_H_ABS: 
				//func = sk_load_half;
				//goto common_load;
				
				seen |= SEEN_DATAREF;
				if ((int)K < 0)
					goto out;
				
				//EMIT1_off32(0xbe, K); /* mov imm32,%esi */		
				//EMIT1(0x41);			
				//EMIT4(0x0f, 0xb7, 0x04, 0x30);/*movzwl(%r8,%rsi),%eax*/

				// Consolidate the above two mov into one
				
				//EMIT1(0x41);
				//EMIT4(0x0f, 0xb7, 0x40, K); /*movzwl K(%r8),%eax*/

				////EMIT3(0x48, 0xc7, 0xc1);/*mov K %rcx*/ //hlen - offset
				////EMIT(K, 4);
				////EMIT4(0x48, 0x83, 0xc1, 0x01);  /*add    $0x1,%rcx*/
				////EMIT3(0x48, 0x39, 0xca); /*cmp    %rcx,%rdx*/
				
				//if(K > max_offset)
				//{
					EMIT2(0x8d, 0x8a); /*lea    -k(%rdx),%ecx*/
					EMIT((~K) + 1, 4);
					EMIT3(0x83, 0xf9, 0x01); /*cmp    $0x1,%ecx*/
					//printf("h_offset:0x%x\n", fast_path_len);

					EMIT_COND_JMP(X86_JLE, fast_path_len[i]);	/*jle  xx*/

					
					//max_offset = K;				
				//}
				if((filter[i+1].code == BPF_S_JMP_JGT_K) || (filter[i+1].code == BPF_S_JMP_JGE_K) || (filter[i+1].code == BPF_S_JMP_JEQ_K))
					goto bpf_fast_path;

				EMIT3(0x0f, 0xb7, 0x86);
				EMIT(K, 4); /*movzwl K(%rsi),%eax*/
				EMIT4(0x66, 0xc1, 0xc0, 0x08);/*# ntohs()*/

				EMIT_JMP(slow_path_len[i]);
				fast_path_len[i] = is_near(slow_path_len[i])? 13 : 17;

				goto slowpath_load;

				break;
			case BPF_S_LD_B_ABS:
				//func = sk_load_byte;
				//goto common_load;

				seen |= SEEN_DATAREF;
				if ((int)K < 0)
					goto out;
				//EMIT1_off32(0xbe, K); /* mov imm32,%esi */
				//EMIT1(0x41);			
				//EMIT4(0x0f, 0xb6, 0x04, 0x30);/*movzbl (%r8,%rsi,1),%eax*/
				
				// Consolidate the above two mov into one
				
				//EMIT1(0x41);			
				//EMIT4(0x0f, 0xb6, 0x40, K);/*movzbl K(%r8),%eax*/

				
				//if(K > max_offset)
				//{
					EMIT3(0x48, 0x81, 0xfa); /*cmp   K,%rdx*/ //hlen - offset
					EMIT(K, 4);
					
					EMIT_COND_JMP(X86_JLE, fast_path_len[i]);	/*jle  xx*/
				
					
					//max_offset = K;				
				//}
				if((filter[i+1].code == BPF_S_JMP_JGT_K) || (filter[i+1].code == BPF_S_JMP_JGE_K) || (filter[i+1].code == BPF_S_JMP_JEQ_K))
					goto bpf_fast_path;

				EMIT3(0x0f, 0xb6, 0x86);
				EMIT(K, 4); /*movzbl K(%rsi),%eax*/

				EMIT_JMP(slow_path_len[i]);
				fast_path_len[i] = is_near(slow_path_len[i])? 9 : 13;
				goto slowpath_load;
				
				break;
			case BPF_S_LDX_B_MSH:
				if ((int)K < 0) {
					if (pc_ret0 != -1) {
						EMIT_JMP(addrs[pc_ret0] - addrs[i]);
						break;
					}
					CLEAR_A();
					EMIT_JMP(cleanup_addr - addrs[i]);
					break;
				}
				seen |= SEEN_DATAREF | SEEN_XREG;
				//t_offset = sk_load_byte_msh - (image + addrs[i]);
				//EMIT1_off32(0xbe, K);	/* mov imm32,%esi */
				//EMIT1_off32(0xe8, t_offset); /* call sk_load_byte_msh */

				// inline the above callq
				//EMIT1_off32(0xbe, K);	/* mov imm32,%esi */
				//EMIT1(0x41); 
				//EMIT4(0x0f, 0xb6, 0x1c, 0x30); /*movzbl (%r8,%rsi,1),%ebx*/

				// Consolidate the above two mov into one
			
				//EMIT1(0x41); 
				//EMIT4(0x0f, 0xb6, 0x58, K); /*movzbl K(%r8),%ebx*/
				
				//if(K > max_offset)
				//{
					EMIT3(0x48, 0x81, 0xfa); /*cmp   K,%rdx*/ //hlen - offset
					EMIT(K, 4);
					EMIT_COND_JMP(X86_JLE, fast_path_len[i]);	/*jle  xx*/
					//max_offset = K;				
				//}

				EMIT4(0x44, 0x0f, 0xb6, 0x8e);
				EMIT(K, 4); /*movzbl K(%rsi),%r9d*/
			
				//EMIT3(0x80, 0xe3, 0x0f);   /*and  $15,%bl*/
				//EMIT3(0xc0, 0xe3, 0x02);   /*shl  $2,%bl*/
				EMIT4(0x66, 0x41, 0x83, 0xe1);
				EMIT1(0x0f);   /*and  $15,%r9w*/
				EMIT4(0x66, 0x41, 0xc1, 0xe1);
				EMIT1(0x02);   /*shl  $2,%r9w*/

				EMIT_JMP(slow_path_len[i]);
				fast_path_len[i] = is_near(slow_path_len[i])? 20 : 24;

				goto slowpath_load;
				
				break;
			case BPF_S_LD_W_IND:
				//func = sk_load_word_ind;
//common_load_ind:		
				//seen |= SEEN_DATAREF | SEEN_XREG;
				//t_offset = func - (image + addrs[i]);

				seen |= SEEN_DATAREF | SEEN_XREG;
				//EMIT1_off32(0xbe, K);	/* mov imm32,%esi   */
				//EMIT1_off32(0xe8, t_offset);	/* call sk_load_xxx_ind */

				// inline the above callq
				//EMIT1_off32(0xbe, K);	/* mov imm32,%esi   */
				//EMIT2(0x01, 0xde);      /*add    %ebx,%esi*/
				//EMIT4(0x41, 0x8b, 0x04, 0x30);  /*mov    (%r8,%rsi,1),%eax*/

				// Consolidate the above two mov into one
				
				//EMIT4(0x41, 0x8b, 0x84, 0x18);
				//EMIT4(K, 0x00, 0x00, 0x00);    /*mov K(%r8,%rbx,1),%eax*/

				////EMIT3(0x48, 0xc7, 0xc1); /*mov K %rcx*/ //hlen - offset
				////EMIT(K, 4);
				////EMIT3(0x4c, 0x01, 0xc9);	/*add    %r9,%rcx*/
				////EMIT4(0x48, 0x83, 0xc1, 0x03);  /*add    $0x3,%rcx*/
				////EMIT3(0x48, 0x39, 0xca); /*cmp    %rcx,%rdx*/
				
				//EMIT2(0x8d, 0x8a); 	/*lea  -k(%rdx),%ecx*/
				//EMIT((~K) + 1, 4);
				//EMIT3(0x44, 0x29, 0xc9);  /*sub  %r9d, %ecx*/
				//EMIT3(0x83, 0xf9, 0x03);       /*cmp    $0x3,%ecx*/
				
				//wx//EMIT4(0x67, 0x41, 0x8d, 0x89); /*lea    K(%r9d),%ecx*/
				//wx//EMIT(K, 4);
				
				EMIT4(0x67, 0x45, 0x8d, 0x81); /*lea    K(%r9d),%r8d*/
				EMIT(K, 4);
				
				//wx//EMIT2(0x85, 0xc9);             /*test   %ecx,%ecx*/
				EMIT3(0x45, 0x85, 0xc0);	     /*test   %r8d,%r8d*/
				EMIT_COND_JMP(X86_JS, cleanup_addr - addrs[i] - 2 + slow_path_len[i] + fast_path_len[i] + (is_near(fast_path_len[i])? 10 : 14));

				EMIT2(0x89, 0xd0);             /*mov    %edx,%eax*/
				//wx//EMIT2(0x29, 0xc8);       /*sub    %ecx,%eax*/
				EMIT3(0x44, 0x29, 0xc0);	/*sub    %r8d,%eax*/
				EMIT3(0x83, 0xf8, 0x03);       /*cmp    $0x3,%eax*/
				
				
				EMIT_COND_JMP(X86_JLE, fast_path_len[i]);
				
				if((filter[i+1].code == BPF_S_JMP_JGT_K) || (filter[i+1].code == BPF_S_JMP_JGE_K) || (filter[i+1].code == BPF_S_JMP_JEQ_K))
					goto bpf_fast_path;
									
				
				EMIT4(0x42, 0x8b, 0x84, 0x0e);
				EMIT(K, 4);  			/*mov K(%rsi,%r9,1),%eax*/
				EMIT2(0x0f, 0xc8);		/*bswap  %eax*/

				EMIT_JMP(slow_path_len[i]);

				fast_path_len[i] = is_near(slow_path_len[i])? 12 : 16;

				goto slowpath_load;

				break;
			case BPF_S_LD_H_IND:
				//func = sk_load_half_ind;
				//goto common_load_ind;

				seen |= SEEN_DATAREF | SEEN_XREG;
				//EMIT1_off32(0xbe, K);	/* mov imm32,%esi   */
				//EMIT2(0x01, 0xde);      /*add    %ebx,%esi*/
				//EMIT1(0x41);			
				//EMIT4(0x0f, 0xb7, 0x04, 0x30);/*movzwl(%r8,%rsi,1),%eax*/

				// Consolidate the above two mov into one
				
				//EMIT1(0x41);			
				//EMIT4(0x0f, 0xb7, 0x84, 0x18);/*movzwl K(%r8,%rbx,1),%eax*/
				//EMIT4(K, 0x00, 0x00, 0x00);	
				
				////EMIT3(0x48, 0xc7, 0xc1); /*mov K %rcx*/ //hlen - offset
				////EMIT(K, 4);
				////EMIT3(0x4c, 0x01, 0xc9);	/*add	 %r9,%rcx*/
				////EMIT4(0x48, 0x83, 0xc1, 0x01);	/*add	 $0x1,%rcx*/
				////EMIT3(0x48, 0x39, 0xca); /*cmp	  %rcx,%rdx*/

				//EMIT2(0x8d, 0x8a); 	/*lea -k(%rdx),%ecx*/
				//EMIT((~K) + 1, 4);
				//EMIT3(0x44, 0x29, 0xc9);  /*sub   %r9d, %ecx*/
				//EMIT3(0x83, 0xf9, 0x01); /*cmp    $0x1,%ecx*/

				//wx//EMIT4(0x67, 0x41, 0x8d, 0x89); /*lea    K(%r9d),%ecx*/
				//wx//EMIT(K, 4);

				EMIT4(0x67, 0x45, 0x8d, 0x81); /*lea    K(%r9d),%r8d*/
				EMIT(K, 4);
				
				//wx//EMIT2(0x85, 0xc9);             /*test   %ecx,%ecx*/
				EMIT3(0x45, 0x85, 0xc0);	     /*test   %r8d,%r8d*/
				EMIT_COND_JMP(X86_JS, cleanup_addr - addrs[i] - 2 + slow_path_len[i] + fast_path_len[i] + (is_near(fast_path_len[i])? 10 : 14));

				EMIT2(0x89, 0xd0);             /*mov    %edx,%eax*/
				//wx//EMIT2(0x29, 0xc8);        /*sub    %ecx,%eax*/
				EMIT3(0x44, 0x29, 0xc0);	/*sub    %r8d,%eax*/
				EMIT3(0x83, 0xf8, 0x01);       /*cmp    $0x1,%eax*/
				
				EMIT_COND_JMP(X86_JLE, fast_path_len[i]);
				
				if((filter[i+1].code == BPF_S_JMP_JGT_K) || (filter[i+1].code == BPF_S_JMP_JGE_K) || (filter[i+1].code == BPF_S_JMP_JEQ_K))
					goto bpf_fast_path;

				EMIT4(0x42, 0x0f, 0xb7, 0x84);
				EMIT1(0x0e);
				EMIT(K, 4);  /*movzwl K(%rsi,%r9,1),%eax*/
				EMIT4(0x66, 0xc1, 0xc0, 0x08);/*# ntohs()*/
	
				EMIT_JMP(slow_path_len[i]);

				fast_path_len[i] = is_near(slow_path_len[i])? 15 : 19;

				goto slowpath_load;

				break;
			case BPF_S_LD_B_IND:
				//func = sk_load_byte_ind;
				//goto common_load_ind;
				seen |= SEEN_DATAREF | SEEN_XREG;
				//EMIT1_off32(0xbe, K);	/* mov imm32,%esi   */
				//EMIT2(0x01, 0xde);      /*add    %ebx,%esi*/
				//EMIT1(0x41);			
				//EMIT4(0x0f, 0xb6, 0x04, 0x30);/*movzbl	(%r8,%rsi),%eax*/

				// Consolidate the above mov into one
				
				//EMIT1(0x41);
				//EMIT4(0x0f, 0xb6, 0x84, 0x18);
				//EMIT4(K, 0x00, 0x00, 0x00);  /*movzbl K(%r8,%rbx,1),%eax*/

				////EMIT3(0x48, 0xc7, 0xc1); /*mov K %rcx*/ //hlen - offset
				////EMIT(K, 4);
				////EMIT3(0x4c, 0x01, 0xc9);	/*add	 %r9,%rcx*/
				////EMIT3(0x48, 0x39, 0xca); /*cmp	  %rcx,%rdx*/

				//EMIT4(0x67, 0x49, 0x8d, 0x89); /*lea    k(%r9d),%rcx*/
				//EMIT(K, 4);
				//EMIT3(0x48, 0x39, 0xca);  /*cmp    %rcx,%rdx*/

				//wx//EMIT4(0x67, 0x41, 0x8d, 0x89); /*lea    K(%r9d),%ecx*/
				//wx//EMIT(K, 4);
				EMIT4(0x67, 0x45, 0x8d, 0x81); /*lea    K(%r9d),%r8d*/
				EMIT(K, 4);

				//wx//EMIT2(0x85, 0xc9);             /*test   %ecx,%ecx*/
				EMIT3(0x45, 0x85, 0xc0);	     /*test   %r8d,%r8d*/
				EMIT_COND_JMP(X86_JS, cleanup_addr - addrs[i] - 2 + slow_path_len[i] + fast_path_len[i] + (is_near(fast_path_len[i])? 5 : 9));
				//wx//EMIT3(0x48, 0x39, 0xca);       /*cmp    %rcx,%rdx*/
				EMIT3(0x44, 0x39, 0xc2);             /*cmp    %r8d,%edx*/


				EMIT_COND_JMP(X86_JLE, fast_path_len[i]);
				
				if((filter[i+1].code == BPF_S_JMP_JGT_K) || (filter[i+1].code == BPF_S_JMP_JGE_K) || (filter[i+1].code == BPF_S_JMP_JEQ_K))
					goto bpf_fast_path;
				
				
				EMIT4(0x42, 0x0f, 0xb6, 0x84);
				EMIT1(0x0e);
				EMIT(K, 4);  /*movzbl K(%rsi,%r9,1),%eax*/

				EMIT_JMP(slow_path_len[i]);
				fast_path_len[i] = is_near(slow_path_len[i])? 11 : 15;

				goto slowpath_load;

				break;
				unsigned short cmp_data_h;
				unsigned int   cmp_data_l;
				unsigned char  cmp_data_b;
				
bpf_fast_path:
				i++;
				switch (filter[i].code){
					COND_JMP_SEL(BPF_S_JMP_JGT_K, X86_JA, X86_JBE);
					COND_JMP_SEL(BPF_S_JMP_JGE_K, X86_JAE, X86_JB);
					COND_JMP_SEL(BPF_S_JMP_JEQ_K, X86_JE, X86_JNE);
cond_jmp_branch:			
					
		            		f_offset = addrs[i + filter[i].jf] - addrs[i-1] + slow_path_len[i-1] + fast_jump_off[i-1];
					t_offset = addrs[i + filter[i].jt] - addrs[i-1] + slow_path_len[i-1] + fast_jump_off[i-1];

					/*ilen = prog - temp;
					if (image) {
						if (proglen + ilen > oldproglen) {
							printf("bpb_jit_compile fatal error\n");
							free(addrs);
							free(image);
							return NULL;
						}
						memcpy(image + proglen, temp, ilen);
					}

					proglen += ilen;
					addrs[i-1] = proglen;
					prog = temp;*/
					
					/* same targets, can avoid doing the test :) */
					if (filter[i].jt == filter[i].jf) {
						EMIT_JMP(t_offset);
						fast_path_len[i-1] = is_near(t_offset)? 2 : 6;
						break;
					}

					switch (filter[i-1].code) {
					case BPF_S_LD_W_ABS:
						cmp_data_l = filter[i].k;
						cmp_data_l = htonl(cmp_data_l);
						EMIT2(0x81, 0xbe);
						EMIT(K, 4);
						EMIT(cmp_data_l, 4);    /*cmpl $cmp_data_l, K(%rsi)*/
						fast_path_len[i-1] = 10;
						break;
					case BPF_S_LD_H_ABS:
						cmp_data_h = filter[i].k;
						cmp_data_h = htons(cmp_data_h);
						EMIT3(0x66, 0x81, 0xbe);
						EMIT(K, 4);
						EMIT(cmp_data_h, 2);    /*cmpw $cmp_data_h, K(%rsi)*/
						fast_path_len[i-1] = 9;
						break;
					case BPF_S_LD_B_ABS:
						cmp_data_b = filter[i].k;
						EMIT2(0x80, 0xbe);
						EMIT(K, 4);
						EMIT1(cmp_data_b);    /*cmpw $cmp_data_b, K(%rsi)*/
						fast_path_len[i-1] = 7;
						break;
					case BPF_S_LD_W_IND:
						cmp_data_l = filter[i].k;
						cmp_data_l = htonl(cmp_data_l);
						EMIT4(0x42, 0x81, 0xbc, 0x0e);
						EMIT(K, 4);
						EMIT(cmp_data_l, 4);    /*cmpl $cmp_data_l, K(%rsi, %r9, 1)*/
						fast_path_len[i-1] = 12;
						break;
					case BPF_S_LD_H_IND:
						cmp_data_h = filter[i].k;
						cmp_data_h = htons(cmp_data_h);
						EMIT1(0x66);
						EMIT4(0x42, 0x81, 0xbc, 0x0e);
						EMIT(K, 4);
						EMIT(cmp_data_h, 2);    /*cmpw $cmp_data_h, K(%rsi, %r9, 1)*/
						fast_path_len[i-1] = 11;
						break;
					case BPF_S_LD_B_IND:
						cmp_data_b = filter[i].k;
						EMIT4(0x42, 0x80, 0xbc, 0x0e);
						EMIT(K, 4);
						EMIT1(cmp_data_b);    /*cmpw $cmp_data_b, K(%rsi, %r9, 1)*/
						fast_path_len[i-1] = 9;
						break;
					}
					if (filter[i].jt != 0) {
						if (filter[i].jf)
							t_offset += is_near(f_offset) ? 2 : 6;
						EMIT_COND_JMP(t_op, t_offset);
						fast_path_len[i-1] += is_near(t_offset)? 2 : 6;
						if (filter[i].jf)
						{
							EMIT_JMP(f_offset);
							fast_jump_off[i-1] = is_near(f_offset)? 2 : 6;;
							fast_path_len[i-1] += is_near(f_offset)? 2 : 6;
						}
						goto fast_path_end;
					}
					EMIT_COND_JMP(f_op, f_offset);
					fast_jump_off[i-1] = 0;
					fast_path_len[i-1] += is_near(f_offset)? 2 : 6;
					goto fast_path_end;
				}	
fast_path_end:
				EMIT_JMP(addrs[i] - addrs[i-1] + slow_path_len[i-1]);
				fast_path_len[i-1] += is_near(addrs[i] - addrs[i-1] + slow_path_len[i-1])? 2 : 6;
				fast_jump_off[i-1] += is_near(addrs[i] - addrs[i-1] + slow_path_len[i-1])? 2 : 6;
				i--;
slowpath_load:			
				
				switch (filter[i].code) 
				{
				case BPF_S_LD_W_ABS:
					func = sk_load_word;
common_load:
					slow_path_len[i] = 11;
					t_offset = func - (image + addrs[i]);
					EMIT2(0x41, 0xb8); /*mov    K,%r8d*/
					EMIT(K, 4);
  
					EMIT1_off32(0xe8, t_offset); /* call */
					break;
				case BPF_S_LD_H_ABS:
					func = sk_load_half;
					goto common_load;
					
				case BPF_S_LD_B_ABS:
					func = sk_load_byte;
					goto common_load;

				case BPF_S_LDX_B_MSH:
					func = sk_load_byte_msh;
					goto common_load;

				case BPF_S_LD_W_IND:
					func = sk_load_word;
common_load_ind:				
					slow_path_len[i] = 5;
					//wx//EMIT4(0x67, 0x45, 0x8d, 0x81); /*lea    K(%r9d),%r8d*/
					//wx//EMIT(K, 4);
					t_offset = func  - (image + addrs[i]);
					EMIT1_off32(0xe8, t_offset); /* call sk_load_func*/
	
					break;
				case BPF_S_LD_H_IND:
					func = sk_load_half;
					goto common_load_ind;
					
				case BPF_S_LD_B_IND:
					func = sk_load_byte;
					goto common_load_ind;
				}
				
				break;
			case BPF_S_JMP_JA:
				t_offset = addrs[i + K] - addrs[i];
				EMIT_JMP(t_offset);
				break;
			COND_SEL(BPF_S_JMP_JGT_K, X86_JA, X86_JBE);
			COND_SEL(BPF_S_JMP_JGE_K, X86_JAE, X86_JB);
			COND_SEL(BPF_S_JMP_JEQ_K, X86_JE, X86_JNE);
			COND_SEL(BPF_S_JMP_JSET_K,X86_JNE, X86_JE);
			COND_SEL(BPF_S_JMP_JGT_X, X86_JA, X86_JBE);
			COND_SEL(BPF_S_JMP_JGE_X, X86_JAE, X86_JB);
			COND_SEL(BPF_S_JMP_JEQ_X, X86_JE, X86_JNE);
			COND_SEL(BPF_S_JMP_JSET_X,X86_JNE, X86_JE);

cond_branch:			f_offset = addrs[i + filter[i].jf] - addrs[i];
				t_offset = addrs[i + filter[i].jt] - addrs[i];

				/* same targets, can avoid doing the test :) */
				if (filter[i].jt == filter[i].jf) {
					EMIT_JMP(t_offset);
					break;
				}

				switch (filter[i].code) {
				case BPF_S_JMP_JGT_X:
				case BPF_S_JMP_JGE_X:
				case BPF_S_JMP_JEQ_X:
					seen |= SEEN_XREG;
					//EMIT2(0x39, 0xd8); /* cmp %ebx,%eax */
					EMIT3(0x44, 0x39, 0xc8); /* cmp %r9d,%eax */
					break;
				case BPF_S_JMP_JSET_X:
					seen |= SEEN_XREG;
					//EMIT2(0x85, 0xd8); /* test %ebx,%eax */
					EMIT3(0x44, 0x85, 0xc8); /* test %r9d,%eax */
					break;
				case BPF_S_JMP_JEQ_K:
					if (K == 0) {
						EMIT2(0x85, 0xc0); /* test   %eax,%eax */
						break;
					}
				case BPF_S_JMP_JGT_K:
				case BPF_S_JMP_JGE_K:
					if (K <= 127)
						EMIT3(0x83, 0xf8, K); /* cmp imm8,%eax */
					else
						EMIT1_off32(0x3d, K); /* cmp imm32,%eax */
					break;
				case BPF_S_JMP_JSET_K:
					if (K <= 0xFF)
						EMIT2(0xa8, K); /* test imm8,%al */
					else if (!(K & 0xFFFF00FF))
						EMIT3(0xf6, 0xc4, K >> 8); /* test imm8,%ah */
					else if (K <= 0xFFFF) {
						EMIT2(0x66, 0xa9); /* test imm16,%ax */
						EMIT(K, 2);
					} else {
						EMIT1_off32(0xa9, K); /* test imm32,%eax */
					}
					break;
				}
				if (filter[i].jt != 0) {
					if (filter[i].jf)
						t_offset += is_near(f_offset) ? 2 : 6;
					EMIT_COND_JMP(t_op, t_offset);
					if (filter[i].jf)
						EMIT_JMP(f_offset);
					break;
				}
				EMIT_COND_JMP(f_op, f_offset);
				break;
			default:
				/* hmm, too complex filter, give up with jit compiler */
				goto out;
			}
			ilen = prog - temp;
			if (image) {
				if (unlikely(proglen + ilen > oldproglen)) {
					pr_err("bpb_jit_compile fatal error\n");
					kfree(addrs);
					module_free(NULL, image);
					return;
				}
				memcpy(image + proglen, temp, ilen);
			}
			proglen += ilen;
			addrs[i] = proglen;
			prog = temp;
		}
		/* last bpf instruction is always a RET :
		 * use it to give the cleanup instruction(s) addr
		 */
		cleanup_addr = proglen - 1; /* ret */
		if (seen)
			cleanup_addr -= 1; /* leaveq */
		//if (seen & SEEN_XREG) 
			//cleanup_addr -= 4; /* mov  -8(%rbp),%rbx */

		if (image) {
			WARN_ON(proglen != oldproglen);
			break;
		}
		if (proglen == oldproglen) {
			image = module_alloc(max_t(unsigned int,
						   proglen,
						   sizeof(struct work_struct)));
			if (!image)
				goto out;
		}
		oldproglen = proglen;
	}
	
	if (bpf_jit_enable > 1)
		pr_err("flen=%d proglen=%u pass=%d image=%p\n",
		       flen, proglen, pass, image);

	if (image) {
		if (bpf_jit_enable > 1)
			print_hex_dump(KERN_ERR, "JIT code: ", DUMP_PREFIX_ADDRESS,
				       16, 1, image, proglen, false);
		i = 0;
		printk("jit machine code: \n");
		while(i < proglen)
		{
		
			if(image[i] <= 0xf)
				printk("0%x", image[i++]);
			else 
				printk("%x", image[i++]);			
		}
		printk("\n");
		bpf_flush_icache(image, image + proglen);

		fp->bpf_func = (void *)image;
	}
	
out:
	kfree(addrs);
	return ;
}



static void jit_free_defer(struct work_struct *arg)
{
	module_free(NULL, arg);
}

/* run from softirq, we must use a work_struct to call
 * module_free() from process context
 */
void bpf_jit_free(struct sk_filter *fp)
{
	if (fp->bpf_func != sk_run_filter) {
		struct work_struct *work = (struct work_struct *)fp->bpf_func;

		INIT_WORK(work, jit_free_defer);
		schedule_work(work);
	}
}

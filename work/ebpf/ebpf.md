ebpf insn

op:8, dst_reg:4, src_reg:4, off:16, imm:32

(LSB)                                 (MSB)

dc 02 00 00 10 00 00 00

a. opcode for ALU & JMP

  +----------------+--------+--------------------+
  |   4 bits       |  1 bit |   3 bits           |
  | operation code | source | instruction class  |
  +----------------+--------+--------------------+

eBPF classes:

 BPF_LD    0x00
 BPF_LDX   0x01
 BPF_ST    0x02
 BPF_STX   0x03
 BPF_ALU   0x04
 BPF_JMP   0x05
 BPF_JMP32 0x06
 BPF_ALU64 0x07

When BPF_CLASS(code) == BPF_ALU or BPF_JMP, 4th bit encodes source operand
 BPF_SRC(code) == BPF_X - use 'src_reg' register as source operand
 BPF_SRC(code) == BPF_K - use 32-bit immediate as source operand

If BPF_CLASS(code) == BPF_ALU or BPF_ALU64 [ in eBPF ], BPF_OP(code) is one of:

  BPF_ADD   0x00
  BPF_SUB   0x10
  BPF_MUL   0x20
  BPF_DIV   0x30
  BPF_OR    0x40
  BPF_AND   0x50
  BPF_LSH   0x60
  BPF_RSH   0x70
  BPF_NEG   0x80
  BPF_MOD   0x90
  BPF_XOR   0xa0
  BPF_MOV   0xb0  /* eBPF only: mov reg to reg */
  BPF_ARSH  0xc0  /* eBPF only: sign extending shift right */
  BPF_END   0xd0  /* eBPF only: endianness conversion */

BPF_NEG:  reg = ! reg
BPF_END:  reg = be16 reg
BPF_MOV:  regd = regs
BPF_MOV:  regd = data

           src_code   dst_reg   src_reg     off        imm     
BPF_NEG       0         reg        0         0          0
BPF_END       0         reg        0         0       16/32/64
BPF_MOV       1         regd       regs      0          0
BPF_MOV       0         regd       0         0         data
BPF_XXX       1         regd       regs      0          0
BPF_XXX       0         regd       0         0         data

BPF_MOD,BPF_DIV src_code is 0 imm should not be 0.
BPF_LSH,BPF_RSH,BPF_ARSH src_code is 0, imm should not more than 32/64(BPF_ALU64)



If BPF_CLASS(code) == BPF_JMP or BPF_JMP32 [ in eBPF ], BPF_OP(code) is one of:

  BPF_JA    0x00  /* BPF_JMP only */
  BPF_JEQ   0x10
  BPF_JGT   0x20
  BPF_JGE   0x30
  BPF_JSET  0x40
  BPF_JNE   0x50  /* eBPF only: jump != */
  BPF_JSGT  0x60  /* eBPF only: signed '>' */
  BPF_JSGE  0x70  /* eBPF only: signed '>=' */
  BPF_CALL  0x80  /* eBPF BPF_JMP only: function call */
  BPF_EXIT  0x90  /* eBPF BPF_JMP only: function return */
  BPF_JLT   0xa0  /* eBPF only: unsigned '<' */
  BPF_JLE   0xb0  /* eBPF only: unsigned '<=' */
  BPF_JSLT  0xc0  /* eBPF only: signed '<' */
  BPF_JSLE  0xd0  /* eBPF only: signed '<=' */

           src_code   dst_reg   src_reg     off        imm     
BPF_CALL      0          0         0         0        func_id
BPF_JA        0          0         0        off         0
BPF_EXIT      0          0         0         0          0

if r2 != 2 goto +22
BPF_JNE       0          2         0         22         2 

b. opcode for LOAD & STORE

For load and store instructions the 8-bit 'code' field is divided as:

  +--------+--------+-------------------+
  | 3 bits | 2 bits |   3 bits          |
  |  mode  |  size  | instruction class |
  +--------+--------+-------------------+
  (MSB)                             (LSB)

Size modifier is one of ...

  BPF_W   0x00    /* word */
  BPF_H   0x08    /* half word */
  BPF_B   0x10    /* byte */
  BPF_DW  0x18    /* eBPF only, double word */

  BPF_IMM  0x00  /* used for 32-bit mov in classic BPF and 64-bit in eBPF */
  BPF_ABS  0x20
  BPF_IND  0x40
  BPF_MEM  0x60
  BPF_LEN  0x80  /* classic BPF only, reserved in eBPF */
  BPF_MSH  0xa0  /* classic BPF only, reserved in eBPF */
  BPF_XADD 0xc0  /* eBPF only, exclusive add */

BPF_ST only support BPF_MEM, src reg is 0
BPF_MEM | <size> | BPF_ST:   *(size *) (dst_reg + off) = imm32

BPF_LD: BPF_IMM BPF_ABS BPF_IND
BPF_IMM: size only BPF_DW, off is 0, src_reg is 0 or BPF_PSEUDO_MAP_FD /* replace_map_fd_with_map_ptr() should have caught bad ld_imm64 */
eBPF has one 16-byte instruction: BPF_LD | BPF_DW | BPF_IMM which interpreted
as single instruction that loads 64-bit immediate value into a dst_reg.
18 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00	r1 = 0

BPF_LD | BPF_DW | BPF_IMM
when src reg is BPF_PSEUDO_MAP_FD, load the fd of map

BPF_ABS & BPF_IND: 
eBPF has two non-generic instructions: (BPF_ABS | <size> | BPF_LD) and
(BPF_IND | <size> | BPF_LD) which are used to access packet data.
For example:
BPF_IND | BPF_W | BPF_LD means:
R0 = ntohl(*(u32 *) (((struct sk_buff *) R6)->data + src_reg + imm32))

BPF_LDX only support BPF_MEM, imm should be 0
BPF_STX only support BPF_MEM and BPF_XADD, imm should be 0

BPF_MEM | <size> | BPF_STX:  *(size *) (dst_reg + off) = src_reg
BPF_MEM | <size> | BPF_LDX:  dst_reg = *(size *) (src_reg + off)
BPF_XADD | BPF_W  | BPF_STX: lock xadd *(u32 *)(dst_reg + off16) += src_reg
BPF_XADD | BPF_DW | BPF_STX: lock xadd *(u64 *)(dst_reg + off16) += src_reg




attach:

1. cgroup_bpf_attach

BPF_CGROUP_RUN_PROG_SOCK_OPS(&sock_ops);

a. BPF_PROG_TYPE_SOCK_OPS

attach_type: BPF_CGROUP_SOCK_OPS

enum {
	BPF_SOCK_OPS_VOID,
	BPF_SOCK_OPS_TIMEOUT_INIT,	/* Should return SYN-RTO value to use or
					 * -1 if default value should be used
					 */
	BPF_SOCK_OPS_RWND_INIT,		/* Should return initial advertized
					 * window (in packets) or -1 if default
					 * value should be used
					 */
	BPF_SOCK_OPS_TCP_CONNECT_CB,	/* Calls BPF program right before an
					 * active connection is initialized
					 */
	BPF_SOCK_OPS_ACTIVE_ESTABLISHED_CB,	/* Calls BPF program when an
						 * active connection is
						 * established
						 */
	BPF_SOCK_OPS_PASSIVE_ESTABLISHED_CB,	/* Calls BPF program when a
						 * passive connection is
						 * established
						 */
	BPF_SOCK_OPS_NEEDS_ECN,		/* If connection's congestion control
					 * needs ECN
					 */
	BPF_SOCK_OPS_BASE_RTT,		/* Get base RTT. The correct value is
					 * based on the path and may be
					 * dependent on the congestion control
					 * algorithm. In general it indicates
					 * a congestion threshold. RTTs above
					 * this indicate congestion
					 */
	BPF_SOCK_OPS_RTO_CB,		/* Called when an RTO has triggered.
					 * Arg1: value of icsk_retransmits
					 * Arg2: value of icsk_rto
					 * Arg3: whether RTO has expired
					 */
	BPF_SOCK_OPS_RETRANS_CB,	/* Called when skb is retransmitted.
					 * Arg1: sequence number of 1st byte
					 * Arg2: # segments
					 * Arg3: return value of
					 *       tcp_transmit_skb (0 => success)
					 */
	BPF_SOCK_OPS_STATE_CB,		/* Called when TCP changes state.
					 * Arg1: old_state
					 * Arg2: new_state
					 */
	BPF_SOCK_OPS_TCP_LISTEN_CB,	/* Called on listen(2), right after
					 * socket transition to LISTEN state.
					 */
	BPF_SOCK_OPS_RTT_CB,		/* Called on every RTT.
					 */
	BPF_SOCK_OPS_PARSE_HDR_OPT_CB,	/* Parse the header option.
					 * It will be called to handle
					 * the packets received at
					 * an already established
					 * connection.
					 *
					 * sock_ops->skb_data:
					 * Referring to the received skb.
					 * It covers the TCP header only.
					 *
					 * bpf_load_hdr_opt() can also
					 * be used to search for a
					 * particular option.
					 */
	BPF_SOCK_OPS_HDR_OPT_LEN_CB,	/* Reserve space for writing the
					 * header option later in
					 * BPF_SOCK_OPS_WRITE_HDR_OPT_CB.
					 * Arg1: bool want_cookie. (in
					 *       writing SYNACK only)
					 *
					 * sock_ops->skb_data:
					 * Not available because no header has
					 * been	written yet.
					 *
					 * sock_ops->skb_tcp_flags:
					 * The tcp_flags of the
					 * outgoing skb. (e.g. SYN, ACK, FIN).
					 *
					 * bpf_reserve_hdr_opt() should
					 * be used to reserve space.
					 */
	BPF_SOCK_OPS_WRITE_HDR_OPT_CB,	/* Write the header options
					 * Arg1: bool want_cookie. (in
					 *       writing SYNACK only)
					 *
					 * sock_ops->skb_data
					 * Referring to the outgoing skb.
					 * It covers the TCP header
					 * that has already been written
					 * by the kernel and the
					 * earlier bpf-progs.
					 *
					 * sock_ops->skb_tcp_flags:
					 * The tcp_flags of the outgoing
					 * skb. (e.g. SYN, ACK, FIN).
					 *
					 * bpf_store_hdr_opt() should
					 * be used to write the
					 * option.
					 *
					 * bpf_load_hdr_opt() can also
					 * be used to search for a
					 * particular option that
					 * has already been written
					 * by the kernel or the
					 * earlier bpf-progs.
					 */
};


2). BPF_PROG_TYPE_CGROUP_SKB : 

attach_type:
	BPF_CGROUP_INET_INGRESS,
	BPF_CGROUP_INET_EGRESS,


3). BPF_PROG_TYPE_CGROUP_SOCK

attach_type:
	BPF_CGROUP_INET_SOCK_CREATE  : inet_create
	BPF_CGROUP_INET_SOCK_RELEASE : inet_release
	BPF_CGROUP_INET4_POST_BIND : inet_bind
	BPF_CGROUP_INET6_POST_BIND : inet6_bind

4). BPF_PROG_TYPE_CGROUP_SOCK_ADDR:

attach_type:
	BPF_CGROUP_INET4_BIND
	BPF_CGROUP_INET6_BIND
	BPF_CGROUP_INET4_CONNECT
	BPF_CGROUP_INET6_CONNECT
	BPF_CGROUP_UDP4_SENDMSG
	BPF_CGROUP_UDP6_SENDMSG
	BPF_CGROUP_UDP4_RECVMSG
	BPF_CGROUP_UDP6_RECVMSG
	BPF_CGROUP_INET4_GETPEERNAME
	BPF_CGROUP_INET6_GETPEERNAME
	BPF_CGROUP_INET4_GETSOCKNAME
	BPF_CGROUP_INET6_GETSOCKNAME
	BPF_CGROUP_SETSOCKOPT
	BPF_CGROUP_GETSOCKOPT

5). BPF_PROG_TYPE_CGROUP_SOCKOPT

attach_type:
	BPF_CGROUP_GETSOCKOPT
	BPF_CGROUP_SETSOCKOPT

6). BPF_PROG_TYPE_SK_SKB/MSG

attach_type:
	BPF_SK_MSG_VERDICT
	BPF_SK_SKB_STREAM_PARSER
	BPF_SK_SKB_STREAM_VERDICT


2. tracing

  "/sys/kernel/debug/tracing/events/syscalls/sys_enter_accept/id"
  id = atoi(buf);
  attr.config = id; 

  efd = sys_perf_event_open(&attr, -1/*pid*/, 0/*cpu*/, -1/*group_fd*/, 0); 
  if (efd < 0) {
          printf("event %d fd %d err %s\n", id, efd, strerror(errno));
          return -1; 
  }   
  ioctl(efd, PERF_EVENT_IOC_ENABLE, 0); 
  ioctl(efd, PERF_EVENT_IOC_SET_BPF, prog_fd);

3. BPF_PROG_TYPE_SOCKET_FILTER

setsockopt(sock, SOL_SOCKET, SO_ATTACH_BPF, &prog_fd, sizeof(__u32)) == 0)




key point:

1. function call
85 00 00 00 02 00 00 00	call 2
bpf_map_update_elem is id 2

loader:
a. verfier
fixup_bpf_calls
{
	fn = prog->aux->ops->get_func_proto(insn->imm);
	insn->imm = fn->func - __bpf_call_base;
}

b. jit
bpf_jit_comp.c-->do_jit
{
	case BPF_JMP | BPF_CALL:
		func = (u8 *) __bpf_call_base + imm32;
		jmp_offset = func - (image + addrs[i]);
		EMIT1_off32(0xE8, jmp_offset);
} 


2. map fd
18 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00	r1 = 0 ll
85 00 00 00 02 00 00 00	call 2

a. preloader
//replace the BPF_LD | BPF_IMM | BPF_DW  src_reg with BPF_PSEUDO_MAP_FD
and imm as the fd of map according to the relocation section info
bpf_load.c-->parse_relo_and_apply
{
	if (insn[insn_idx].code != (BPF_LD | BPF_IMM | BPF_DW)) {
                        printf("invalid relo for insn[%d].code 0x%x\n",
                               insn_idx, insn[insn_idx].code);
                        return 1;
       }   
       insn[insn_idx].src_reg = BPF_PSEUDO_MAP_FD;

       /* Match FD relocation against recorded map_data[] offset */
       for (map_idx = 0; map_idx < nr_maps; map_idx++) {
               if (maps[map_idx].elf_offset == sym.st_value) {
                       match = true;
                       break;
               }   
       }   
       if (match)
               insn[insn_idx].imm = maps[map_idx].fd;

}

b. loader
1). verifer
replace_map_fd_with_map_ptr
{
     if (insn->src_reg != BPF_PSEUDO_MAP_FD) {
               verbose("unrecognized bpf_ld_imm64 insn\n");
               return -EINVAL;
       }    

       f = fdget(insn->imm);
       map = __bpf_map_get(f);
       if (IS_ERR(map)) {
               verbose("fd %d is not pointing to valid bpf_map\n",
                       insn->imm);
               return PTR_ERR(map);
       }    

       err = check_map_prog_compatibility(map, env->prog);
       if (err) {
               fdput(f);
               return err; 
       }    

       /* store map pointer inside BPF_LD_IMM64 instruction */
       insn[0].imm = (u32) (unsigned long) map; 
       insn[1].imm = ((u64) (unsigned long) map) >> 32;
}

2). jit
bpf_jit_comp.c-->do_jit
    case BPF_LD | BPF_IMM | BPF_DW:
            /* optimization: if imm64 is zero, use 'xor <dst>,<dst>'
             * to save 7 bytes.
             */
            if (insn[0].imm == 0 && insn[1].imm == 0) {
                    b1 = add_2mod(0x48, dst_reg, dst_reg);
                    b2 = 0x31; /* xor */
                    b3 = 0xC0;
                    EMIT3(b1, b2, add_2reg(b3, dst_reg, dst_reg));

                    insn++;
                    i++;
                    break;
            }

            /* movabsq %rax, imm64 */
            EMIT2(add_1mod(0x48, dst_reg), add_1reg(0xB8, dst_reg));
            EMIT(insn[0].imm, 4);
            EMIT(insn[1].imm, 4);

            insn++;
            i++;
            break;

3. tail call
85 00 00 00 07 00 00 00	call 7
bpf_tai_call is id 7
a. verfier
fixup_bpf_calls
{
	if (insn->imm == BPF_FUNC_tail_call) {
                        /* mark bpf_tail_call as different opcode to avoid
                         * conditional branch in the interpeter for every normal
                         * call and to prevent accidental JITing by JIT compiler
                         * that doesn't support bpf_tail_call yet
                         */
                        insn->imm = 0;
                        insn->code = BPF_JMP | BPF_TAIL_CALL;
	}

}

b. jit
bpf_jit_comp.c-->do_jit
case BPF_JMP | BPF_TAIL_CALL:                    
              emit_bpf_tail_call(&prog);

4. call ctx
struct bpf_sock_ops_kern  vs  struct bpf_sock_ops

loader
convert_ctx_accesses
{
	ops->convert_ctx_access();
}


5. sockmap /map in map / prog map
array: array map store with prog/map/sock ptr
hash:  hash map store with prog/map/sock ptr


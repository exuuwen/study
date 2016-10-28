/* bpf_jit.S : BPF JIT helper functions
 *
 * Copyright (C) 2011 Eric Dumazet (eric.dumazet@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */
#include <linux/linkage.h>
#include <asm/dwarf2.h>
/*
 * Calling convention :
 * rdi : skb pointer
 * esi : copy of skb->data
 * r8d : k offset
 * r9d : hlen = skb->len - skb->data_len
 */

/*call	skb_copy_bits;			\*/
/*test    %eax,%eax;			\*/

#define SKBDATA	%rsi


#define bpf_slow_path_common(LEN)		\
	push	%rdi;    /* save skb */		\
	push	SKBDATA;			\
	push	%rdx;	       			\
	push	%r9;				\
/* rsi already has offset */			\
	mov	$LEN, %ecx;	/* len */	\
	mov 	%r8d, %esi;			\
	lea	-12(%rbp),%rdx;			\
	call	skb_copy_bits;			\
	test    %eax,%eax;			\
	pop 	%r9;				\
	pop	%rdx;				\
	pop	SKBDATA;			\
	pop	%rdi

	
sk_load_word:
	.globl	sk_load_word

	bpf_slow_path_common(4)
	js	bpf_error
	mov	-12(%rbp),%eax
	bswap	%eax 			/* ntohl() */
	ret


sk_load_half:
	.globl	sk_load_half

	bpf_slow_path_common(2)
	js	bpf_error
	mov	-12(%rbp),%ax
	rol	$8,%ax			# ntohs()
	movzwl	%ax,%eax
	ret


sk_load_byte:
	.globl	sk_load_byte

	bpf_slow_path_common(1)
	js	bpf_error
	movzbl	-12(%rbp),%eax
	ret

/**
 * sk_load_byte_msh - BPF_S_LDX_B_MSH helper
 *
 * Implements BPF_S_LDX_B_MSH : ldxb  4*([offset]&0xf)
 * Must preserve A accumulator (%eax)
 * Inputs : %esi is the offset value, already known positive
 */
ENTRY(sk_load_byte_msh)
	CFI_STARTPROC
sk_load_byte_msh:
	.globl  sk_load_byte_msh
	xchg	%eax,%r9d /* dont lose A , X is about to be scratched */
	bpf_slow_path_common(1)
	js	bpf_error
	movzbl	-12(%rbp),%eax
	and	$15,%al
	shl	$2,%al
	xchg	%eax,%r9d
	ret
	CFI_ENDPROC
ENDPROC(sk_load_byte_msh)

bpf_error:
# force a return 0 from jit handler
	xor		%eax,%eax
	//mov		-8(%rbp),%rbx
	leaveq
	ret



	

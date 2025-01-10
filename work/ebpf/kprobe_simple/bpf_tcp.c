// +build ignore
//


#include "vmlinux.h"

/* #include <stdbool.h> */
/* #include <linux/if_ether.h> */
/* #include <linux/in.h> */
/* #include <linux/ip.h> */
/* #include <linux/udp.h> */
/* #include <linux/tcp.h> */
/* #include <linux/types.h> */
/* #include <linux/pkt_cls.h> */
//#include <linux/bpf.h> 

#include <bpf/bpf_endian.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

#define trace_printk(fmt, ...)                                       \
	do {                                                         \
		char _fmt[] = fmt;                                   \
		bpf_trace_printk(_fmt, sizeof(_fmt), ##__VA_ARGS__); \
	} while (0)

#define member_address(source_struct, source_member)                                                         \
	({                                                                                                   \
		void *__ret;                                                                                 \
		__ret = (void *)(((char *)source_struct) + offsetof(typeof(*source_struct), source_member)); \
		__ret;                                                                                       \
	})

#define member_read(destination, source_struct, source_member)         \
	do {                                                           \
		bpf_probe_read_kernel(                                        \
			destination,                                   \
			sizeof(source_struct->source_member),          \
			member_address(source_struct, source_member)); \
	} while (0)

SEC("kprobe/tcp_v4_rcv")
int BPF_KPROBE(tcp_v4_rcv, struct __sk_buff *skb)
{
	__u16  eth_proto;
	unsigned int len;

	bpf_probe_read_kernel(&len, sizeof(unsigned int), &skb->len);
	bpf_probe_read_kernel(&eth_proto, sizeof(__u16), &skb->protocol);

	trace_printk("hook in tcp_v4_rcv, len:%u, proto:%x", len, __bpf_ntohs(eth_proto));

	return 0;
}

char __license[] SEC("license") = "Dual MIT/GPL";

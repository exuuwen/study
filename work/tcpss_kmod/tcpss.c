/*
 * NOTE: This example is works on x86 and powerpc.
 * Here's a sample kernel module showing the use of kprobes to dump a
 * stack trace and selected registers when _do_fork() is called.
 *
 * For more information on theory of operation of kprobes, see
 * Documentation/kprobes.txt
 *
 * You will see the trace data in /var/log/messages and on the console
 * whenever _do_fork() is invoked to create a new process.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <net/sock.h>
#include <net/tcp.h>

#define INET_LHTABLE_SIZE 32
#define MAX_SYMBOL_LEN	64
#define IP_FMT(ip)                                            \
	(ip) & 0xff, ((ip) >> 8) & 0xff, ((ip) >> 16) & 0xff, \
		((ip) >> 24) & 0xff

static char symbol[MAX_SYMBOL_LEN] = "_do_fork";
module_param_string(symbol, symbol, sizeof(symbol), 0644);

static int __init tcpss_init(void)
{
	int bucket = 0;
	struct inet_listen_hashbucket *ilb;
	struct hlist_nulls_node *node;
	struct sock *sk = NULL;
	struct net *net = &init_net;
	int i = 0;

	for (i = 0; i < INET_LHTABLE_SIZE; i++) {
		bucket = i;
		ilb = &tcp_hashinfo.listening_hash[bucket];
		spin_lock(&ilb->lock);
		sk = sk_nulls_head(&ilb->nulls_head);

		ilb = &tcp_hashinfo.listening_hash[bucket];

		sk_nulls_for_each_from(sk, node) {
			if (!net_eq(sock_net(sk), net))
				continue;
			if (sk->sk_family == AF_INET) {
				const struct inet_sock *inet = inet_sk(sk);

				printk("listen sk=%p, state %x, %d.%d.%d.%d:%d->%d.%d.%d.%d:%d, inode %ld\n", sk, sk->sk_state,  IP_FMT(inet->inet_rcv_saddr), ntohs(inet->inet_sport),  IP_FMT(inet->inet_daddr), ntohs(inet->inet_dport), sock_i_ino(sk));
			}
		}
		spin_unlock(&ilb->lock);
	}

	bucket = 0;
	for (; bucket <= tcp_hashinfo.ehash_mask; ++bucket) {
		struct sock *sk;
		struct hlist_nulls_node *node;
		spinlock_t *lock = inet_ehash_lockp(&tcp_hashinfo, bucket);

		/* Lockless fast path for the common case of empty buckets */
		if (hlist_nulls_empty(&tcp_hashinfo.ehash[bucket].chain))
			continue;

		spin_lock_bh(lock);
		sk_nulls_for_each(sk, node, &tcp_hashinfo.ehash[bucket].chain) {
			const struct inet_sock *inet = inet_sk(sk);

			if (sk->sk_family != AF_INET ||
			    !net_eq(sock_net(sk), net)) {
				continue;
			}
		
			printk("established sk=%p, state %x, %d.%d.%d.%d:%d->%d.%d.%d.%d:%d, inode %ld\n", sk, sk->sk_state,  IP_FMT(inet->inet_rcv_saddr), ntohs(inet->inet_sport), IP_FMT(inet->inet_daddr), ntohs(inet->inet_dport), sock_i_ino(sk));
		}
		spin_unlock_bh(lock);
	}

	pr_info("success\n");
	return 0;
}

static void __exit tcpss_exit(void)
{
	pr_info("success exit\n");
}

module_init(tcpss_init)
module_exit(tcpss_exit)
MODULE_LICENSE("GPL");

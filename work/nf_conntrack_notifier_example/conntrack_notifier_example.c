#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/rculist.h>
#include <linux/rculist_nulls.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/security.h>
#include <linux/skbuff.h>
#include <linux/errno.h>
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/siphash.h>

#include <linux/netfilter.h>
#include <net/netlink.h>
#include <net/sock.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>

enum {
    EVT_TCP_CONNECTED,
    EVT_TCP_DISCONNECTED,
    EVT_MAX
};

static unsigned long record_type = EVT_TCP_CONNECTED;
module_param(record_type, ulong, 0644);
MODULE_PARM_DESC(record_type, "record connected or disconnected TCP connection");

MODULE_LICENSE("GPL");

static inline void dump_tuple_ip(const struct nf_conntrack_tuple *t, char *prefix)
{
	printk("%s: %u %pI4:%hu -> %pI4:%hu\n",
	       prefix, t->dst.protonum,
	       &t->src.u3.ip, ntohs(t->src.u.all),
	       &t->dst.u3.ip, ntohs(t->dst.u.all));
}

static int
conntrack_notifier_example_event(unsigned int events, struct nf_ct_event *item)
{
    struct nf_conn *ct = item->ct;
    struct nf_conntrack_tuple *orig = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    struct nf_conntrack_tuple *reply = &ct->tuplehash[IP_CT_DIR_REPLY].tuple;

    if (record_type == EVT_TCP_CONNECTED && (events & (1 << IPCT_ASSURED))) {
        printk("TCP connection connected");
    } else if (record_type == EVT_TCP_DISCONNECTED && (events & (1 << IPCT_DESTROY))) {
        printk("TCP connection disconnected");
    } else {
        return 0;
    }

    dump_tuple_ip(orig, "original");
    dump_tuple_ip(reply, "reply");

    return 0;
}

static struct nf_ct_event_notifier conntrack_notifier_example_notifier = {
	.fcn = conntrack_notifier_example_event,
};

static int __net_init conntrack_notifier_example_net_init(struct net *net)
{
	int ret;

	ret = nf_conntrack_register_notifier(net, &conntrack_notifier_example_notifier);
	if (ret < 0) {
		pr_err("ctnetlink_init: cannot register notifier.\n");
	}

	return ret;
}

static void conntrack_notifier_example_net_exit(struct net *net)
{
	nf_conntrack_unregister_notifier(net, &conntrack_notifier_example_notifier);
}

static void __net_exit conntrack_notifier_example_net_exit_batch(struct list_head *net_exit_list)
{
	struct net *net;

	list_for_each_entry(net, net_exit_list, exit_list)
		conntrack_notifier_example_net_exit(net);

	/* wait for other cpus until they are done with ctnl_notifiers */
	synchronize_rcu();
}

static struct pernet_operations conntrack_notifier_example_net_ops = {
	.init		= conntrack_notifier_example_net_init,
	.exit_batch	= conntrack_notifier_example_net_exit_batch,
};

static int __init conntrack_notifier_example_init(void)
{
	int ret;

	ret = register_pernet_subsys(&conntrack_notifier_example_net_ops);
	if (ret < 0) {
		pr_err("conntrack_notifier_example_init: cannot register pernet operations\n");
	}

	return ret;
}

static void __exit conntrack_notifier_example_exit(void)
{
    unregister_pernet_subsys(&conntrack_notifier_example_net_ops);
}

module_init(conntrack_notifier_example_init);
module_exit(conntrack_notifier_example_exit);
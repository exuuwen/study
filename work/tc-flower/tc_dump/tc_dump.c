#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/module.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <net/pkt_sched.h>
#include <net/pkt_cls.h>

#include <linux/rhashtable.h>
#include <linux/workqueue.h>

#include <linux/if_ether.h>
#include <linux/in6.h>
#include <linux/ip.h>
#include <linux/mpls.h>

#include <net/sch_generic.h>
#include <net/pkt_cls.h>
#include <net/ip.h>
#include <net/flow_dissector.h>

#include <net/dst.h>
#include <net/dst_metadata.h>


#define TC_INGRESS_PARENT TC_H_MAKE(TC_H_CLSACT, TC_H_MIN_INGRESS)

struct fl_flow_key {
        int     indev_ifindex;
        struct flow_dissector_key_control control;
        struct flow_dissector_key_control enc_control;
        struct flow_dissector_key_basic basic;
        struct flow_dissector_key_eth_addrs eth;
        struct flow_dissector_key_vlan vlan;
        union {
                struct flow_dissector_key_ipv4_addrs ipv4;
                struct flow_dissector_key_ipv6_addrs ipv6;
        };
        struct flow_dissector_key_ports tp;
        struct flow_dissector_key_icmp icmp;
        struct flow_dissector_key_arp arp; 
        struct flow_dissector_key_keyid enc_key_id;
        union {
                struct flow_dissector_key_ipv4_addrs enc_ipv4;
                struct flow_dissector_key_ipv6_addrs enc_ipv6;
        };   
        struct flow_dissector_key_ports enc_tp;
        struct flow_dissector_key_tcp tcp; 
        struct flow_dissector_key_ip ip;
} __aligned(BITS_PER_LONG / 8); /* Ensure that we can do comparisons as longs. */


struct fl_flow_mask_range {
        unsigned short int start;
        unsigned short int end; 
};

struct fl_flow_mask {
        struct fl_flow_key key; 
        struct fl_flow_mask_range range;
        struct rcu_head rcu; 
};

struct cls_fl_head {
	u8 padding[192];
        struct fl_flow_mask mask;
        struct flow_dissector dissector;
        bool mask_assigned;
        struct list_head filters;
        struct rhashtable_params ht_params;
        union {
                struct work_struct work;
                struct rcu_head rcu; 
        };   
        struct idr handle_idr;
        struct idr user_handle_idr;
};

static inline unsigned int 
tc_make_handle(unsigned int major, unsigned int minor)
{
    return TC_H_MAKE(major << 16, minor);
}

static struct Qdisc *qdisc_match_from_root(struct Qdisc *root, u32 handle)
{
        struct Qdisc *q;

        if (!qdisc_dev(root))
                return (root->handle == handle ? root : NULL);

        if (!(root->flags & TCQ_F_BUILTIN) &&
            root->handle == handle)
                return root;

        hash_for_each_possible_rcu(qdisc_dev(root)->qdisc_hash, q, hash, handle) {
                if (q->handle == handle)
                        return q;
        }    
        return NULL;
}

static int get_chain_init(void)
{
	u32 tcm_parent = TC_INGRESS_PARENT;
	u32 tcm_info = tc_make_handle(2, 0);
	int ifindex = 105;
	struct net_device *dev;
	struct Qdisc *q;
	struct tcf_block *block;
	struct tcf_chain *chain;
	unsigned long cl = 0;
	struct tcf_proto *tp;
	const struct Qdisc_class_ops *cops;

	dev = __dev_get_by_index(&init_net, ifindex);
	if (!dev)
		return 0;

	q = qdisc_match_from_root(dev_ingress_queue(dev)->qdisc_sleeping, TC_H_MAJ(tcm_parent));
	if (!q)
		goto out;
	cops = q->ops->cl_ops;
	if (!cops)
		goto out;
	if (!cops->tcf_block)
		goto out;
	if (TC_H_MIN(tcm_parent)) {
		cl = cops->find(q, tcm_parent);
		if (cl == 0)
			goto out;
	}
	block = cops->tcf_block(q, cl);
	if (!block)
		goto out;

	list_for_each_entry(chain, &block->chain_list, list) {
		if (0 != chain->index)
			continue;
		for (tp = rtnl_dereference(chain->filter_chain);
	     	     tp; tp = rtnl_dereference(tp->next)) {
			if (tp->prio != TC_H_MAJ(tcm_info))
				continue;

			struct cls_fl_head *head = rtnl_dereference(tp->root);

			if (head && head->mask_assigned) {
				struct fl_flow_mask *mask = &head->mask;
				printk("the kind %s %p %p \n", tp->ops->kind, head, mask);
				printk("mask->range.start %u\n", mask->range.start);
				printk("mask->range.end %u\n", mask->range.end);
				printk("mask->key.indev_ifindex 0x%x\n", mask->key.indev_ifindex);
				printk("mask->key.control.thoff 0x%x\n", mask->key.control.thoff);
				printk("mask->key.control.addr_type 0x%x\n", mask->key.control.addr_type);
				printk("mask->key.control.flags 0x%x\n", mask->key.control.flags);
				printk("mask->key.enc_control.thoff 0x%x\n", mask->key.enc_control.thoff);
				printk("mask->key.enc_control.addr_type 0x%x\n", mask->key.enc_control.addr_type);
				printk("mask->key.enc_control.flags 0x%x\n", mask->key.enc_control.flags);
				printk("mask->key.basic.ip_proto 0x%x\n", mask->key.basic.ip_proto);
				printk("mask->key.basic.n_proto 0x%x\n", mask->key.basic.n_proto);
				printk("mask->key.basic.padding 0x%x\n", mask->key.basic.padding);
				printk("mask->key.eth.dst[0] 0x%x\n", mask->key.eth.dst[0]);
				printk("mask->key.eth.dst[1] 0x%x\n", mask->key.eth.dst[1]);
				printk("mask->key.eth.dst[2] 0x%x\n", mask->key.eth.dst[2]);
				printk("mask->key.eth.dst[3] 0x%x\n", mask->key.eth.dst[3]);
				printk("mask->key.eth.dst[4] 0x%x\n", mask->key.eth.dst[4]);
				printk("mask->key.eth.dst[5] 0x%x\n", mask->key.eth.dst[5]);
				printk("mask->key.eth.src[0] 0x%x\n", mask->key.eth.src[0]);
				printk("mask->key.eth.src[1] 0x%x\n", mask->key.eth.src[1]);
				printk("mask->key.eth.src[2] 0x%x\n", mask->key.eth.src[2]);
				printk("mask->key.eth.src[3] 0x%x\n", mask->key.eth.src[3]);
				printk("mask->key.eth.src[4] 0x%x\n", mask->key.eth.src[4]);
				printk("mask->key.eth.src[5] 0x%x\n", mask->key.eth.src[5]);
				printk("mask->key.vlan.vlan_id 0x%x\n", mask->key.vlan.vlan_id);
				printk("mask->key.vlan.vlan_priority 0x%x\n", mask->key.vlan.vlan_priority);
				printk("mask->key.vlan.padding 0x%x\n", mask->key.vlan.padding);
				printk("mask->key.ipv4.src 0x%x\n", mask->key.ipv4.src);
				printk("mask->key.ipv4.dst 0x%x\n", mask->key.ipv4.dst);
				printk("mask->key.tp.src 0x%x\n", mask->key.tp.src);
				printk("mask->key.tp.dst 0x%x\n", mask->key.tp.dst);
				printk("mask->key.icmp.type 0x%x\n", mask->key.icmp.type);
				printk("mask->key.icmp.code 0x%x\n", mask->key.icmp.code);
				printk("mask->key.arp.sip 0x%x\n", mask->key.arp.sip);
				printk("mask->key.arp.tip 0x%x\n", mask->key.arp.tip);
				printk("mask->key.arp.op 0x%x\n", mask->key.arp.op);
				printk("mask->key.arp.sha[0] 0x%x\n", mask->key.arp.sha[0]);
				printk("mask->key.arp.sha[1] 0x%x\n", mask->key.arp.sha[1]);
				printk("mask->key.arp.sha[2] 0x%x\n", mask->key.arp.sha[2]);
				printk("mask->key.arp.sha[3] 0x%x\n", mask->key.arp.sha[3]);
				printk("mask->key.arp.sha[4] 0x%x\n", mask->key.arp.sha[4]);
				printk("mask->key.arp.sha[5] 0x%x\n", mask->key.arp.sha[5]);
				printk("mask->key.arp.tha[0] 0x%x\n", mask->key.arp.tha[0]);
				printk("mask->key.arp.tha[1] 0x%x\n", mask->key.arp.tha[1]);
				printk("mask->key.arp.tha[2] 0x%x\n", mask->key.arp.tha[2]);
				printk("mask->key.arp.tha[3] 0x%x\n", mask->key.arp.tha[3]);
				printk("mask->key.arp.tha[4] 0x%x\n", mask->key.arp.tha[4]);
				printk("mask->key.arp.tha[5] 0x%x\n", mask->key.arp.tha[5]);
				printk("mask->key.enc_key_id 0x%x\n", mask->key.enc_key_id.keyid);
				printk("mask->key.enc_ipv4.src 0x%x\n", mask->key.enc_ipv4.src);
				printk("mask->key.enc_ipv4.dst 0x%x\n", mask->key.enc_ipv4.dst);
				printk("mask->key.enc_tp.src 0x%x\n", mask->key.enc_tp.src);
				printk("mask->key.enc_tp.dst 0x%x\n", mask->key.enc_tp.dst);
				printk("mask->key.tcp.flags 0x%x\n", mask->key.tcp.flags);
				printk("mask->key.ip.tos 0x%x\n", mask->key.ip.tos);
				printk("mask->key.ip.ttl 0x%x\n", mask->key.ip.ttl);
			}
				
		}
	}

out:
	return 0;
}

static void get_chain_exit(void)
{
}

module_init(get_chain_init);
module_exit(get_chain_exit);

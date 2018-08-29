/* (C) 2016/09/8 By wenxu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_nat_core.h>
#include <linux/jhash.h>

#define DRV_VERSION     "1.0"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wenxu <wenxu@ucloud.cn>");
MODULE_VERSION(DRV_VERSION);
MODULE_DESCRIPTION("Xtables: SNAT from ip-pool");

#define MAX 64
struct snat_pool {
	unsigned int size;
	__be32 ips[MAX];
};

static int pool_check(const struct xt_tgchk_param *par)
{
	const struct snat_pool *mr = par->targinfo;
	if (mr->size > MAX) {
		pr_info("%s: pool size must be less than %d\n",
			par->target->name, MAX);
		return -EINVAL;
	}

	return 0;
}

static unsigned int
pool_target(struct sk_buff *skb, const struct xt_action_param *par)
{
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	const struct snat_pool *mr = par->targinfo;
	unsigned int index;
	struct nf_nat_range newrange;
	uint32_t sip, dip;
	uint32_t port = 0;
	uint32_t sport = 0, dport = 0;
	uint8_t  protonum;

	ct = nf_ct_get(skb, &ctinfo);

	NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED 
		|| ctinfo == IP_CT_RELATED_REPLY));

	sip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u3.ip;
	dip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u3.ip;
	protonum = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
	if (protonum == IPPROTO_TCP || protonum == IPPROTO_UDP) {
		sport = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
		dport = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
		port = (sport << 16) | dport;
	}

	index = jhash_3words(sip, dip, port, 0);
	index %= mr->size;

	memset(&newrange, 0, sizeof(newrange));

	newrange.min_addr.ip = mr->ips[index];
	newrange.max_addr.ip = mr->ips[index];
	newrange.flags = NF_NAT_RANGE_MAP_IPS;
	
	return nf_nat_setup_info(ct, &newrange, NF_NAT_MANIP_SRC);
}

static struct xt_target pool_reg __read_mostly = {
	.name           = "SNATPOOL",
	.family         = NFPROTO_IPV4,
	.target         = pool_target,
	.targetsize     = sizeof(struct snat_pool),
	.table          = "nat",
	.hooks          = (1 << NF_INET_POST_ROUTING) | (1 << NF_INET_LOCAL_IN),
	.checkentry     = pool_check,
	.me             = THIS_MODULE,
};

static int __init pool_target_init(void)
{
	return xt_register_target(&pool_reg);
}

static void __exit pool_target_exit(void)
{
	xt_unregister_target(&pool_reg);
}

module_init(pool_target_init);
module_exit(pool_target_exit);

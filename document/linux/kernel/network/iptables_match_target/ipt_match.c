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

#define DRV_VERSION     "1.0"
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_DESCRIPTION("Xtables: match test");

struct id {
	unsigned char id;
};

static int match_check(const struct xt_mtchk_param *par)
{
	const struct id *mr = par->matchinfo;
	if (mr->id == 0) {
		pr_info("%s: id must not be 0\n",
			par->match->name);
		return -EINVAL;
	}

	return 0;
}

static bool
match_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct id *mr = par->matchinfo;

	printk("just test match id: %u, hooknum %u\n", mr->id, par->hooknum);

	return XT_CONTINUE;

}

static struct xt_match match_reg __read_mostly = {
	.name           = "match",
	.family         = NFPROTO_IPV4,
	.match         = match_mt,
	.matchsize     = sizeof(struct id),
	//.table          = "mangle",
	.hooks          = (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_LOCAL_IN),
	.checkentry     = match_check,
	.me             = THIS_MODULE,
};

static int __init match_match_init(void)
{
	return xt_register_match(&match_reg);
}

static void __exit match_match_exit(void)
{
	xt_unregister_match(&match_reg);
}

module_init(match_match_init);
module_exit(match_match_exit);

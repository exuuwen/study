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
MODULE_DESCRIPTION("Xtables: TARGET print");

struct id {
	unsigned char id;
};

static int target_check(const struct xt_tgchk_param *par)
{
	const struct id *mr = par->targinfo;
	if (mr->id == 0) {
		pr_info("%s: id must not be 0\n",
			par->target->name);
		return -EINVAL;
	}

	return 0;
}

static unsigned int
target_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct id *mr = par->targinfo;

	printk("just test target id: %u, hooknum %u\n", mr->id, par->hooknum);

	return XT_CONTINUE;

}

static struct xt_target target_reg __read_mostly = {
	.name           = "TARGET",
	.family         = NFPROTO_IPV4,
	.target         = target_tg,
	.targetsize     = sizeof(struct id),
	//.table          = "mangle",
	.hooks          = (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_LOCAL_IN),
	.checkentry     = target_check,
	.me             = THIS_MODULE,
};

static int __init target_target_init(void)
{
	return xt_register_target(&target_reg);
}

static void __exit target_target_exit(void)
{
	xt_unregister_target(&target_reg);
}

module_init(target_target_init);
module_exit(target_target_exit);

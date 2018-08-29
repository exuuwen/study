/* (C) 2016/09/8 By wenxu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter/x_tables.h>
#include <net/ip.h>

#define DRV_VERSION     "1.0"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wenxu <wenxu@ucloud.cn>");
MODULE_VERSION(DRV_VERSION);
MODULE_DESCRIPTION("Xtables: STATIC change src address for nvgre packet");

struct static_src {
        __be32 ip_addr;
};

static int src_check(const struct xt_tgchk_param *par)
{
        const struct static_src *mr = par->targinfo;
        if (mr->ip_addr == 0) {
                pr_info("%s: ip address must not be zero\n",
                        par->target->name);
                return -EINVAL;
        }

        printk("STATICSRC address:%x\n", mr->ip_addr);

        return 0;
}


static unsigned int
src_target(struct sk_buff *skb, const struct xt_action_param *par)
{
        const struct static_src *mr = par->targinfo;
        struct iphdr *iph;

        iph = ip_hdr(skb);
        if (iph && iph->protocol == IPPROTO_GRE) {
                iph->saddr = mr->ip_addr;
                ip_send_check(iph);
        }

        return NF_ACCEPT;
}

static struct xt_target src_reg __read_mostly = {
        .name           = "STATICSRC",
        .family         = NFPROTO_IPV4,
        .target         = src_target,
        .targetsize     = sizeof(struct static_src),
        .table          = "raw",
        .hooks          = 1 << NF_INET_PRE_ROUTING,
        .checkentry     = src_check,
        .me             = THIS_MODULE,
};

static int __init src_target_init(void)
{
        return xt_register_target(&src_reg);
}

static void __exit src_target_exit(void)
{
        xt_unregister_target(&src_reg);
}

module_init(src_target_init);
module_exit(src_target_exit);


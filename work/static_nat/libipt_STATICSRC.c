/* (C) 2016/09/8 By wenxu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <xtables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

enum {
        FROM_STATIC = 0,
};

struct static_src {
        u_int32_t ips;
};

struct ipt_natinfo
{
        struct xt_entry_target t;
        struct static_src mr;
};

static void STATIC_help(void)
{
        printf(
                        "STATIC_SRC target options:\n"
                        " --addr <ipaddr>\n");
}

static const struct xt_option_entry STATIC_opts[] = {
        {
                .name = "addr",
                .id = FROM_STATIC,
                .type = XTTYPE_STRING,
                .flags = XTOPT_MAND
        },
        XTOPT_TABLEEND,
};

static struct ipt_natinfo *
set_contents(struct ipt_natinfo *info, char *arg)
{
        unsigned int size;
        char *tok;
        unsigned int i = 0;

        size = XT_ALIGN(sizeof(struct ipt_natinfo));
        info = realloc(info, size);
        if (!info)
                xtables_error(OTHER_PROBLEM, "Out of memory\n");

        info->mr.ips = (u_int32_t)inet_addr(arg);

        return info;
}

static void STATIC_parse(struct xt_option_call *cb)
{
        const struct ipt_entry *entry = cb->xt_entry;
        struct ipt_natinfo *info = (void *)(*cb->target);

        xtables_option_parse(cb);
        switch (cb->entry->id) {
                case FROM_STATIC: {
                                char *arg ;
                                arg = strdup(cb->arg);
                                if (arg == NULL)
                                        xtables_error(RESOURCE_PROBLEM, "strdup");
                                info = set_contents(info, arg);
                                free(arg);
                                *cb->target = &(info->t);
                                break;
                }
        }
}

static void STATIC_save(const void *ip, const struct xt_entry_target *target)
{
        const struct ipt_natinfo *info = (const void *)target;
        unsigned int i = 0;

        printf(" --addr ");
        struct in_addr ia;
        char *addr;
        ia.s_addr = info->mr.ips;
        addr = inet_ntoa(ia);
        printf("%s", addr);
}

static struct xtables_target static_tg_reg = {
        .name           = "STATICSRC",
        .version        = XTABLES_VERSION,
        .family         = NFPROTO_IPV4,
        .size           = XT_ALIGN(sizeof(struct static_src)),
        .userspacesize  = XT_ALIGN(sizeof(struct static_src)),
        .help           = STATIC_help,
        .x6_parse       = STATIC_parse,
        .save           = STATIC_save,
        .x6_options     = STATIC_opts,
};

void _init(void)
{
        xtables_register_target(&static_tg_reg);
}

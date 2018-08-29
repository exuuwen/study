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
	FROM_SNATPOOL = 0,
};

#define MAX 64
struct snat_pool {
	unsigned int size;
	u_int32_t ips[MAX];
};

struct ipt_natinfo
{
	struct xt_entry_target t;
	struct snat_pool mr;
};

static void POOL_help(void)
{
	printf(
			"SNATPOOL target options:\n"
			" --pool [<ipaddr>[,<ipaddr[,<...>]>]]\n");
}

static const struct xt_option_entry POOL_opts[] = {
	{
		.name = "pool",
		.id = FROM_SNATPOOL,
		.type = XTTYPE_STRING,
		.flags = XTOPT_MAND | XTOPT_MULTI
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

	tok = strtok(arg, ",");
	if (tok){
		while (tok && i < MAX) {
			info->mr.ips[i] = (u_int32_t)inet_addr(tok);
			info->mr.size++;
			tok = strtok(NULL, ",");
            i ++;
		}
	} else {
		info->mr.ips[i] = (u_int32_t)inet_addr(arg);
		info->mr.size++;
	}

	return info;
}

static void POOL_parse(struct xt_option_call *cb)
{
	const struct ipt_entry *entry = cb->xt_entry;
	struct ipt_natinfo *info = (void *)(*cb->target);

	xtables_option_parse(cb);
	switch (cb->entry->id) {
		case FROM_SNATPOOL: {
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

static void POOL_save(const void *ip, const struct xt_entry_target *target)
{
	const struct ipt_natinfo *info = (const void *)target;
	unsigned int i = 0;

	printf(" --pool ");
    for (i = 0; i < info->mr.size; i++) {
		struct in_addr ia;
		char *addr;
		ia.s_addr = info->mr.ips[i];
		addr = inet_ntoa(ia);
		if (i == info->mr.size-1)
			printf("%s", addr);
		else
			printf("%s,", addr);
	}
}

static struct xtables_target pool_tg_reg = {
	.name           = "SNATPOOL",
	.version        = XTABLES_VERSION,
	.family         = NFPROTO_IPV4,
	.size           = XT_ALIGN(sizeof(struct snat_pool)),
	.userspacesize  = XT_ALIGN(sizeof(struct snat_pool)),
	.help           = POOL_help,
	.x6_parse       = POOL_parse,
	.save           = POOL_save,
	.x6_options     = POOL_opts,
};

void _init(void)
{
	xtables_register_target(&pool_tg_reg);
}


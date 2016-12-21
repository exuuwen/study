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
	FROM_TARGET = 0,
};

#define MAX 64
struct id {
	unsigned char id;
};

struct ipt_targetinfo
{
	struct xt_entry_target t;
	struct id mr;
};

static void TARGET_help(void)
{
	printf(
			"TARGET target options:\n"
			" --set-id id <id 1-255>\\n");
}

static const struct xt_option_entry TARGET_opts[] = {
	{
		.name = "set-id",
		.id = FROM_TARGET,
		.type = XTTYPE_UINT8,
		.flags = XTOPT_MAND,
		.min = 1
	},
	XTOPT_TABLEEND,
};

static void TARGET_parse(struct xt_option_call *cb)
{
	struct ipt_targetinfo *info = (void *)(*cb->target);

	xtables_option_parse(cb);
	switch (cb->entry->id) {
		case FROM_TARGET: {
				info->mr.id = cb->val.u8;
				break;
		}
	}
}

static void TARGET_save(const void *ip, const struct xt_entry_target *target)
{
	const struct ipt_targetinfo *info = (const void *)target;
	unsigned int i = 0;

	printf(" --set-id %u\n", info->mr.id);
}

static struct xtables_target target_tg_reg = {
	.name           = "TARGET",
	.version        = XTABLES_VERSION,
	.family         = NFPROTO_IPV4,
	.size           = XT_ALIGN(sizeof(struct id)),
	.userspacesize  = XT_ALIGN(sizeof(struct id)),
	.help           = TARGET_help,
	.x6_parse       = TARGET_parse,
	.save           = TARGET_save,
	.x6_options     = TARGET_opts,
};

void _init(void)
{
	xtables_register_target(&target_tg_reg);
}


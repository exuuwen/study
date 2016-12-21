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
	FROM_MATCH = 0,
};

struct id {
	unsigned char id;
};

struct ipt_matchinfo
{
	struct xt_entry_match t;
	struct id mr;
};

static void match_help(void)
{
	printf(
			"match options:\n"
			" --id id <id 1--255>\n");
}

static const struct xt_option_entry match_opts[] = {
	{
		.name = "id",
		.id = FROM_MATCH,
		.type = XTTYPE_UINT8,
		.flags = XTOPT_MAND,
		.min = 1
	},
	XTOPT_TABLEEND,
};

static void match_parse(struct xt_option_call *cb)
{
	struct ipt_matchinfo *info = (void *)(*cb->match);

	xtables_option_parse(cb);
	switch (cb->entry->id) {
		case FROM_MATCH: {
				info->mr.id = cb->val.u8;
				break;
		}
	}
}

static void match_save(const void *ip, const struct xt_entry_match *match)
{
	const struct ipt_matchinfo *info = (const void *)match;

	printf(" --id %u\n", info->mr.id);
}

static struct xtables_match match_mt_reg = {
	.name           = "match",
	.version        = XTABLES_VERSION,
	.family         = NFPROTO_IPV4,
	.size           = XT_ALIGN(sizeof(struct id)),
	.userspacesize  = XT_ALIGN(sizeof(struct id)),
	.help           = match_help,
	.x6_parse       = match_parse,
	.save           = match_save,
	.x6_options     = match_opts,
};

void _init(void)
{
	xtables_register_match(&match_mt_reg);
}


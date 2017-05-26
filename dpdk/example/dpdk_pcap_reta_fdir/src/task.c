/*-
 *
 * Copyright(c) UCloud. All rights reserved.
 * All rights reserved.
 *
 */

#include <termios.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <ev.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>   
#include <sys/socket.h>  
#include <sys/inotify.h>

#include <rte_byteorder.h>
#include <rte_malloc.h>
#include <rte_string_fns.h>
#include <cmdline_rdline.h>
#include <cmdline_parse.h>
#include <cmdline_parse_num.h>
#include <cmdline_parse_string.h>
#include <cmdline_parse_ipaddr.h>
#include <cmdline_parse_etheraddr.h>
#include <cmdline_socket.h>
#include <rte_cycles.h>
#include <rte_ethdev.h>
#include <rte_jhash.h>
#include <cmdline.h>

#include "main.h"
#include "log.h"

struct cmd_udpi_mempool_result {
	cmdline_fixed_string_t udpi_string;
	cmdline_fixed_string_t mempool_string;
	cmdline_fixed_string_t show_string;
};

static void cmd_udpi_mempool_show_parsed(
	 __attribute__((unused)) void *parsed_result,
        struct cmdline *cl,
        __attribute__((unused)) void *data)
{
	FILE *file = fdopen(cl->s_out, "w+");

	rte_mempool_dump(file, udpi.pool);
	fprintf(file, "\n");
	rte_mempool_dump(file, udpi.msg_pool);

	fflush(file);

	return;
}

cmdline_parse_token_string_t cmd_udpi_mempool_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_mempool_result, udpi_string,
        "udpi");

cmdline_parse_token_string_t cmd_udpi_mempool_mempool_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_mempool_result, mempool_string,
        "mempool");

cmdline_parse_token_string_t cmd_udpi_mempool_show_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_mempool_result, show_string,
        "show");

cmdline_parse_inst_t cmd_mempool_show = {
        .f = cmd_udpi_mempool_show_parsed,
        .data = NULL,
        .help_str = "udpi mempool show",
        .tokens = {
                (void *)&cmd_udpi_mempool_string,
                (void *)&cmd_udpi_mempool_mempool_string,
                (void *)&cmd_udpi_mempool_show_string,
                NULL,
        },
};

struct cmd_udpi_reta_result {
	cmdline_fixed_string_t udpi_string;
	cmdline_fixed_string_t reta_string;
	cmdline_fixed_string_t show_string;
};

static void cmd_udpi_reta_show_parsed(
	 __attribute__((unused)) void *parsed_result,
        struct cmdline *cl,
        __attribute__((unused)) void *data)
{
	struct rte_eth_dev_info dev_info;
	struct rte_eth_rss_reta_entry64 reta_conf[UDPI_RETA_SIZE_MAX];
	uint32_t i, j;
	int status;

	FILE *file = fdopen(cl->s_out, "w+");

    	/* Get RETA size */
	for (j=0; j<udpi.n_ports; j++)
	{
		fprintf(file, "Port %u:\n", j);
		memset(&dev_info, 0, sizeof(dev_info));
		rte_eth_dev_info_get(j, &dev_info);

		memset(reta_conf, 0, sizeof(reta_conf));
		for (i = 0; i < dev_info.reta_size; i++)
			reta_conf[i / RTE_RETA_GROUP_SIZE].mask = UINT64_MAX;
		status = rte_eth_dev_rss_reta_query(j, reta_conf, dev_info.reta_size);
		if (status == 0) {
			for (i = 0; i < dev_info.reta_size; i++) {
				uint32_t reta_id = i / RTE_RETA_GROUP_SIZE;
				uint32_t reta_pos = i % RTE_RETA_GROUP_SIZE;

				if ((i%8==0) && (i!= 0))
					fprintf(file, "\n");
				fprintf(file, "%u ", reta_conf[reta_id].reta[reta_pos]); 
			}
			fprintf(file, "\n");
		}
		else
			fprintf(file, "get fail\n");
	}
		
	fflush(file);

	return;
}

cmdline_parse_token_string_t cmd_udpi_reta_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_reta_result, udpi_string,
        "udpi");

cmdline_parse_token_string_t cmd_udpi_reta_reta_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_reta_result, reta_string,
        "reta");

cmdline_parse_token_string_t cmd_udpi_reta_show_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_reta_result, show_string,
        "show");

cmdline_parse_inst_t cmd_reta_show = {
        .f = cmd_udpi_reta_show_parsed,
        .data = NULL,
        .help_str = "udpi reta show",
        .tokens = {
                (void *)&cmd_udpi_reta_string,
                (void *)&cmd_udpi_reta_reta_string,
                (void *)&cmd_udpi_reta_show_string,
                NULL,
        },
};

struct cmd_udpi_ethdev_result {
	cmdline_fixed_string_t udpi_string;
	cmdline_fixed_string_t ethdev_string;
	cmdline_fixed_string_t show_string;
};

static void cmd_udpi_ethdev_show_parsed(
	 __attribute__((unused)) void *parsed_result,
        struct cmdline *cl,
        __attribute__((unused)) void *data)
{
	int32_t ret = -1;
	struct rte_eth_stats stats;
	uint32_t i, port_id;

	memset(&stats, 0, sizeof(struct rte_eth_stats));
	FILE *file = fdopen(cl->s_out, "w+");

	for (port_id=0; port_id<udpi.n_ports; port_id++)
	{
		ret = rte_eth_stats_get(port_id, &stats);
		if(!ret)
		{
			for (i=0; i<udpi.ports[port_id].n_queues; i++)
			{
				fprintf(file, "\n\tQueue %u Rx packerts: %lu\n", i, stats.q_ipackets[i]);
				fprintf(file, "\tQueue %u Tx packerts: %lu\n", i, stats.q_opackets[i]);
				fprintf(file, "\tQueue %u Rx bytes: %lu\n", i, stats.q_ibytes[i]);
				fprintf(file, "\tQueue %u Tx bytes: %lu\n", i, stats.q_obytes[i]);
				fprintf(file, "\tQueue %u Tx error: %lu\n", i, stats.q_errors[i]);
			}

			fprintf(file, "\nProt %u Hardware Stats \n--------------------\n", port_id);
			fprintf(file, "\tRx packets: %lu\n", stats.ipackets);
			fprintf(file, "\tTx packets: %lu\n", stats.opackets);
			fprintf(file, "\tRx Bytes: %lu\n", stats.ibytes);
			fprintf(file, "\tTx Bytes: %lu\n", stats.obytes);
			fprintf(file, "\tRx No mbuf dropped packets: %lu\n", stats.imissed);
			fprintf(file, "\tRx errroneous packets: %lu\n", stats.ierrors);
			fprintf(file, "\tTx failed packets: %lu\n", stats.oerrors);
			fprintf(file, "\tRx mbuf allocation failed: %lu\n", stats.rx_nombuf);
		}		
	}

	fflush(file);
	
	return;
}

cmdline_parse_token_string_t cmd_udpi_ethdev_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_ethdev_result, udpi_string,
        "udpi");

cmdline_parse_token_string_t cmd_udpi_ethdev_ethdev_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_ethdev_result, ethdev_string,
        "ethdev");

cmdline_parse_token_string_t cmd_udpi_ethdev_show_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_ethdev_result, show_string,
        "show");

cmdline_parse_inst_t cmd_ethdev_show = {
        .f = cmd_udpi_ethdev_show_parsed,
        .data = NULL,
        .help_str = "udpi ethdev show",
        .tokens = {
                (void *)&cmd_udpi_ethdev_string,
                (void *)&cmd_udpi_ethdev_ethdev_string,
                (void *)&cmd_udpi_ethdev_show_string,
                NULL,
        },
};

struct cmd_udpi_ring_result {
	cmdline_fixed_string_t udpi_string;
	cmdline_fixed_string_t ring_string;
	cmdline_fixed_string_t show_string;
};

static void cmd_udpi_ring_show_parsed(
	 __attribute__((unused)) void *parsed_result,
        struct cmdline *cl,
        __attribute__((unused)) void *data)
{
	uint32_t i;
	FILE *file = fdopen(cl->s_out, "w+");
	
	for (i=0; i<udpi.n_workers; i++)
	{
		
		fprintf(file, "\n\tring[%u] count: %u\n", i, rte_ring_count(udpi.rings[i]));
		fprintf(file, "\tring[%u] free count: %u\n", i, rte_ring_free_count(udpi.rings[i]));
		fprintf(file, "\tring[%u] full: %u\n", i, rte_ring_full(udpi.rings[i]));
		fprintf(file, "\tring[%u] empty: %u\n", i, rte_ring_empty(udpi.rings[i]));
	}
	
	fflush(file);

	return;
}

cmdline_parse_token_string_t cmd_udpi_ring_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_ring_result, udpi_string,
        "udpi");

cmdline_parse_token_string_t cmd_udpi_ring_ring_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_ring_result, ring_string,
        "ring");

cmdline_parse_token_string_t cmd_udpi_ring_show_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_ring_result, show_string,
        "show");

cmdline_parse_inst_t cmd_ring_show = {
        .f = cmd_udpi_ring_show_parsed,
        .data = NULL,
        .help_str = "udpi ring show",
        .tokens = {
                (void *)&cmd_udpi_ring_string,
                (void *)&cmd_udpi_ring_ring_string,
                (void *)&cmd_udpi_ring_show_string,
                NULL,
        },
};


struct cmd_udpi_load_result {
	cmdline_fixed_string_t udpi_string;
	cmdline_fixed_string_t udpi_load;
	uint8_t core_id;
};

struct last_cycle {
	uint64_t core_handle_cycles;
	uint64_t core_total_cycles;
};
static struct last_cycle last_cycle[RTE_MAX_LCORE];

static void cmd_udpi_load_parsed (
	 	void *parsed_result,
    struct cmdline *cl,
        __attribute__((unused)) void *data)
{
	struct cmd_udpi_load_result *res = (struct cmd_udpi_load_result *)parsed_result;
	FILE *file = fdopen(cl->s_out, "w+");

	if (res->core_id == 0)
	{
		uint8_t i;
		for (i=1; i<udpi.n_cores; i++)
		{
			struct udpi_core_params *p = &udpi.cores[i];
			uint64_t core_total_cycles = p->core_total_cycles;
			uint64_t core_handle_cycles = p->core_handle_cycles;

			if (core_total_cycles != 0)
			{
				if (last_cycle[i].core_total_cycles && (core_total_cycles - last_cycle[i].core_total_cycles != 0))
				{
					fprintf(file, "core_id %u recent load: %lf\n", i, 100*((double)(core_handle_cycles - last_cycle[i].core_handle_cycles)/(core_total_cycles - last_cycle[i].core_total_cycles)));
				}

				last_cycle[i].core_total_cycles = core_total_cycles;
				last_cycle[i].core_handle_cycles = core_handle_cycles;
			}
		}
	}
	else if (res->core_id < udpi.n_cores)
	{
		struct udpi_core_params *p = &udpi.cores[res->core_id];
		uint64_t core_total_cycles = p->core_total_cycles;
		uint64_t core_handle_cycles = p->core_handle_cycles;

		if (core_total_cycles != 0)
		{
			fprintf(file, "core_id %u load: %lf\n", res->core_id, 100*((double)core_handle_cycles)/(core_total_cycles));
			if (last_cycle[res->core_id].core_total_cycles && (core_total_cycles - last_cycle[res->core_id].core_total_cycles != 0))
			{
				fprintf(file, "core_id %u recent load: %lf\n", res->core_id, 100*((double)(core_handle_cycles - last_cycle[res->core_id].core_handle_cycles)/(core_total_cycles - last_cycle[res->core_id].core_total_cycles)));
			}

			last_cycle[res->core_id].core_total_cycles = core_total_cycles;
			last_cycle[res->core_id].core_handle_cycles = core_handle_cycles;
		}
		else
			fprintf(file, "core_id %u unsupport\n", res->core_id);
	}
	else
		fprintf(file, "error core id. The max core_id is %u\n", udpi.n_cores);

	fflush(file);

	return;
}

cmdline_parse_token_string_t cmd_udpi_load_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_ring_result, udpi_string,
        "udpi");

cmdline_parse_token_string_t cmd_udpi_load_load_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_ring_result, ring_string,
        "load");

cmdline_parse_token_num_t cmd_udpi_load_id_string =
        TOKEN_NUM_INITIALIZER(struct cmd_udpi_load_result, core_id,
        UINT8);

cmdline_parse_inst_t cmd_load_show = {
        .f = cmd_udpi_load_parsed,
        .data = NULL,
        .help_str = "udpi load core_id (0 for all)",
        .tokens = {
                (void *)&cmd_udpi_load_string,
                (void *)&cmd_udpi_load_load_string,
                (void *)&cmd_udpi_load_id_string,
                NULL,
        },
};

struct cmd_log_result {
	cmdline_fixed_string_t udpi;
	cmdline_fixed_string_t log;
	cmdline_fixed_string_t level;
};

static void cmd_log_parsed(
	void *parsed_result,
	struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	struct cmd_log_result *res = parsed_result;
	FILE *file = fdopen(cl->s_out, "w+");

	uint8_t level = 0;
	if (!strcmp(res->level, "EMERG"))
	{
		level = 0;
	}
	else if (!strcmp(res->level, "ALERT"))
	{
		level = 1;
	}
	else if (!strcmp(res->level, "CRIT"))
	{
		level = 2;
	}
	else if (!strcmp(res->level, "ERR"))
	{
		level = 3;
	}
	else if (!strcmp(res->level, "WARNING"))
	{
		level = 4;
	}
	else if (!strcmp(res->level, "NOTICE"))
	{
		level = 5;
	}
	else if (!strcmp(res->level, "INFO"))
	{
		level = 6;
	}
	else if (!strcmp(res->level, "DEBUG"))
	{
		level = 7;
	}
	else	
	{
		fprintf(file, "unknow log level\n");
		return;
	}

	uint8_t i;
	void *msg;
	struct udpi_msg_req *req;			

	for (i=0; i<udpi.n_workers; i++)
	{
		/* Allocate message buffer */
		msg = (void *)rte_ctrlmbuf_alloc(udpi.msg_pool);
		if (msg == NULL)
			UDPI_LOG(ERR, "Unable to allocate new message\n");

		/* Fill request message */
		req = (struct udpi_msg_req *)rte_ctrlmbuf_data((struct rte_mbuf *)msg);
		memset(req, 0, sizeof(struct udpi_msg_req));

		req->type = UDPI_MSG_SET_LOG_LEVEL;
		req->set_log_level.level = level;
					
		int ret = rte_ring_sp_enqueue(udpi.msg_rings[i], msg);
		if (ret != 0)
			UDPI_LOG(ERR, "enqueue msg ring %u fail %d\n", i, ret);
	}

	fflush(file);
	
	set_local_log_level(level);
	udpi.log.level = level;
}

cmdline_parse_token_string_t cmd_log_udpi = 
	TOKEN_STRING_INITIALIZER(struct cmd_log_result, udpi, "udpi");
cmdline_parse_token_string_t cmd_log_log = 
	TOKEN_STRING_INITIALIZER(struct cmd_log_result, log, "log");
cmdline_parse_token_string_t cmd_log_level = 
	TOKEN_STRING_INITIALIZER(struct cmd_log_result, level,
				 "EMERG#ALERT#CRIT#ERR#"
				 "WARNING#NOTICE#INFO#DEBUG");

cmdline_parse_inst_t cmd_log = {
	.f = cmd_log_parsed,
	.data = NULL,
	.help_str = "udpi log level",
	.tokens = {
		(void *)&cmd_log_udpi,
		(void *)&cmd_log_log,
		(void *)&cmd_log_level,
		NULL,
	},
};

struct cmd_trace_result {
	cmdline_fixed_string_t udpi;
	cmdline_fixed_string_t trace;
	cmdline_ipaddr_t srcip;
	cmdline_ipaddr_t dstip;
	uint16_t srcport;
	uint16_t dstport;
	uint8_t proto;
};

cmdline_parse_token_string_t cmd_udpi_trace_udpi = 
	TOKEN_STRING_INITIALIZER(struct cmd_trace_result, udpi, "udpi");
cmdline_parse_token_string_t cmd_udpi_trace_trace = 
	TOKEN_STRING_INITIALIZER(struct cmd_trace_result, udpi, "trace");
cmdline_parse_token_ipaddr_t cmd_udpi_trace_srcip = 
        TOKEN_IPADDR_INITIALIZER(struct cmd_trace_result, srcip);
cmdline_parse_token_ipaddr_t cmd_udpi_trace_dstip =
        TOKEN_IPADDR_INITIALIZER(struct cmd_trace_result, dstip);
cmdline_parse_token_num_t cmd_udpi_trace_srcport =
        TOKEN_NUM_INITIALIZER(struct cmd_trace_result, srcport, UINT16);
cmdline_parse_token_num_t cmd_udpi_trace_dstport =
        TOKEN_NUM_INITIALIZER(struct cmd_trace_result, dstport, UINT16);
cmdline_parse_token_num_t cmd_udpi_trace_proto =
        TOKEN_NUM_INITIALIZER(struct cmd_trace_result, proto, UINT8);

static void cmd_trace_parsed(
	void *parsed_result,
	struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	FILE *file = fdopen(cl->s_out, "w+");

	if (udpi.trace)
	{
		fprintf(file, "udpi is already in trace\n");
		return;
	}

	struct cmd_trace_result *res = (struct cmd_trace_result *)parsed_result;
	
	uint32_t srcip = rte_be_to_cpu_32(res->srcip.addr.ipv4.s_addr);
	uint32_t dstip = rte_be_to_cpu_32(res->dstip.addr.ipv4.s_addr);
	uint16_t srcport = res->srcport;
	uint16_t dstport = res->dstport;
	uint8_t proto = res->proto;

	uint8_t i;
	void *msg;
	struct udpi_msg_req *req;			

	for (i=0; i<udpi.n_workers; i++)
	{
		/* Allocate message buffer */
		msg = (void *)rte_ctrlmbuf_alloc(udpi.msg_pool);
		if (msg == NULL)
			UDPI_LOG(ERR, "Unable to allocate new message\n");

		/* Fill request message */
		req = (struct udpi_msg_req *)rte_ctrlmbuf_data((struct rte_mbuf *)msg);
		memset(req, 0, sizeof(struct udpi_msg_req));

		req->type = UDPI_MSG_SET_TRACE;
		req->set_trace.flow.srcip = srcip;
		req->set_trace.flow.dstip = dstip;
		req->set_trace.flow.srcport = srcport;
		req->set_trace.flow.dstport = dstport;
		req->set_trace.flow.proto = proto;
					
		int ret = rte_ring_sp_enqueue(udpi.msg_rings[i], msg);
		if (ret != 0)
			UDPI_LOG(ERR, "enqueueset trace msg ring %u fail %d\n", i, ret);
	}

	fflush(file);

	udpi.log.is_trace_enabled = true;
	udpi.log.flow.srcip = srcip;
	udpi.log.flow.dstip = dstip;
	udpi.log.flow.srcport = srcport;
	udpi.log.flow.dstport = dstport;
	udpi.log.flow.proto = proto;
}

cmdline_parse_inst_t cmd_trace = {
	.f = cmd_trace_parsed,
	.data = NULL,
	.help_str = "udpi trace src_ip dst_ip src_port dst_port proto",
	.tokens = {
		(void *)&cmd_udpi_trace_udpi,
		(void *)&cmd_udpi_trace_trace,
		(void *)&cmd_udpi_trace_srcip,
		(void *)&cmd_udpi_trace_dstip,
		(void *)&cmd_udpi_trace_srcport,
		(void *)&cmd_udpi_trace_dstport,
		(void *)&cmd_udpi_trace_proto,
		NULL,
	},
};

struct cmd_tracedis_result {
	cmdline_fixed_string_t udpi;
	cmdline_fixed_string_t trace;
	cmdline_fixed_string_t disable;
};

static void cmd_tracedis_parsed(
	__attribute__((unused)) void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	uint8_t i;
	void *msg;
	struct udpi_msg_req *req;			

	for (i=0; i<udpi.n_workers; i++)
	{
		/* Allocate message buffer */
		msg = (void *)rte_ctrlmbuf_alloc(udpi.msg_pool);
		if (msg == NULL)
			UDPI_LOG(ERR, "Unable to allocate new message\n");

		/* Fill request message */
		req = (struct udpi_msg_req *)rte_ctrlmbuf_data((struct rte_mbuf *)msg);
		memset(req, 0, sizeof(struct udpi_msg_req));

		req->type = UDPI_MSG_DISABLE_TRACE;

		int ret = rte_ring_sp_enqueue(udpi.msg_rings[i], msg);
		if (ret != 0)
			UDPI_LOG(ERR, "enqueue dis trace msg ring %u fail %d\n", i, ret);
	}

	udpi.log.is_trace_enabled = false;
}


cmdline_parse_token_string_t cmd_tracedis_udpi = 
	TOKEN_STRING_INITIALIZER(struct cmd_tracedis_result, udpi, "udpi");
cmdline_parse_token_string_t cmd_tracedis_trace = 
	TOKEN_STRING_INITIALIZER(struct cmd_tracedis_result, trace, "trace");
cmdline_parse_token_string_t cmd_tracedis_disable = 
	TOKEN_STRING_INITIALIZER(struct cmd_tracedis_result, disable, "disable");

cmdline_parse_inst_t cmd_tracedis = {
	.f = cmd_tracedis_parsed,
	.data = NULL,
	.help_str = "udpi trace disable",
	.tokens = {
		(void *)&cmd_tracedis_udpi,
		(void *)&cmd_tracedis_trace,
		(void *)&cmd_tracedis_disable,
		NULL,
	},
};


struct cmd_tracelog_result {
	cmdline_fixed_string_t udpi;
	cmdline_fixed_string_t tracelog;
	cmdline_fixed_string_t show;
};

static void cmd_tracelog_parsed(
	__attribute__((unused)) void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	FILE *file = fdopen(cl->s_out, "w+");

	fprintf(file, "log level:%d\n", udpi.log.level);
	
	if (udpi.log.is_trace_enabled)
	{
		fprintf(file, "log is traced: srcip 0x%x, dstip 0x%x, srcport %u, dstport %u, proto %u\n", udpi.log.flow.srcip, udpi.log.flow.dstip, udpi.log.flow.srcport, udpi.log.flow.dstport, udpi.log.flow.proto);
	}
	else
	{
		fprintf(file, "log is not traced\n");
	}

	fflush(file);
}


cmdline_parse_token_string_t cmd_tracelog_udpi = 
	TOKEN_STRING_INITIALIZER(struct cmd_tracelog_result, udpi, "udpi");
cmdline_parse_token_string_t cmd_tracelog_tracelog = 
	TOKEN_STRING_INITIALIZER(struct cmd_tracelog_result, tracelog, "tracelog");
cmdline_parse_token_string_t cmd_tracelog_show = 
	TOKEN_STRING_INITIALIZER(struct cmd_tracelog_result, show, "show");

cmdline_parse_inst_t cmd_tracelog = {
	.f = cmd_tracelog_parsed,
	.data = NULL,
	.help_str = "udpi trace log",
	.tokens = {
		(void *)&cmd_tracelog_udpi,
		(void *)&cmd_tracelog_tracelog,
		(void *)&cmd_tracelog_show,
		NULL,
	},
};


struct cmd_quit_result {
	cmdline_fixed_string_t quit;
};

static void cmd_quit_parsed(
	__attribute__((unused)) void *parsed_result,
	struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	cmdline_quit(cl);
}

cmdline_parse_token_string_t cmd_quit_quit = 
	TOKEN_STRING_INITIALIZER(struct cmd_quit_result, quit, "quit");

cmdline_parse_inst_t cmd_quit = {
	.f = cmd_quit_parsed,
	.data = NULL,
	.help_str = "Exit application",
	.tokens = {
		(void *)&cmd_quit_quit,
		NULL,
	},
};

cmdline_parse_ctx_t main_ctx[] = {
	(cmdline_parse_inst_t *)&cmd_mempool_show,
	(cmdline_parse_inst_t *)&cmd_reta_show,
	(cmdline_parse_inst_t *)&cmd_ethdev_show,
	(cmdline_parse_inst_t *)&cmd_ring_show,
	(cmdline_parse_inst_t *)&cmd_load_show,
	(cmdline_parse_inst_t *)&cmd_log,
	(cmdline_parse_inst_t *)&cmd_trace,
	(cmdline_parse_inst_t *)&cmd_tracedis,
	(cmdline_parse_inst_t *)&cmd_tracelog,
	(cmdline_parse_inst_t *)&cmd_quit,
	NULL,
};


#define EVENT_SIZE  (sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define UNIX_DOMAIN "/tmp/UNIX.udpi"  

static const char* watch_dir = "/root/watch_dir";
static int inotify_fd;
static int watch_fd;

static struct ev_loop * loop;
static struct ev_io inotify_watcher;
static struct ev_io cmdline_watcher;
static struct ev_io accept_watcher;

static struct cmdline *cl = NULL;

static int udpi_cmdline_interact(struct cmdline *cl)
{
	char c;

	if (!cl)
		return -1;

	c = -1;
	while (1) {
		if (read(cl->s_in, &c, 1) <= 0)
		{
				return -1;
		}
		
		const char *history, *buffer;
		size_t histlen, buflen;
		int ret = 0;
		int same;

		ret = rdline_char_in(&cl->rdl, c);

		if (ret == RDLINE_RES_VALIDATED) {
			buffer = rdline_get_buffer(&cl->rdl);
			history = rdline_get_history_item(&cl->rdl, 0);
			if (history) {
				histlen = strnlen(history, RDLINE_BUF_SIZE);
				same = !memcmp(buffer, history, histlen) &&
					buffer[histlen] == '\n';
			}
			else
				same = 0;
			buflen = strnlen(buffer, RDLINE_BUF_SIZE);
			if (buflen > 1 && !same)
				rdline_add_history(&cl->rdl, buffer);
			rdline_newline(&cl->rdl, cl->prompt);
			break;
		}
		else if (ret == RDLINE_RES_EOF)
			return -1;
		else if (ret == RDLINE_RES_EXITED)
			return -1;
	}

	return 0;
}

static void
inotify_cb(__attribute__((unused))struct ev_loop *loop, ev_io *w, int revents)
{
	int length, j = 0;
	char buffer[EVENT_BUF_LEN];
	length = read(w->fd, buffer, EVENT_BUF_LEN);
	if (length < 0)
		return;

	while (j < length) 
	{
    	struct inotify_event *event = (struct inotify_event*)&buffer[j];
		if (event->len) 
		{
			RTE_LOG(INFO, USER1, "get file %s\n", event->name);
		}
    				
		j += EVENT_SIZE + event->len;
	}
}

static void
cmdline_cb(__attribute__((unused)) struct ev_loop *loop, ev_io *w, int revents)
{
	int err = udpi_cmdline_interact(cl);
	if (err)
	{
 		ev_io_stop(loop, w);
		cmdline_free(cl);
		cl = NULL;
		close(w->fd);
	}
}


static void
accept_cb (struct ev_loop *loop, ev_io *w, int revents)
{
	socklen_t len;  
	struct sockaddr_un clt_addr;  
    int com_fd;  

	//have connect request use accept  
	len = sizeof(clt_addr);  
	com_fd = accept(w->fd, (struct sockaddr*)&clt_addr, &len);  
	if(com_fd < 0)  
	{  
		perror("cannot accept client connect request");  
		return;  
	} 

	if (cl)
	{
		printf("a cmdline attached\n");  
		close(com_fd);
		return;  
	}


	cl = cmdline_new(main_ctx, "[udpi]>> ", com_fd, com_fd);
	if (cl == NULL)
	{
		printf("cmdline new fail\n");  
		close(com_fd);
		return;  
	}

	ev_init(&cmdline_watcher, cmdline_cb);
	ev_io_set(&cmdline_watcher, cl->s_in, EV_READ);
	ev_io_start(loop, &cmdline_watcher);
}

int unixsock_init()
{
	int listen_fd;  
	int ret;  
 	int i;  
	int len;  
	struct sockaddr_un srv_addr;  
    
	listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);  
 	if (listen_fd < 0)  
	{  
		perror("cannot create communication socket");  
		return -1;  
	}    
          
	//set server addr_param  
	srv_addr.sun_family = AF_UNIX;  
	strncpy(srv_addr.sun_path, UNIX_DOMAIN, sizeof(srv_addr.sun_path)-1);  
	unlink(UNIX_DOMAIN);  

	//bind sockfd & addr  
	ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));  
	if(ret < 0)  
	{  
		perror("cannot bind server socket");  
		close(listen_fd);  
		unlink(UNIX_DOMAIN);  
		return -1;  
	}  

	//listen sockfd   
	ret = listen(listen_fd, 10);  
	if (ret < 0)  
	{  
		perror("cannot listen the client connect request");  
		close(listen_fd);  
		unlink(UNIX_DOMAIN);
		return -1;  
	}  

	ev_init(&accept_watcher, accept_cb);
	ev_io_set(&accept_watcher, listen_fd, EV_READ);
	ev_io_start(loop, &accept_watcher);
}

static int udpi_task_init()
{
	loop = ev_default_loop(0);
	if (loop == NULL)
		return -1;

	inotify_fd = inotify_init1(IN_NONBLOCK);
	if (inotify_fd < -1)
		return -1;

	watch_fd = inotify_add_watch(inotify_fd, watch_dir, IN_CLOSE_WRITE | IN_MOVED_TO);
	if (watch_fd < 0)
	{
		close(inotify_fd);
		return -1;
	}
	
	int ret = unixsock_init();
	if (ret < 0)
	{
		close(inotify_fd);
		return -1;
	}

	ev_init(&inotify_watcher, inotify_cb);
	ev_io_set(&inotify_watcher, inotify_fd, EV_READ);
	ev_io_start(loop, &inotify_watcher);

	return 0;
}


void udpi_main_loop_task(void)
{
	uint32_t core_id = rte_lcore_id();
	
	struct udpi_core_params *core_params 
		= udpi_get_core_params(core_id);

	if((!core_params) 
		|| (core_params->core_type != UDPI_CORE_TASK))
	{
		rte_panic("Core %u misconfiguration\n", core_id);
		return;
	}

	UDPI_LOG_COPY_DPDK_LEVEL();	
	RTE_LOG(INFO, USER1, "Core %u is doing task\n", core_id);
	
	int err = udpi_task_init();
	if (err < 0)
	{
		rte_panic("Core %u task init fail\n", core_id);
		return;
	}

	ev_run(loop, 0);

	return;
}


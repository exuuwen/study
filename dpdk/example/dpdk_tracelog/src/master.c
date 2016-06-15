/*-
 *
 * Copyright(c) UCloud. All rights reserved.
 * All rights reserved.
 *
 */

#include <termios.h>
#include <netinet/in.h>

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

struct cmd_udpi_mempool_result {
	cmdline_fixed_string_t udpi_string;
	cmdline_fixed_string_t mempool_string;
	cmdline_fixed_string_t show_string;
};

static void cmd_udpi_mempool_show_parsed(
	 __attribute__((unused)) void *parsed_result,
        __attribute__((unused)) struct cmdline *cl,
        __attribute__((unused)) void *data)
{
	printf("\n\tdata mempool count: %u\n", rte_mempool_count(udpi.pool));
	printf("\tdata mempool free count: %u\n", rte_mempool_free_count(udpi.pool));
	printf("\tmsg mempool count: %u\n", rte_mempool_count(udpi.msg_pool));
	printf("\tmsg mempool free count: %u\n", rte_mempool_free_count(udpi.msg_pool));
	
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

struct cmd_udpi_ethdev_result {
	cmdline_fixed_string_t udpi_string;
	cmdline_fixed_string_t ethdev_string;
	cmdline_fixed_string_t show_string;
};

static void cmd_udpi_ethdev_show_parsed(
	 __attribute__((unused)) void *parsed_result,
        __attribute__((unused)) struct cmdline *cl,
        __attribute__((unused)) void *data)
{
	int32_t ret = -1;
	struct rte_eth_stats stats;
	uint32_t i, port_id;

	memset(&stats, 0, sizeof(struct rte_eth_stats));

	for (port_id=0; port_id<udpi.n_ports; port_id++)
	{
		ret = rte_eth_stats_get(port_id, &stats);
		if(!ret)
		{
			printf("\nProt %u Hardware Stats \n--------------------\n", port_id);
			printf("\tRx packets: %lu\n", stats.ipackets);
			printf("\tTx packets: %lu\n", stats.opackets);
			printf("\tRx Bytes: %lu\n", stats.ibytes);
			printf("\tTx Bytes: %lu\n", stats.obytes);
			printf("\tRx No mbuf dropped packets: %lu\n", stats.imissed);
			printf("\tRx Bad CRD dropped packets: %lu\n", stats.ibadcrc);
			printf("\tRx errroneous packets: %lu\n", stats.ierrors);
			printf("\tTx failed packets: %lu\n", stats.oerrors);
			printf("\tRx mbuf allocation failed: %lu\n", stats.rx_nombuf);

			for (i=0; i<udpi.ports[port_id].n_queues; i++)
			{
				printf("\n\tQueue %u Rx packerts: %lu\n", i, stats.q_ipackets[i]);
				printf("\tQueue %u Tx packerts: %lu\n", i, stats.q_opackets[i]);
				printf("\tQueue %u Rx bytes: %lu\n", i, stats.q_ibytes[i]);
				printf("\tQueue %u Tx bytes: %lu\n", i, stats.q_obytes[i]);
			}
		}		
	}
	
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
        __attribute__((unused)) struct cmdline *cl,
        __attribute__((unused)) void *data)
{
	struct cmd_udpi_load_result *res = (struct cmd_udpi_load_result *)parsed_result;

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
				printf("core_id %u load: %lf\n", i, 100*((double)core_handle_cycles)/(core_total_cycles));
				if (last_cycle[i].core_total_cycles && (core_total_cycles - last_cycle[i].core_total_cycles != 0))
				{
					printf("core_id %u recent load: %lf\n", i, 100*((double)(core_handle_cycles - last_cycle[i].core_handle_cycles)/(core_total_cycles - last_cycle[i].core_total_cycles)));
				}

				last_cycle[i].core_total_cycles = core_total_cycles;
				last_cycle[i].core_handle_cycles = core_handle_cycles;
			}
			else
				printf("core_id %u unsupport\n", i);

		}
	}
	else if (res->core_id < udpi.n_cores)
	{
		struct udpi_core_params *p = &udpi.cores[res->core_id];
		uint64_t core_total_cycles = p->core_total_cycles;
		uint64_t core_handle_cycles = p->core_handle_cycles;

		if (core_total_cycles != 0)
		{
			printf("core_id %u load: %lf\n", res->core_id, 100*((double)core_handle_cycles)/(core_total_cycles));
			if (last_cycle[res->core_id].core_total_cycles && (core_total_cycles - last_cycle[res->core_id].core_total_cycles != 0))
			{
				printf("core_id %u recent load: %lf\n", res->core_id, 100*((double)(core_handle_cycles - last_cycle[res->core_id].core_handle_cycles)/(core_total_cycles - last_cycle[res->core_id].core_total_cycles)));
			}

			last_cycle[res->core_id].core_total_cycles = core_total_cycles;
			last_cycle[res->core_id].core_handle_cycles = core_handle_cycles;
		}
		else
			printf("core_id %u unsupport\n", res->core_id);
	}
	else
		printf("error core id. The max core_id is %u\n", udpi.n_cores);

	return;
}

cmdline_parse_token_string_t cmd_udpi_load_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_load_result, udpi_string,
        "udpi");

cmdline_parse_token_string_t cmd_udpi_load_load_string =
        TOKEN_STRING_INITIALIZER(struct cmd_udpi_load_result, udpi_load,
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


struct cmd_debug_result {
	cmdline_fixed_string_t udpi;
	cmdline_fixed_string_t debug;
	cmdline_fixed_string_t option;
};

static void cmd_debug_parsed(
	void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	struct cmd_debug_result *res = parsed_result;
	if (!strcmp(res->option, "on"))
	{
		udpi.debug = 1;
		printf("enable debug udpi.debug:%u\n", udpi.debug);
	}
	else if (!strcmp(res->option, "off"))
	{
		udpi.debug = 0;
		printf("disable debug udpi.debug:%u\n", udpi.debug);
	}
	else
	{
		printf("show debug udpi.debug:%u\n", udpi.debug);
	}
}

cmdline_parse_token_string_t cmd_debug_udpi = 
	TOKEN_STRING_INITIALIZER(struct cmd_debug_result, udpi, "udpi");
cmdline_parse_token_string_t cmd_debug_debug = 
	TOKEN_STRING_INITIALIZER(struct cmd_debug_result, debug, "debug");
cmdline_parse_token_string_t cmd_debug_option = 
	TOKEN_STRING_INITIALIZER(struct cmd_debug_result, option, "on#off#show");

cmdline_parse_inst_t cmd_debug = {
	.f = cmd_debug_parsed,
	.data = NULL,
	.help_str = "udpi debug on|off|show",
	.tokens = {
		(void *)&cmd_debug_udpi,
		(void *)&cmd_debug_debug,
		(void *)&cmd_debug_option,
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
	(cmdline_parse_inst_t *)&cmd_ethdev_show,
	(cmdline_parse_inst_t *)&cmd_load_show,
	(cmdline_parse_inst_t *)&cmd_debug,
	(cmdline_parse_inst_t *)&cmd_quit,
	NULL,
};

void udpi_main_loop_master(void)
 {
 	struct cmdline *cl = NULL;
	uint32_t core_id = rte_lcore_id();
	struct udpi_core_params *core_params = 
		udpi_get_core_params(core_id);

	if((core_params == NULL)
		|| (core_params->core_type != UDPI_CORE_MASTER))
	{
		rte_panic("Core %u misconfiguration\n", core_id);
	}

	RTE_LOG(INFO, USER1, "Core %u is doing MASTER\n", core_id);

	cl = cmdline_stdin_new(main_ctx, "[udpi]>> ");
	if(cl == NULL)
	{
		return;
	}

	cmdline_interact(cl);
	cmdline_stdin_exit(cl);

 	return;
 }


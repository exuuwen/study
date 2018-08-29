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
#include "dpi.h"
#include "task.h"

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
	int j;
	for (j=0; j<UDPI_MAX_QUEUES; j++)
	{
		if (udpi.pool[j])
		{
			printf("\tdata mempool count: %u\n", rte_mempool_count(udpi.pool[j]));
			printf("\tdata mempool free count: %u\n", rte_mempool_free_count(udpi.pool[j]));
		}
	}
	printf("\n\tmsg mempool count: %u\n", rte_mempool_count(udpi.msg_pool));
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
			for (i=0; i<udpi.ports[port_id].n_queues; i++)
			{
				printf("\tQueue %u Rx packerts: %lu, Rx Bytes: %lu, Rx error: %lu\n", i, stats.q_ipackets[i], stats.q_ibytes[i], stats.q_errors[i]);
				//printf("\tQueue %u Tx packerts: %lu\n", i, stats.q_opackets[i]);
				//printf("\tQueue %u Tx bytes: %lu\n", i, stats.q_obytes[i]);
			}

			printf("\nProt %u Hardware Stats \n--------------------\n", port_id);
			printf("\tRx packets: %lu\n", stats.ipackets);
			//printf("\tTx packets: %lu\n", stats.opackets);
			printf("\tRx Bytes: %lu\n", stats.ibytes);
			//printf("\tTx Bytes: %lu\n", stats.obytes);
			printf("\tRx No mbuf dropped packets: %lu\n", stats.imissed);
			printf("\tRx errroneous packets: %lu\n", stats.ierrors);
			//printf("\tTx failed packets: %lu\n", stats.oerrors);
			printf("\tRx mbuf allocation failed: %lu\n", stats.rx_nombuf);
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

struct cmd_udpi_ring_result {
	cmdline_fixed_string_t udpi_string;
	cmdline_fixed_string_t ring_string;
	cmdline_fixed_string_t show_string;
};

static void cmd_udpi_ring_show_parsed(
	 __attribute__((unused)) void *parsed_result,
        __attribute__((unused)) struct cmdline *cl,
        __attribute__((unused)) void *data)
{
	uint32_t i;
	
	for (i=0; i<udpi.n_workers; i++)
	{
		printf("\n\tring[%u] count: %u, free count: %u,  full %u, empty %u\n", i, rte_ring_count(udpi.rings[i]),  rte_ring_free_count(udpi.rings[i]), rte_ring_full(udpi.rings[i]), rte_ring_empty(udpi.rings[i]));
	}

	printf("\n");

	for (i=0; i<udpi.n_dumpers-1; i++)
	{
		printf("\n\tdumper_ring[%u] count: %u, free count: %u,  full %u, empty %u\n", i, rte_ring_count(udpi.stats_rings[i]),  rte_ring_free_count(udpi.stats_rings[i]), rte_ring_full(udpi.stats_rings[i]), rte_ring_empty(udpi.stats_rings[i]));
	}

	for (i=0; i<2; i++)
	{
	printf("\n\tpacket_ring[%u] count: %u, free count: %u,  full %u, empty %u\n", i, rte_ring_count(udpi.packet_ring[i]),  rte_ring_free_count(udpi.packet_ring[i]), rte_ring_full(udpi.packet_ring[i]), rte_ring_empty(udpi.packet_ring[i]));
	}
	
	
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


struct cmd_flows_result {
	cmdline_fixed_string_t udpi;
	cmdline_fixed_string_t flows;
	cmdline_fixed_string_t show;
};

static void cmd_flows_parsed(
	__attribute__((unused)) void *parsed_result,
	 __attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	uint32_t i;

	printf("\n\ttotal eips: %lu\n", rte_atomic64_read(&udpi.eip_entries));	
	printf("\ttotal flows: %lu\n", rte_atomic64_read(&udpi.stats.flows_stats.flow_entries));	
	printf("\ttotal closed_flows: %lu\n", rte_atomic64_read(&udpi.stats.flows_stats.closed_flow_entries));	
	printf("\ttotal tcp_rst_flows: %lu\n", rte_atomic64_read(&udpi.stats.flows_stats.tcp_rst_flow_entries));	
	printf("\ttotal tcp flows: %lu\n", rte_atomic64_read(&udpi.stats.flows_stats.tcp_flow_entries));	
	printf("\ttotal tcp_closed_flows: %lu\n", rte_atomic64_read(&udpi.stats.flows_stats.tcp_closed_flow_entries));	
	printf("\ttotal udp flows: %lu\n", rte_atomic64_read(&udpi.stats.flows_stats.udp_flow_entries));	
	printf("\ttotal udp_closed_flows: %lu\n\n", rte_atomic64_read(&udpi.stats.flows_stats.udp_closed_flow_entries));	

	for (i=0; i<udpi.n_workers; i++)
	{
		printf("\tworker %u eips: %lu\n", i, rte_atomic64_read(&udpi.hash[i].eip_entries));	
	}

	//printf("\n");

	/*for (i=0; i<TCP_CONNTRACK_MAX; i++)
	{
		printf("\ttotal state %d closed_flows: %lu\n", i, rte_atomic64_read(&udpi.closed_flows[i]));	
	}*/	

	printf("\n");

	printf("\ttotal overflow eip:%lu flow:%lu notsyn:%lu cps:%lu packet:%lu packet_packet:%lu packet_mem:%lu packet_fail:%lu\n", rte_atomic64_read(&udpi.overflow.eip), rte_atomic64_read(&udpi.overflow.flow), rte_atomic64_read(&udpi.overflow.notsyn), rte_atomic64_read(&udpi.overflow.cps), rte_atomic64_read(&udpi.overflow.packet), rte_atomic64_read(&udpi.overflow.packet_packet), rte_atomic64_read(&udpi.overflow.packet_mem), rte_atomic64_read(&udpi.overflow.packet_fail));	
}

cmdline_parse_token_string_t cmd_udpi_flows_udpi = 
	TOKEN_STRING_INITIALIZER(struct cmd_flows_result, udpi, "udpi");
cmdline_parse_token_string_t cmd_udpi_flows_flows = 
	TOKEN_STRING_INITIALIZER(struct cmd_flows_result, flows, "flows");
cmdline_parse_token_string_t cmd_udpi_flows_show = 
	TOKEN_STRING_INITIALIZER(struct cmd_flows_result, show, "show");

cmdline_parse_inst_t cmd_flows_show = {
	.f = cmd_flows_parsed,
	.data = NULL,
	.help_str = "udpi flows show",
	.tokens = {
		(void *)&cmd_udpi_flows_udpi,
		(void *)&cmd_udpi_flows_flows,
		(void *)&cmd_udpi_flows_show,
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
				//printf("core_id %u load: %lf\n", i, 100*((double)core_handle_cycles)/(core_total_cycles));
				if (last_cycle[i].core_total_cycles && (core_total_cycles - last_cycle[i].core_total_cycles != 0))
				{
					printf("core_id %u recent load: %lf\n", i, 100*((double)(core_handle_cycles - last_cycle[i].core_handle_cycles)/(core_total_cycles - last_cycle[i].core_total_cycles)));
				}

				last_cycle[i].core_total_cycles = core_total_cycles;
				last_cycle[i].core_handle_cycles = core_handle_cycles;
			}
			/*else
				printf("core_id %u unsupport\n", i);
			*/

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

struct cmd_udpi_ipaddr_result {
	cmdline_fixed_string_t udpi_string;
	cmdline_fixed_string_t udpi_ipaddr;
	cmdline_ipaddr_t ip_value;
};

struct cmd_allstats_result {
	cmdline_fixed_string_t udpi;
	cmdline_fixed_string_t allstats;
	cmdline_fixed_string_t show;
};

static void cmd_allstats_parsed(
	__attribute__((unused)) void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	printf("in|ip  out|ip  in|udp  out|udp  in|tcp  out|tcp  in|retcp  out|retcp  in|tcpsyn  out|tcpsyn\n");	
	printf("0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx\n", rte_atomic64_read(&udpi.stats.input_stats.l3.ip_pkts), rte_atomic64_read(&udpi.stats.output_stats.l3.ip_pkts), rte_atomic64_read(&udpi.stats.input_stats.l4.udp_pkts), rte_atomic64_read(&udpi.stats.output_stats.l4.udp_pkts), rte_atomic64_read(&udpi.stats.input_stats.l4.tcp_pkts), rte_atomic64_read(&udpi.stats.output_stats.l4.tcp_pkts), rte_atomic64_read(&udpi.stats.input_stats.l4.tcp_retransmit_pkts), rte_atomic64_read(&udpi.stats.output_stats.l4.tcp_retransmit_pkts), rte_atomic64_read(&udpi.stats.input_stats.l4.tcp_syn_pkts), rte_atomic64_read(&udpi.stats.output_stats.l4.tcp_syn_pkts));

	printf("\nin|ipbytes  out|ipbytes  in|udpbytes  out|udpbytes  in|tcpbytes  outtcp|bytes  in|retcpbyte  out|retcpbyte\n");	
	printf("0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx\n", rte_atomic64_read(&udpi.stats.input_stats.l3.ip_bytes), rte_atomic64_read(&udpi.stats.output_stats.l3.ip_bytes), rte_atomic64_read(&udpi.stats.input_stats.l4.udp_bytes), rte_atomic64_read(&udpi.stats.output_stats.l4.udp_bytes), rte_atomic64_read(&udpi.stats.input_stats.l4.tcp_bytes), rte_atomic64_read(&udpi.stats.output_stats.l4.tcp_bytes), rte_atomic64_read(&udpi.stats.input_stats.l4.tcp_retransmit_bytes), rte_atomic64_read(&udpi.stats.output_stats.l4.tcp_retransmit_bytes));

	printf("\nin|ssh  out|ssh  in|http  out|http  in|https  out|https  in|others  out|outers\n");	
	printf("0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx\n", rte_atomic64_read(&udpi.stats.input_stats.l7.ssh), rte_atomic64_read(&udpi.stats.output_stats.l7.ssh), rte_atomic64_read(&udpi.stats.input_stats.l7.http), rte_atomic64_read(&udpi.stats.output_stats.l7.http), rte_atomic64_read(&udpi.stats.input_stats.l7.https), rte_atomic64_read(&udpi.stats.output_stats.l7.https), rte_atomic64_read(&udpi.stats.input_stats.l7.others), rte_atomic64_read(&udpi.stats.output_stats.l7.others));

	printf("\nin|sshbytes  out|sshbytes  in|httpbytes  out|httpbytes  in|httpsbytes  out|httpsbytes  in|othersbytes  out|othersbytes\n");	
	printf("0x%lx     0x%lx     0x%lx     0x%lx    0x%lx     0x%lx     0x%lx     0x%lx\n", rte_atomic64_read(&udpi.stats.input_stats.l7.ssh_bytes), rte_atomic64_read(&udpi.stats.output_stats.l7.ssh_bytes), rte_atomic64_read(&udpi.stats.input_stats.l7.http_bytes), rte_atomic64_read(&udpi.stats.output_stats.l7.http_bytes), rte_atomic64_read(&udpi.stats.input_stats.l7.https_bytes), rte_atomic64_read(&udpi.stats.output_stats.l7.https_bytes), rte_atomic64_read(&udpi.stats.input_stats.l7.others_bytes), rte_atomic64_read(&udpi.stats.output_stats.l7.others_bytes));

	printf("\nflows  closed_flows  tcp|flows  tcp|close_flows  tcp|rst_flows  udp|flows  udp|closed_flows  tcp|delay_ms  tcp|delay_flows\n");	
	printf("0x%lx    0x%lx    0x%lx    0x%lx     0x%lx     0x%lx     0x%lx     0x%lx     0x%lx\n", rte_atomic64_read(&udpi.stats.flows_stats.flow_entries), rte_atomic64_read(&udpi.stats.flows_stats.closed_flow_entries), rte_atomic64_read(&udpi.stats.flows_stats.tcp_flow_entries), rte_atomic64_read(&udpi.stats.flows_stats.tcp_closed_flow_entries), rte_atomic64_read(&udpi.stats.flows_stats.tcp_rst_flow_entries), rte_atomic64_read(&udpi.stats.flows_stats.udp_flow_entries), rte_atomic64_read(&udpi.stats.flows_stats.udp_closed_flow_entries), rte_atomic64_read(&udpi.stats.delay_stats.tcp_delay_ms), rte_atomic64_read(&udpi.stats.delay_stats.tcp_delay_flows));

	printf("\naliin   aliout   tecentin   tecentout    otherin     otherout\n");
	printf("0x%lx     0x%lx     0x%lx      0x%lx     0x%lx     0x%lx\n", rte_atomic64_read(&udpi.stats.input_comp_stats[0].ip_bytes), rte_atomic64_read(&udpi.stats.output_comp_stats[0].ip_bytes), rte_atomic64_read(&udpi.stats.input_comp_stats[1].ip_bytes), rte_atomic64_read(&udpi.stats.output_comp_stats[1].ip_bytes), rte_atomic64_read(&udpi.stats.input_comp_stats[2].ip_bytes), rte_atomic64_read(&udpi.stats.output_comp_stats[2].ip_bytes));
}

cmdline_parse_token_string_t cmd_udpi_allstats_udpi = 
	TOKEN_STRING_INITIALIZER(struct cmd_allstats_result, udpi, "udpi");
cmdline_parse_token_string_t cmd_udpi_allstats_allstats = 
	TOKEN_STRING_INITIALIZER(struct cmd_allstats_result, allstats, "allstats");
cmdline_parse_token_string_t cmd_udpi_allstats_show = 
	TOKEN_STRING_INITIALIZER(struct cmd_allstats_result, show, "show");

cmdline_parse_inst_t cmd_allstats_show = {
	.f = cmd_allstats_parsed,
	.data = NULL,
	.help_str = "udpi allstats show",
	.tokens = {
		(void *)&cmd_udpi_allstats_udpi,
		(void *)&cmd_udpi_allstats_allstats,
		(void *)&cmd_udpi_allstats_show,
		NULL,
	},
};

struct cmd_allmap_result {
	cmdline_fixed_string_t udpi;
	cmdline_fixed_string_t allmap;
	uint8_t region;
	uint8_t operator;
};

static void cmd_allmap_parsed(
	void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	struct cmd_allmap_result *res = parsed_result;
	uint8_t region, operator;

	region = res->region;
	operator = res->operator;
	if (region >= UDPI_REGION_STATS || operator >= UDPI_OPERATOR_STATS)
	{
		printf("region can't be bigger than %u. operator can't be bigger than %u\n", UDPI_REGION_STATS, UDPI_OPERATOR_STATS);
		return;
	}

	printf("in|ip  out|ip  in|udp  out|udp  in|tcp  out|tcp  in|retcp  out|retcp  in|tcpsyn  out|tcpsyn\n");	
	printf("0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx\n", rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l3.ip_pkts), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l3.ip_pkts), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l4.udp_pkts), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l4.udp_pkts), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l4.tcp_pkts), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l4.tcp_pkts), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l4.tcp_retransmit_pkts), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l4.tcp_retransmit_pkts), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l4.tcp_syn_pkts), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l4.tcp_syn_pkts));

	printf("\nin|ipbytes  out|ipbytes  in|udpbytes  out|udpbytes  in|tcpbytes  outtcp|bytes  in|retcpbyte  out|retcpbyte\n");	
	printf("0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx\n", rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l3.ip_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l3.ip_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l4.udp_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l4.udp_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l4.tcp_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l4.tcp_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l4.tcp_retransmit_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l4.tcp_retransmit_bytes));

	printf("\nin|ssh  out|ssh  in|http  out|http  in|https  out|https  in|others  out|outers\n");	
	printf("0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx\n", rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l7.ssh), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l7.ssh), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l7.http), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l7.http), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l7.https), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l7.https), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l7.others), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l7.others));

	printf("\nin|sshbytes  out|sshbytes  in|httpbytes  out|httpbytes  in|httpsbytes  out|httpsbytes  in|othersbytes  out|othersbytes\n");	
	printf("0x%lx     0x%lx     0x%lx     0x%lx    0x%lx     0x%lx     0x%lx     0x%lx\n", rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l7.ssh_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l7.ssh_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l7.http_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l7.http_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l7.https_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l7.https_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].input_stats.l7.others_bytes), rte_atomic64_read(&udpi.stats.map_stats[region][operator].output_stats.l7.others_bytes));

	printf("\nflows  closed_flows  tcp|flows  tcp|close_flows  tcp|rst_flows  udp|flows  udp|closed_flows  tcp|delay_ms  tcp|delay_flows\n");	
	printf("0x%lx    0x%lx    0x%lx    0x%lx     0x%lx     0x%lx     0x%lx     0x%lx     0x%lx\n", rte_atomic64_read(&udpi.stats.map_stats[region][operator].flows_stats.flow_entries), rte_atomic64_read(&udpi.stats.map_stats[region][operator].flows_stats.closed_flow_entries), rte_atomic64_read(&udpi.stats.map_stats[region][operator].flows_stats.tcp_flow_entries), rte_atomic64_read(&udpi.stats.map_stats[region][operator].flows_stats.tcp_closed_flow_entries), rte_atomic64_read(&udpi.stats.map_stats[region][operator].flows_stats.tcp_rst_flow_entries), rte_atomic64_read(&udpi.stats.map_stats[region][operator].flows_stats.udp_flow_entries), rte_atomic64_read(&udpi.stats.map_stats[region][operator].flows_stats.udp_closed_flow_entries), rte_atomic64_read(&udpi.stats.map_stats[region][operator].delay_stats.tcp_delay_ms), rte_atomic64_read(&udpi.stats.map_stats[region][operator].delay_stats.tcp_delay_flows));

}

cmdline_parse_token_string_t cmd_udpi_allmap_udpi = 
	TOKEN_STRING_INITIALIZER(struct cmd_allmap_result, udpi, "udpi");
cmdline_parse_token_string_t cmd_udpi_allmap_allmap = 
	TOKEN_STRING_INITIALIZER(struct cmd_allmap_result, allmap, "allmap");
cmdline_parse_token_num_t cmd_udpi_allmap_region =
        TOKEN_NUM_INITIALIZER(struct cmd_allmap_result, region, UINT8);
cmdline_parse_token_num_t cmd_udpi_allmap_operator =
        TOKEN_NUM_INITIALIZER(struct cmd_allmap_result, operator, UINT8);

cmdline_parse_inst_t cmd_allmap_show = {
	.f = cmd_allmap_parsed,
	.data = NULL,
	.help_str = "udpi allmap region_id operator_id",
	.tokens = {
		(void *)&cmd_udpi_allmap_udpi,
		(void *)&cmd_udpi_allmap_allmap,
		(void *)&cmd_udpi_allmap_region,
		(void *)&cmd_udpi_allmap_operator,
		NULL,
	},
};

struct cmd_eipstats_result {
	cmdline_fixed_string_t udpi;
	cmdline_fixed_string_t eipstats;
	cmdline_ipaddr_t eip;
};

static void cmd_eipstats_parsed(
	void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	struct cmd_eipstats_result *res = (struct cmd_eipstats_result *)parsed_result;

	uint32_t eip = rte_be_to_cpu_32(res->eip.addr.ipv4.s_addr);

	uint32_t hashed = rte_jhash_1word(eip, 0);	
	hashed %= udpi.n_workers;
	
	struct eip_key key;
	key.eip = eip;
	struct eip_entry *entry;
	int ret;
	ret = rte_hash_lookup(udpi.hash[hashed].eip_hash, (const void*)&key);
	if (ret >= 0)
	{
		printf("ret is %d, %u\n", ret, hashed);
		entry = udpi.hash[hashed].eip_entry;
		entry = entry + ret;

		printf("eip 0x%x stats:\n", entry->eip.eip);
		printf("in|ip  out|ip  in|udp  out|udp  in|tcp  out|tcp  in|retcp  out|retcp  in|tcpsyn  out|tcpsyn\n");	
		printf("0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx   0x%lx  0x%lx\n", entry->stats.input_stats.l3.ip_pkts, entry->stats.output_stats.l3.ip_pkts, entry->stats.input_stats.l4.udp_pkts, entry->stats.output_stats.l4.udp_pkts, entry->stats.input_stats.l4.tcp_pkts, entry->stats.output_stats.l4.tcp_pkts, entry->stats.input_stats.l4.tcp_retransmit_pkts, entry->stats.output_stats.l4.tcp_retransmit_pkts, entry->stats.input_stats.l4.tcp_syn_pkts, entry->stats.output_stats.l4.tcp_syn_pkts);

		printf("\nin|ipbytes  out|ipbytes  in|udpbytes  out|udpbytes  in|tcpbytes  outtcp|bytes  in|retcpbyte  out|retcpbyte\n");	
		printf("0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx\n", entry->stats.input_stats.l3.ip_bytes, entry->stats.output_stats.l3.ip_bytes, entry->stats.input_stats.l4.udp_bytes, entry->stats.output_stats.l4.udp_bytes, entry->stats.input_stats.l4.tcp_bytes, entry->stats.output_stats.l4.tcp_bytes, entry->stats.input_stats.l4.tcp_retransmit_bytes, entry->stats.output_stats.l4.tcp_retransmit_bytes);

		printf("\nin|ssh  out|ssh  in|http  out|http  in|https  out|https  in|others  out|outers\n");	
		printf("0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx\n", entry->stats.input_stats.l7.ssh, entry->stats.output_stats.l7.ssh, entry->stats.input_stats.l7.http, entry->stats.output_stats.l7.http, entry->stats.input_stats.l7.https, entry->stats.output_stats.l7.https, entry->stats.input_stats.l7.others, entry->stats.output_stats.l7.others);

		printf("\nin|sshbytes  out|sshbytes  in|httpbytes  out|httpbytes  in|httpsbytes  out|httpsbytes  in|othersbytes  out|othersbytes\n");	
		printf("0x%lx     0x%lx     0x%lx     0x%lx    0x%lx     0x%lx     0x%lx     0x%lx\n", entry->stats.input_stats.l7.ssh_bytes, entry->stats.output_stats.l7.ssh_bytes, entry->stats.input_stats.l7.http_bytes, entry->stats.output_stats.l7.http_bytes, entry->stats.input_stats.l7.https_bytes, entry->stats.output_stats.l7.https_bytes, entry->stats.input_stats.l7.others_bytes, entry->stats.output_stats.l7.others_bytes);

		printf("\nflows  closed_flows  tcp|flows  tcp|close_flows  tcp|rst_flows  udp|flows  udp|closed_flows  tcp|delay_ms  tcp|delay_flows\n");	
		printf("0x%lx    0x%lx    0x%lx    0x%lx     0x%lx     0x%lx     0x%lx     0x%lx     0x%lx\n", entry->stats.flows_stats.flow_entries, entry->stats.flows_stats.closed_flow_entries, entry->stats.flows_stats.tcp_flow_entries, entry->stats.flows_stats.tcp_closed_flow_entries, entry->stats.flows_stats.tcp_rst_flow_entries, entry->stats.flows_stats.udp_flow_entries, entry->stats.flows_stats.udp_closed_flow_entries, entry->stats.delay_stats.tcp_delay_ms, entry->stats.delay_stats.tcp_delay_flows);

		printf("\nhttp_ms  http_times  http_code_1xx  http_code_2xx  http_code_3xx  http_code_4xx  http_code_5xx\n");
		printf("0x%lx    0x%lx    0x%lx    0x%lx     0x%lx     0x%lx     0x%lx\n", entry->stats.http_stats.http_delay_ms, entry->stats.http_stats.http_delay_flows, entry->stats.http_stats.http_return[0], entry->stats.http_stats.http_return[1], entry->stats.http_stats.http_return[2], entry->stats.http_stats.http_return[3], entry->stats.http_stats.http_return[4]);

		printf("\naliin   aliout   tecentin   tecentout\n");
		printf("0x%lx    0x%lx,    0x%lx      0x%lx\n", entry->stats.input_comp_stats[0].ip_bytes, entry->stats.output_comp_stats[0].ip_bytes, entry->stats.input_comp_stats[1].ip_bytes, entry->stats.output_comp_stats[1].ip_bytes);
	}
	else
	{
		printf("don't find the ipaddr\n");
	}

}

cmdline_parse_token_string_t cmd_udpi_eipstats_udpi = 
	TOKEN_STRING_INITIALIZER(struct cmd_eipstats_result, udpi, "udpi");
cmdline_parse_token_string_t cmd_udpi_eipstats_eipstats = 
	TOKEN_STRING_INITIALIZER(struct cmd_eipstats_result, eipstats, "eipstats");
cmdline_parse_token_ipaddr_t cmd_udpi_eipstats_eip =
        TOKEN_IPADDR_INITIALIZER(struct cmd_eipstats_result, eip);

cmdline_parse_inst_t cmd_eipstats_show = {
	.f = cmd_eipstats_parsed,
	.data = NULL,
	.help_str = "udpi eipstats ip_address",
	.tokens = {
		(void *)&cmd_udpi_eipstats_udpi,
		(void *)&cmd_udpi_eipstats_eipstats,
		(void *)&cmd_udpi_eipstats_eip,
		NULL,
	},
};


struct cmd_eipmap_result {
	cmdline_fixed_string_t udpi;
	cmdline_fixed_string_t eipmap;
	cmdline_ipaddr_t eip;
	uint8_t region;
	uint8_t operator;
};

static void cmd_eipmap_parsed(
	void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	struct cmd_eipmap_result *res = (struct cmd_eipmap_result *)parsed_result;
	uint8_t region, operator;

	region = res->region;
	operator = res->operator;
	if (region >= UDPI_REGION_STATS || operator >= UDPI_OPERATOR_STATS)
	{
		printf("region can't be bigger than %u. operator can't be bigger than %u\n", UDPI_REGION_STATS, UDPI_OPERATOR_STATS);
		return;
	}

	uint32_t eip = rte_be_to_cpu_32(res->eip.addr.ipv4.s_addr);

	uint32_t hashed = rte_jhash_1word(eip, 0);	
	hashed %= udpi.n_workers;
	
	struct eip_key key;
	key.eip = eip;
	struct eip_entry *entry;
	int ret;
	ret = rte_hash_lookup(udpi.hash[hashed].eip_hash, (const void*)&key);
	if (ret >= 0)
	{
		printf("ret is %d, %u\n", ret, hashed);
		entry = udpi.hash[hashed].eip_entry;
		entry = entry + ret;

		printf("eip 0x%x map[%u][%u] stats:\n", entry->eip.eip, region, operator);
		printf("in|ip  out|ip  in|udp  out|udp  in|tcp  out|tcp  in|retcp  out|retcp  in|tcpsyn  out|tcpsyn\n");	
		printf("0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx\n", entry->stats.map_stats[region][operator].input_stats.l3.ip_pkts, entry->stats.map_stats[region][operator].output_stats.l3.ip_pkts, entry->stats.map_stats[region][operator].input_stats.l4.udp_pkts, entry->stats.map_stats[region][operator].output_stats.l4.udp_pkts, entry->stats.map_stats[region][operator].input_stats.l4.tcp_pkts, entry->stats.map_stats[region][operator].output_stats.l4.tcp_pkts, entry->stats.map_stats[region][operator].input_stats.l4.tcp_retransmit_pkts, entry->stats.map_stats[region][operator].output_stats.l4.tcp_retransmit_pkts, entry->stats.map_stats[region][operator].input_stats.l4.tcp_syn_pkts, entry->stats.map_stats[region][operator].output_stats.l4.tcp_syn_pkts);

		printf("\nin|ipbytes  out|ipbytes  in|udpbytes  out|udpbytes  in|tcpbytes  outtcp|bytes  in|retcpbyte  out|retcpbyte\n");	
		printf("0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx    0x%lx\n", entry->stats.map_stats[region][operator].input_stats.l3.ip_bytes, entry->stats.map_stats[region][operator].output_stats.l3.ip_bytes, entry->stats.map_stats[region][operator].input_stats.l4.udp_bytes, entry->stats.map_stats[region][operator].output_stats.l4.udp_bytes, entry->stats.map_stats[region][operator].input_stats.l4.tcp_bytes, entry->stats.map_stats[region][operator].output_stats.l4.tcp_bytes, entry->stats.map_stats[region][operator].input_stats.l4.tcp_retransmit_bytes, entry->stats.map_stats[region][operator].output_stats.l4.tcp_retransmit_bytes);

		printf("\nin|ssh  out|ssh  in|http  out|http  in|https  out|https  in|others  out|outers\n");	
		printf("0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx  0x%lx\n", entry->stats.map_stats[region][operator].input_stats.l7.ssh,entry->stats.map_stats[region][operator].output_stats.l7.ssh, entry->stats.map_stats[region][operator].input_stats.l7.http, entry->stats.map_stats[region][operator].output_stats.l7.http, entry->stats.map_stats[region][operator].input_stats.l7.https, entry->stats.map_stats[region][operator].output_stats.l7.https, entry->stats.map_stats[region][operator].input_stats.l7.others, entry->stats.map_stats[region][operator].output_stats.l7.others);

		printf("\nin|sshbytes  out|sshbytes  in|httpbytes  out|httpbytes  in|httpsbytes  out|httpsbytes  in|othersbytes  out|othersbytes\n");	
		printf("0x%lx     0x%lx     0x%lx     0x%lx    0x%lx     0x%lx     0x%lx     0x%lx\n", entry->stats.map_stats[region][operator].input_stats.l7.ssh_bytes, entry->stats.map_stats[region][operator].output_stats.l7.ssh_bytes, entry->stats.map_stats[region][operator].input_stats.l7.http_bytes, entry->stats.map_stats[region][operator].output_stats.l7.http_bytes, entry->stats.map_stats[region][operator].input_stats.l7.https_bytes, entry->stats.map_stats[region][operator].output_stats.l7.https_bytes, entry->stats.map_stats[region][operator].input_stats.l7.others_bytes, entry->stats.map_stats[region][operator].output_stats.l7.others_bytes);

		printf("\nflows  closed_flows  tcp|flows  tcp|close_flows  tcp|rst_flows  udp|flows  udp|closed_flows  tcp|delay_ms  tcp|delay_flows\n");	
		printf("0x%lx    0x%lx    0x%lx    0x%lx     0x%lx     0x%lx     0x%lx     0x%lx     0x%lx\n", entry->stats.map_stats[region][operator].flows_stats.flow_entries, entry->stats.map_stats[region][operator].flows_stats.closed_flow_entries, entry->stats.map_stats[region][operator].flows_stats.tcp_flow_entries, entry->stats.map_stats[region][operator].flows_stats.tcp_closed_flow_entries, entry->stats.map_stats[region][operator].flows_stats.tcp_rst_flow_entries, entry->stats.map_stats[region][operator].flows_stats.udp_flow_entries, entry->stats.map_stats[region][operator].flows_stats.udp_closed_flow_entries, entry->stats.map_stats[region][operator].delay_stats.tcp_delay_ms, entry->stats.map_stats[region][operator].delay_stats.tcp_delay_flows);
	}
	else
	{
		printf("don't find the ipaddr\n");
	}

}

cmdline_parse_token_string_t cmd_udpi_eipmap_udpi = 
	TOKEN_STRING_INITIALIZER(struct cmd_eipmap_result, udpi, "udpi");
cmdline_parse_token_string_t cmd_udpi_eipmap_eipmap = 
	TOKEN_STRING_INITIALIZER(struct cmd_eipmap_result, eipmap, "eipmap");
cmdline_parse_token_ipaddr_t cmd_udpi_eipmap_eip =
        TOKEN_IPADDR_INITIALIZER(struct cmd_eipmap_result, eip);
cmdline_parse_token_num_t cmd_udpi_eipmap_region =
        TOKEN_NUM_INITIALIZER(struct cmd_eipmap_result, region, UINT8);
cmdline_parse_token_num_t cmd_udpi_eipmap_operator =
        TOKEN_NUM_INITIALIZER(struct cmd_eipmap_result, operator, UINT8);

cmdline_parse_inst_t cmd_eipmap_show = {
	.f = cmd_eipmap_parsed,
	.data = NULL,
	.help_str = "udpi eipmap ip_address region_id operator_id",
	.tokens = {
		(void *)&cmd_udpi_eipmap_udpi,
		(void *)&cmd_udpi_eipmap_eipmap,
		(void *)&cmd_udpi_eipmap_eip,
		(void *)&cmd_udpi_eipmap_region,
		(void *)&cmd_udpi_eipmap_operator,
		NULL,
	},
};


struct cmd_flow_state_result {
	cmdline_fixed_string_t udpi;
	cmdline_ipaddr_t eip;
	cmdline_ipaddr_t srcip;
	cmdline_ipaddr_t dstip;
	uint16_t srcport;
	uint16_t dstport;
	uint8_t proto;
};

cmdline_parse_token_string_t cmd_udpi_flow_state_udpi = 
	TOKEN_STRING_INITIALIZER(struct cmd_flow_state_result, udpi, "udpi");
cmdline_parse_token_string_t cmd_udpi_flow_state_flowstate = 
	TOKEN_STRING_INITIALIZER(struct cmd_flow_state_result, udpi, "flow_state");
cmdline_parse_token_ipaddr_t cmd_udpi_flow_state_eip = 
        TOKEN_IPADDR_INITIALIZER(struct cmd_flow_state_result, eip);
cmdline_parse_token_ipaddr_t cmd_udpi_flow_state_srcip = 
        TOKEN_IPADDR_INITIALIZER(struct cmd_flow_state_result, srcip);
cmdline_parse_token_ipaddr_t cmd_udpi_flow_state_dstip =
        TOKEN_IPADDR_INITIALIZER(struct cmd_flow_state_result, dstip);
cmdline_parse_token_num_t cmd_udpi_flow_state_srcport =
        TOKEN_NUM_INITIALIZER(struct cmd_flow_state_result, srcport, UINT16);
cmdline_parse_token_num_t cmd_udpi_flow_state_dstport =
        TOKEN_NUM_INITIALIZER(struct cmd_flow_state_result, dstport, UINT16);
cmdline_parse_token_num_t cmd_udpi_flow_state_proto =
        TOKEN_NUM_INITIALIZER(struct cmd_flow_state_result, proto, UINT8);

static void cmd_flow_state_parsed(
	void *parsed_result,
	__attribute__((unused)) struct cmdline *cl,
	__attribute__((unused)) void *data)
{
	struct cmd_flow_state_result *res = (struct cmd_flow_state_result *)parsed_result;
	
	uint32_t eip = rte_be_to_cpu_32(res->eip.addr.ipv4.s_addr);
	uint32_t srcip = rte_be_to_cpu_32(res->srcip.addr.ipv4.s_addr);
	uint32_t dstip = rte_be_to_cpu_32(res->dstip.addr.ipv4.s_addr);
	uint16_t srcport = res->srcport;
	uint16_t dstport = res->dstport;
	uint8_t proto = res->proto;

	if ((srcip != eip) && (dstip != eip))
	{
		printf("eip must be srcip or dstip\n");
		return;
	}

	uint32_t hashed = rte_jhash_1word(eip, 0);	
	hashed %= udpi.n_workers;
	
	struct eip_key key;
	key.eip = eip;
	int ret;
	ret = rte_hash_lookup(udpi.hash[hashed].eip_hash, (const void*)&key);
	if (ret < 0)
	{
		printf("no eip\n");
		return;
	}

	struct flow_key fkey;

	if(srcip < dstip) 
	{
		fkey.lower_ip = srcip;
		fkey.upper_ip = dstip;
		if (srcport < dstport)
			fkey.diff = 0;
		else
			fkey.diff = 1;
	} 
	else 
	{
		fkey.lower_ip = dstip;
		fkey.upper_ip = srcip;
		if (srcport > dstport)
			fkey.diff = 0;
		else
			fkey.diff = 1;
	}

	if(srcport < dstport) 
	{
		fkey.lower_port = srcport;
		fkey.upper_port = dstport;
	} 
	else 
	{
		fkey.lower_port = dstport;
		fkey.upper_port = srcport;
	}
	fkey.proto = proto;

	struct flow_entry **pfentry = udpi.hash[hashed].flow_entry;
	struct flow_entry *fentry;
	ret = rte_hash_lookup(udpi.hash[hashed].flow_hash, (const void*)&fkey);
	if (ret < 0)
	{
		printf("no flows\n");
	}
	else
	{
		fentry = pfentry[ret];
		if (!fentry)
		{
			printf("no flows in ret\n");
		}
		else
		{
			printf("flow status %d\n", fentry->state);
		}
	}
}

cmdline_parse_inst_t cmd_flow_state_show = {
	.f = cmd_flow_state_parsed,
	.data = NULL,
	.help_str = "udpi flowstate eip src_ip dst_ip src_port dst_port proto",
	.tokens = {
		(void *)&cmd_udpi_flow_state_udpi,
		(void *)&cmd_udpi_flow_state_flowstate,
		(void *)&cmd_udpi_flow_state_eip,
		(void *)&cmd_udpi_flow_state_srcip,
		(void *)&cmd_udpi_flow_state_dstip,
		(void *)&cmd_udpi_flow_state_srcport,
		(void *)&cmd_udpi_flow_state_dstport,
		(void *)&cmd_udpi_flow_state_proto,
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
	(cmdline_parse_inst_t *)&cmd_ring_show,
	(cmdline_parse_inst_t *)&cmd_flows_show,
	(cmdline_parse_inst_t *)&cmd_load_show,
	(cmdline_parse_inst_t *)&cmd_allstats_show,
	(cmdline_parse_inst_t *)&cmd_allmap_show,
	(cmdline_parse_inst_t *)&cmd_eipstats_show,
	(cmdline_parse_inst_t *)&cmd_eipmap_show,
	(cmdline_parse_inst_t *)&cmd_flow_state_show,
	(cmdline_parse_inst_t *)&cmd_debug,
	(cmdline_parse_inst_t *)&cmd_quit,
	NULL,
};


#define EVENT_SIZE  (sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

static const char* watch_dir = "/root/watch_dir/";
static const char* ips_file = "/root/watch_dir/ips_list";
static const char* ips_file_name = "ips_list";
static const char* comp_file = "/root/watch_dir/comp_list";
static const char* comp_file_name = "comp_list";
static int inotify_fd;
static int watch_fd;

static struct ev_loop * loop;
static struct ev_io inotify_watcher;
static struct ev_io cmdline_watcher;

static struct cmdline *cl = NULL;

static uint8_t key_data[UDPI_COMP_MAX];
static uint8_t key_packet_data[2];

void udpi_task_complist(void)
{
	FILE *fp;

	if ((fp = fopen(comp_file, "r")) == NULL)
	{   
		printf("File open Error %d\n", errno);
		return;
	}   

	uint8_t id = !(udpi.comp_hash_id);

	rte_hash_reset(udpi.comp_addr_hash[id]);

	uint32_t count = 0;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char *token;
	int j;
	const char *delim = " ";
	char *str1;

	while ((read = getline(&line, &len, fp)) != -1) 
	{
		uint32_t key;
		uint8_t comp_type;
		for (j = 0, str1 = line; ; j++, str1 = NULL) {
			token = strtok(str1, delim);
			if (token == NULL)
				break;
			if (j == 0)
			{
				uint32_t a = 0, b = 0, c = 0, d = 0;
				int ret = sscanf(token, "%u.%u.%u.%u", &a, &b, &c, &d);
				if (ret == 0 || ret == EOF)
				{
					printf("sccanf error %s\n", token);
					break;
				}
				key = (a<<24)|(b<<16)|(c<<8)|d;
			}   
			else if (j == 1)
			{
				uint64_t tmp;
 				tmp = strtoul(token, NULL, 0);
				if (tmp >= UDPI_COMP_MAX)
				{
					printf("comp_type fail %lu\n", tmp);
					break;
				}
				comp_type = tmp;
			}
		}

		if (j == 2)
		{
			//TODO
			int ret = rte_hash_add_key_data(udpi.comp_addr_hash[id], (const void*)&key, (void*)(&key_data[comp_type]));	
			if (ret < 0)
			{
				printf("comp hash add key error %d for key %x\n", ret, key);
				continue;
			}
				
			count ++;
		} 
	}

	free(line);

	if (unlikely(udpi.debug))
		printf("add comp ip list count %u in id %u\n", count, id);

	if (count)
		udpi.comp_hash_id = id;

	fclose(fp);
}


void udpi_task_iplist(void)
{
	FILE *fp;

	if ((fp = fopen(ips_file, "r")) == NULL)
	{   
		printf("File open Error %d\n", errno);
		return;
	}   

	uint8_t id = !(udpi.hash_id);

	rte_hash_reset(udpi.addr_hash[id]);

	uint32_t count = 0;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	char *token;
	int j;
	const char *delim = " ";
	char *str1;

	while ((read = getline(&line, &len, fp)) != -1) 
	{
		uint32_t key = 0;
		uint8_t packet_type = 0;
		for (j = 0, str1 = line; ; j++, str1 = NULL) {
			token = strtok(str1, delim);
			if (token == NULL)
				break;
			if (j == 0)
			{
				uint32_t a = 0, b = 0, c = 0, d = 0;
				int ret = sscanf(token, "%u.%u.%u.%u", &a, &b, &c, &d);
				if (ret == 0 || ret == EOF)
				{
					printf("sccanf error %s\n", token);
					break;
				}
				key = (a<<24)|(b<<16)|(c<<8)|d;
			}   
			else if (j == 1)
			{
				uint64_t tmp;
 				tmp = strtoul(token, NULL, 0);
				if (tmp > 1)
				{
					printf("packet_type err %lu\n", tmp);
					tmp = 0;
				}
		
				packet_type = tmp;
			}
		}

		if (key)
		{
			//TODO
			int ret = rte_hash_add_key_data(udpi.addr_hash[id], (const void*)&key, (void*)(&key_packet_data[packet_type]));	
			if (ret < 0)
			{
				printf("hash add key error %d for key %x type %u\n", ret, key, packet_type);
				continue;
			}
				
			count ++;
		} 
	}

	if (unlikely(udpi.debug))
		printf("add ip list count %u in id %u\n", count, id);

	if (count)
		udpi.hash_id = id;

	fclose(fp);
}

static int udpi_cmdline_interact(struct cmdline *cl)
{
	char c;

	if (!cl)
		return -1;

	c = -1;
	while (1) {
		if (read(cl->s_in, &c, 1) <= 0)
			break;
		
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
inotify_cb (__attribute__((unused)) struct ev_loop *loop, ev_io *w, int revents)
{
	if ((revents & EV_READ) && (w->fd == inotify_fd))
	{
		int length, j = 0;
		char buffer[EVENT_BUF_LEN];
		length = read(w->fd, buffer, EVENT_BUF_LEN);
		if (length <= 0)
			return;

		while (j < length) 
		{
    		struct inotify_event *event = (struct inotify_event*)&buffer[j];
			if (event->len)
			{
				if (strcmp(event->name, ips_file_name) == 0) 
				{    
					udpi_task_iplist();
				}
				else if (strcmp(event->name, comp_file_name) == 0) 
				{    
					udpi_task_complist();
				}    
			}
    				
			j += EVENT_SIZE + event->len;
		}
	}
}

static void
cmdline_cb (__attribute__((unused)) struct ev_loop *loop, ev_io *w, int revents)
{
	if ((revents & EV_READ) && (w->fd == cl->s_in))
	{
		int err = udpi_cmdline_interact(cl);
		if (err)
		{
			inotify_rm_watch(inotify_fd, watch_fd);	
			close(inotify_fd);
			cmdline_stdin_exit(cl);
			//TODO
			exit(1);
		}
	}
}

static int udpi_task_init(struct cmdline *cl)
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

	ev_init(&inotify_watcher, inotify_cb);
	ev_io_set(&inotify_watcher, inotify_fd, EV_READ);
	ev_io_start(loop, &inotify_watcher);

	ev_init(&cmdline_watcher, cmdline_cb);
	ev_io_set(&cmdline_watcher, cl->s_in, EV_READ);
	ev_io_start(loop, &cmdline_watcher);

	return 0;
}

void udpi_main_loop_task(void)
{
	uint32_t core_id = rte_lcore_id();
	
	struct udpi_core_params *core_params 
		= udpi_get_core_params(core_id);

	uint8_t i;
	for (i=0; i<UDPI_COMP_MAX; i++)
	{
		key_data[i] = i;	
	}

	for (i=0; i<2; i++)
	{
		key_packet_data[i] = i;	
	}

	if((!core_params) 
		|| (core_params->core_type != UDPI_CORE_TASK))
	{
		rte_panic("Core %u misconfiguration\n", core_id);
		return;
	}

	RTE_LOG(INFO, USER1, "Core %u is doing task\n", core_id);

	cl = cmdline_stdin_new(main_ctx, "[udpi]>> ");
	if(cl == NULL)
	{
		rte_panic("Core %u cmdline init fail\n", core_id);
		return;
	}
	
	int err = udpi_task_init(cl);
	if (err < 0)
	{
		rte_panic("Core %u task init fail\n", core_id);
		return;
	}

	ev_run(loop, 0);

	return;
}

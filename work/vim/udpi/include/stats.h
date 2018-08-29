#ifndef _UDPI_STATS_H_
#define _UDPI_STATS_H_

#include <rte_atomic.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "ipaddr_map.h"

#define UDPI_STATS_ITEMS 43
#define UDPI_MAP_STATS_ITEMS 43

#define UDPI_EIP_STATS_ITEMS 52
#define UDPI_EIP_MAP_STATS_ITEMS 43

#if REGIONS == DB_XJ
#define UDPI_REGION_STATS 52
#define UDPI_OPERATOR_STATS 7
#define UDPI_LINE_STATS   6
#define LINE_MAC_S_ADD (eth_hdr->s_addr)
#define LINE_MAC_D_ADD (eth_hdr->d_addr)

#else 
#define UDPI_REGION_STATS 39
#define UDPI_OPERATOR_STATS 7
#define UDPI_LINE_STATS   7
#define UDPI_REGION_UNKNOW  35
#define UDPI_OPERATOR_UNKNOW  5
#define LINE_MAC_S_ADD (eth_hdr->d_addr)
#define LINE_MAC_D_ADD (eth_hdr->s_addr)
#endif

#define UDPI_COMP_MAX 3

enum L7_PROTOCOL
{
        PROTOCOL_SSH=22,
        PROTOCOL_HTTP=80,
        PROTOCOL_HTTPS=443,
        PROTOCOL_OTHERS,
};

struct l7_stats
{
        rte_atomic64_t ssh;
        rte_atomic64_t ssh_bytes;
        rte_atomic64_t http;
        rte_atomic64_t http_bytes;
        rte_atomic64_t https;
        rte_atomic64_t https_bytes;
        rte_atomic64_t others;
        rte_atomic64_t others_bytes;
};

struct l4_stats
{
        rte_atomic64_t udp_pkts;
        rte_atomic64_t udp_bytes;
        rte_atomic64_t tcp_pkts;
        rte_atomic64_t tcp_bytes;
        rte_atomic64_t tcp_syn_pkts;
        rte_atomic64_t tcp_retransmit_pkts;
        rte_atomic64_t tcp_retransmit_bytes;
};

struct ip_stats
{
        rte_atomic64_t ip_pkts;
        rte_atomic64_t ip_bytes;
};


struct all_stats
{
        struct ip_stats l3;
        struct l4_stats l4;
        struct l7_stats l7;
};

struct comp_stats
{
	rte_atomic64_t ip_bytes;
};

struct connection_stats
{
        rte_atomic64_t flow_entries;
        rte_atomic64_t closed_flow_entries;
        rte_atomic64_t tcp_flow_entries;
        rte_atomic64_t tcp_closed_flow_entries;
        rte_atomic64_t tcp_rst_flow_entries;
        rte_atomic64_t udp_flow_entries;
        rte_atomic64_t udp_closed_flow_entries;
};

struct delay_stats
{
        rte_atomic64_t tcp_delay_ms;
        rte_atomic64_t tcp_delay_flows;
};

struct map_stats
{
	struct all_stats input_stats;
	struct all_stats output_stats;
	struct connection_stats flows_stats;
	struct delay_stats delay_stats;	
	uint8_t seen;
};
struct udpi_line_stats
{
	struct all_stats input_stats;
	struct all_stats output_stats;
	struct connection_stats flows_stats;
	struct delay_stats delay_stats;	
	struct map_stats map_stats[UDPI_REGION_STATS][UDPI_OPERATOR_STATS];

};
struct udpi_stats
{
	struct all_stats input_stats;
	struct all_stats output_stats;
	struct connection_stats flows_stats;
	struct delay_stats delay_stats;	
	struct comp_stats input_comp_stats[UDPI_COMP_MAX];
	struct comp_stats output_comp_stats[UDPI_COMP_MAX];
	struct map_stats map_stats[UDPI_REGION_STATS][UDPI_OPERATOR_STATS];
	struct udpi_line_stats line_stats[UDPI_LINE_STATS];
};

/*
struct udpi_eip_stats
{
	struct all_stats input_stats;
	struct all_stats output_stats;
	struct connection_stats flows_stats;
	struct delay_stats delay_stats;	
	
	struct map_stats map_stats[UDPI_REGION_STATS][UDPI_OPERATOR_STATS];
	uint8_t seen;
};
*/

struct l7_local_stats
{
        uint64_t ssh;
        uint64_t ssh_bytes;
        uint64_t http;
        uint64_t http_bytes;
        uint64_t https;
        uint64_t https_bytes;
        uint64_t others;
        uint64_t others_bytes;
};

struct http_local_stats
{
        uint64_t http_delay_ms;
        uint64_t http_delay_flows;
        uint64_t http_return[5];
        uint64_t http_code[600];
        uint8_t http_code_seen[600];
};

struct l4_local_stats
{
        uint64_t udp_pkts;
        uint64_t udp_bytes;
        uint64_t tcp_pkts;
        uint64_t tcp_bytes;
        uint64_t tcp_syn_pkts;
        uint64_t tcp_retransmit_pkts;
        uint64_t tcp_retransmit_bytes;
};

struct ip_local_stats
{
        uint64_t ip_pkts;
        uint64_t ip_bytes;
};


struct all_local_stats
{
        struct ip_local_stats l3;
        struct l4_local_stats l4;
        struct l7_local_stats l7;
};

struct connection_local_stats
{
        uint64_t flow_entries;
        uint64_t closed_flow_entries;
        uint64_t tcp_flow_entries;
        uint64_t tcp_closed_flow_entries;
        uint64_t tcp_rst_flow_entries;
        uint64_t udp_flow_entries;
        uint64_t udp_closed_flow_entries;
};

struct comp_local_stats
{
        uint64_t ip_bytes;
};

struct delay_local_stats
{
        uint64_t tcp_delay_ms;
        uint64_t tcp_delay_flows;
};

struct map_local_stats
{
	struct all_local_stats input_stats;
	struct all_local_stats output_stats;
	struct connection_local_stats flows_stats;
	struct delay_local_stats delay_stats;	
	uint8_t seen;
};
//line
struct udpi_local_line_stats
{
	struct all_local_stats input_stats;
	struct all_local_stats output_stats;
	struct connection_local_stats flows_stats;
	struct delay_local_stats delay_stats;	
	struct map_local_stats map_stats[UDPI_REGION_STATS][UDPI_OPERATOR_STATS];

};
//all
struct udpi_local_stats
{
	struct all_local_stats input_stats;
	struct all_local_stats output_stats;
	struct connection_local_stats flows_stats;
	struct delay_local_stats delay_stats;	
	struct comp_local_stats input_comp_stats[UDPI_COMP_MAX];
	struct comp_local_stats output_comp_stats[UDPI_COMP_MAX];

	struct map_local_stats map_stats[UDPI_REGION_STATS][UDPI_OPERATOR_STATS];
	struct udpi_local_line_stats line_stats[UDPI_LINE_STATS];
};

struct udpi_local_port_stats
{
	uint64_t  port_list[65536];
	uint8_t  port_list_seen[65536];
};

struct udpi_eip_local_stats
{
	struct all_local_stats input_stats;
	struct all_local_stats output_stats;
	struct connection_local_stats flows_stats;
	struct delay_local_stats delay_stats;
	struct http_local_stats http_stats;
	struct udpi_local_port_stats *port_stats;
	struct comp_local_stats input_comp_stats[UDPI_COMP_MAX];
	struct comp_local_stats output_comp_stats[UDPI_COMP_MAX];
	
	struct map_local_stats map_stats[UDPI_REGION_STATS][UDPI_OPERATOR_STATS];
	
	uint8_t seen;
};


void udpi_stats_init(struct udpi_stats *stats);
//void udpi_eipstats_init(struct udpi_eip_stats *stats);

#ifdef __cplusplus
}
#endif

#endif


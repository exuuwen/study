/*-
 *
 * Copyright(c) UCloud. All rights reserved.
 * All rights reserved.
 *
 */

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <rte_malloc.h>
#include <rte_lcore.h>
#include <rte_port.h>
#include <rte_cycles.h>
#include <rte_hash.h>
#include <rte_tcp.h>

#include "main.h"
#include "stats.h"
#include "dpi.h"

#define CPS 10000
#define TOKEN_PER_C  1000
#define MSEC_PER_SEC 1000
#define BURST ((CPS)*(TOKEN_PER_C))

static int tsc_mhz[UDPI_MAX_WORKERS];

/*
 *	Timeout table[state]
 */
static unsigned int udpi_tcp_timeouts[TCP_CONNTRACK_MAX] = {
	[TCP_CONNTRACK_SYN_SENT]	= 2*60,
	[TCP_CONNTRACK_SYN_RECV]	= 60,
	[TCP_CONNTRACK_ESTABLISHED]	= 15*60,
	[TCP_CONNTRACK_FIN_WAIT]	= 2*60,
	[TCP_CONNTRACK_CLOSE_WAIT]	= 2*60,
	[TCP_CONNTRACK_LAST_ACK]	= 60,
	[TCP_CONNTRACK_TIME_WAIT]	= 30,
	[TCP_CONNTRACK_CLOSE]		= 10,
	[TCP_CONNTRACK_SYN_SENT2]	= 2*60,
};

/*
 * The TCP state transition table needs a few words...
 *
 * We are the man in the middle. All the packets go through us
 * but might get lost in transit to the destination.
 * It is assumed that the destinations can't receive segments
 * we haven't seen.
 *
 * The checked segment is in window, but our windows are *not*
 * equivalent with the ones of the sender/receiver. We always
 * try to guess the state of the current sender.
 * 
 * The meaning of the states are:
 *
 * NONE:	initial state
 * SYN_SENT:	SYN-only packet seen
 * SYN_SENT2:	SYN-only packet seen from reply dir, simultaneous open
 * SYN_RECV:	SYN-ACK packet seen
 * ESTABLISHED:	ACK packet seen
 * FIN_WAIT:	FIN packet seen
 * CLOSE_WAIT:	ACK seen (after FIN)
 * LAST_ACK:	FIN seen (after FIN)
 * TIME_WAIT:	last ACK seen
 * CLOSE:	closed connection (RST)
 *
 * Packets marked as IGNORED (sIG):
 * if they may be either invalid or valid
 * and the receiver may send back a connection
 * closing RST or a SYN/ACK.
 *
 * Packets marked as INVALID (sIV):
 *	if we regard them as truly invalid packets
 */
static const uint8_t tcp_conntracks[2][6][TCP_CONNTRACK_MAX] = {
	{
/* ORIGINAL */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*syn*/	   { sSS, sSS, sIG, sIG, sIG, sIG, sIG, sSS, sSS, sS2 },
/*
 *	sNO -> sSS	Initialize a new connection
 *	sSS -> sSS	Retransmitted SYN
 *	sS2 -> sS2	Late retransmitted SYN
 * 	sSR -> sIG
 * 	sES -> sIG	Error: SYNs in window outside the SYN_SENT state
 * 		   are errors. Receiver will reply with RST
 * 	       and close the connection.
 *         Or we are not in sync and hold a dead connection.
 *  sFW -> sIG
 *  sCW -> sIG
 *  sLA -> sIG
 *	sTW -> sSS	Reopened connection (RFC 1122).
 *	sCL -> sSS
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*synack*/ { sIV, sIV, sIG, sIG, sIG, sIG, sIG, sIG, sIG, sSR },
/*
 *	sNO -> sIV	Too late and no reason to do anything
 * 	sSS -> sIV	Client can't send SYN and then SYN/ACK
 * 	sS2 -> sSR	SYN/ACK sent to SYN2 in simultaneous open
 * 	sSR -> sIG
 * 	sES -> sIG	Error: SYNs in window outside the SYN_SENT state
 * 			are errors. Receiver will reply with RST
 * 			and close the connection.
 * 			Or we are not in sync and hold a dead connection.
 * 	sFW -> sIG
 * 	sCW -> sIG
 * 	sLA -> sIG
 * 	sTW -> sIG
 * 	sCL -> sIG
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*fin*/    { sIV, sIV, sFW, sFW, sLA, sLA, sLA, sTW, sCL, sIV },
/*
 *	sNO -> sIV	Too late and no reason to do anything...
 * 	sSS -> sIV	Client migth not send FIN in this state:
 * 			we enforce waiting for a SYN/ACK reply first.
 * 	sS2 -> sIV
 * 	sSR -> sFW	Close started.
 * 	sES -> sFW
 * 	sFW -> sLA	FIN seen in both directions, waiting for
 * 			the last ACK.
 * 			Migth be a retransmitted FIN as well...
 * 	sCW -> sLA
 * 	sLA -> sLA	Retransmitted FIN. Remain in the same state.
 * 	sTW -> sTW
 * 	sCL -> sCL
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*ack*/	   { sES, sIV, sES, sES, sCW, sCW, sTW, sTW, sCL, sIV },
/*
 *	sNO -> sES	Assumed.
 * 	sSS -> sIV	ACK is invalid: we haven't seen a SYN/ACK yet.
 * 	sS2 -> sIV
 * 	sSR -> sES	Established state is reached.
 * 	sES -> sES	:-)
 * 	sFW -> sCW	Normal close request answered by ACK.
 * 	sCW -> sCW
 * 	sLA -> sTW	Last ACK detected.
 * 	sTW -> sTW	Retransmitted last ACK. Remain in the same state.
 * 	sCL -> sCL
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*rst*/    { sIV, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL },
/*none*/   { sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV }
	},
	{
/* REPLY */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*syn*/	   { sIV, sS2, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sS2 },
/*
 *	sNO -> sIV	Never reached.
 * 	sSS -> sS2	Simultaneous open
 * 	sS2 -> sS2	Retransmitted simultaneous SYN
 * 	sSR -> sIV	Invalid SYN packets sent by the server
 * 	sES -> sIV
 * 	sFW -> sIV
 * 	sCW -> sIV
 * 	sLA -> sIV
 * 	sTW -> sIV	Reopened connection, but server may not do it.
 * 	sCL -> sIV
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*synack*/ { sIV, sSR, sSR, sIG, sIG, sIG, sIG, sIG, sIG, sSR },
/*
 *	sSS -> sSR	Standard open.
 * 	sS2 -> sSR	Simultaneous open
 * 	sSR -> sSR	Retransmitted SYN/ACK.
 * 	sES -> sIG	Late retransmitted SYN/ACK?
 * 	sFW -> sIG	Might be SYN/ACK answering ignored SYN
 * 	sCW -> sIG
 * 	sLA -> sIG
 * 	sTW -> sIG
 * 	sCL -> sIG
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*fin*/    { sIV, sIV, sFW, sFW, sLA, sLA, sLA, sTW, sCL, sIV },
/*
 *	sSS -> sIV	Server might not send FIN in this state.
 * 	sS2 -> sIV
 * 	sSR -> sFW	Close started.
 * 	sES -> sFW
 * 	sFW -> sLA	FIN seen in both directions.
 * 	sCW -> sLA
 * 	sLA -> sLA	Retransmitted FIN.
 * 	sTW -> sTW
 * 	sCL -> sCL
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*ack*/	   { sIV, sIG, sSR, sES, sCW, sCW, sTW, sTW, sCL, sIG },
/*
 *	sSS -> sIG	Might be a half-open connection.
 *	sS2 -> sIG
 *	sSR -> sSR	Might answer late resent SYN.
 *	sES -> sES	:-)
 *	sFW -> sCW	Normal close request answered by ACK.
 * 	sCW -> sCW
 * 	sLA -> sTW	Last ACK detected.
 * 	sTW -> sTW	Retransmitted last ACK.
 * 	sCL -> sCL
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*rst*/    { sIV, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL },
/*none*/   { sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV }
	}
};

static void udpi_init_eipentry(struct eip_entry* entry, uint32_t eip)
{
	entry->eip.eip = eip;

	entry->token = BURST;
	entry->last_conn = rte_get_tsc_cycles();
	entry->last_cps = 0;
		
	memset(&entry->stats, 0 , sizeof(struct udpi_eip_local_stats));

	char name[20];
	
	memset(name, 0, sizeof(name));
	sprintf(name, "port_stats_%x", eip);
	entry->stats.port_stats = rte_zmalloc(name, sizeof(struct udpi_local_port_stats), 0);
	if (entry->stats.port_stats == NULL)
	{
		printf("haha rte_malloc not enough\n");
		entry->stats.port_stats = malloc(sizeof(struct udpi_local_port_stats));
		if (entry->stats.port_stats == NULL)
			rte_panic("malloc port_stats fail\n");
	}		
	//udpi_eipstats_init(&entry->stats);
}


static struct eip_entry* udpi_find_eip(struct eip_key *key, uint32_t worker_id)
{
	int32_t ret;
	struct rte_hash *h = udpi.hash[worker_id].eip_hash;
	struct eip_entry *entry = udpi.hash[worker_id].eip_entry;

	ret = rte_hash_lookup(h, (const void*)key);
	if (ret < 0)
	{
		/*if (ret != -ENOENT)
			rte_panic("udpi_find_eip lookup fail:%d\n", ret);
		*/

		ret = rte_hash_add_key(h, (const void*)key);
		if (ret < 0)
		{
			rte_atomic64_inc(&udpi.overflow.eip);
			return NULL;
		}
		entry = entry + ret;

		udpi_init_eipentry(entry, key->eip);
		rte_atomic64_inc(&udpi.hash[worker_id].eip_entries);
		rte_atomic64_inc(&udpi.eip_entries);
	}
	else
	{
		entry = entry + ret;
	}

	return entry;
}

static void udpi_update_eipstats(struct eip_entry *entry,  struct rte_mbuf *pkt)
{
	struct udpi_pkt_metadata *c = (struct udpi_pkt_metadata *) RTE_MBUF_METADATA_UINT8_PTR(pkt, UDPI_METADATA_OFFSET(0));

	entry->stats.map_stats[c->region_type][c->operator_type].seen = 1;
	entry->stats.seen = 1;

	if (c->port_type == UDPI_PORT_INPUT)
	{
		entry->stats.input_stats.l3.ip_pkts += 1;
		entry->stats.input_stats.l3.ip_bytes += pkt->pkt_len;
		
		entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l3.ip_pkts += 1;
		entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l3.ip_bytes += pkt->pkt_len;

		if (c->comp_type < UDPI_COMP_MAX)
		{
			entry->stats.input_comp_stats[c->comp_type].ip_bytes += pkt->pkt_len;
		}

		if (c->proto == IPPROTO_UDP)
		{
			entry->stats.input_stats.l4.udp_pkts += 1;
			entry->stats.input_stats.l4.udp_bytes += pkt->pkt_len;
		
			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l4.udp_pkts += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l4.udp_bytes += pkt->pkt_len;
		}
		else if (c->proto == IPPROTO_TCP)
		{
			entry->stats.input_stats.l4.tcp_pkts += 1;
			entry->stats.input_stats.l4.tcp_bytes += pkt->pkt_len;
		
			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_pkts += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_bytes += pkt->pkt_len;
		}

		if (c->port_src == PROTOCOL_SSH || c->port_dst == PROTOCOL_SSH)
		{
			entry->stats.input_stats.l7.ssh += 1;
			entry->stats.input_stats.l7.ssh_bytes += pkt->pkt_len;

			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l7.ssh += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l7.ssh_bytes += pkt->pkt_len;
		}
		else if (c->port_src == PROTOCOL_HTTP || c->port_dst == PROTOCOL_HTTP || c->port_src == 8080 || c->port_dst == 8080)
		{
			entry->stats.input_stats.l7.http += 1;
			entry->stats.input_stats.l7.http_bytes += pkt->pkt_len;

			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l7.http += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l7.http_bytes += pkt->pkt_len;
		}
		else if (c->port_src == PROTOCOL_HTTPS || c->port_dst == PROTOCOL_HTTPS)
		{
			entry->stats.input_stats.l7.https += 1;
			entry->stats.input_stats.l7.https_bytes += pkt->pkt_len;

			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l7.https += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l7.https_bytes += pkt->pkt_len;
		}
		else
		{
			entry->stats.input_stats.l7.others += 1;
			entry->stats.input_stats.l7.others_bytes += pkt->pkt_len;

			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l7.others += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].input_stats.l7.others_bytes += pkt->pkt_len;
		}
	}
	else if (c->port_type == UDPI_PORT_OUTPUT)
	{
		entry->stats.output_stats.l3.ip_pkts += 1;
		entry->stats.output_stats.l3.ip_bytes += pkt->pkt_len;

		if (c->comp_type < UDPI_COMP_MAX)
		{
			entry->stats.output_comp_stats[c->comp_type].ip_bytes += pkt->pkt_len;
		}
		
		entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l3.ip_pkts += 1;
		entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l3.ip_bytes += pkt->pkt_len;

		if (c->proto == IPPROTO_UDP)
		{
			entry->stats.output_stats.l4.udp_pkts += 1;
			entry->stats.output_stats.l4.udp_bytes += pkt->pkt_len;
		
			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l4.udp_pkts += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l4.udp_bytes += pkt->pkt_len;
		}
		else if (c->proto == IPPROTO_TCP)
		{
			entry->stats.output_stats.l4.tcp_pkts += 1;
			entry->stats.output_stats.l4.tcp_bytes += pkt->pkt_len;
		
			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_pkts += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_bytes += pkt->pkt_len;
		}

		if (c->port_src == PROTOCOL_SSH || c->port_dst == PROTOCOL_SSH)
		{
			entry->stats.output_stats.l7.ssh += 1;
			entry->stats.output_stats.l7.ssh_bytes += pkt->pkt_len;

			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l7.ssh += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l7.ssh_bytes += pkt->pkt_len;
		}
		else if (c->port_src == PROTOCOL_HTTP || c->port_dst == PROTOCOL_HTTP || c->port_src == 8080 || c->port_dst == 8080)
		{
			entry->stats.output_stats.l7.http += 1;
			entry->stats.output_stats.l7.http_bytes += pkt->pkt_len;

			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l7.http += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l7.http_bytes += pkt->pkt_len;
		}	
		else if (c->port_src == PROTOCOL_HTTPS || c->port_dst == PROTOCOL_HTTPS)
		{
			entry->stats.output_stats.l7.https += 1;
			entry->stats.output_stats.l7.https_bytes += pkt->pkt_len;

			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l7.https += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l7.https_bytes += pkt->pkt_len;
		}

		if (c->port_dst == PROTOCOL_HTTP || c->port_dst == 8080)
		{
			entry->stats.output_stats.l7.others += 1;
			entry->stats.output_stats.l7.others_bytes += pkt->pkt_len;

			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l7.others += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].output_stats.l7.others_bytes += pkt->pkt_len;
		}	
	}

}

static void udpi_init_flowentry(struct flow_entry* entry, struct eip_entry* eip, struct flow_key *key, uint8_t region, uint8_t isp, uint8_t dir_origin)
{
	entry->flow.upper_ip = key->upper_ip;
	entry->flow.lower_ip = key->lower_ip;
	entry->flow.upper_port = key->upper_port;
	entry->flow.lower_port = key->lower_port;
	entry->flow.proto = key->proto;
	entry->flow.diff = key->diff;

	entry->last_seen = 0;

	entry->eip = eip;

	entry->next_tcp_seq_nr[0] = 0;
	entry->next_tcp_seq_nr[1] = 0;

	entry->state = sNO;
	entry->dir_origin = dir_origin;

	entry->region = region; 
	entry->isp = isp;
	
	entry->ts.last_time_stamps = 0;
	entry->ts.last_finished_stamps = 0;
	entry->ts.last_seen = 0;

	entry->state_ok = 0;
	entry->validate = 1;
}

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
/* Saturating multiplication of "unsigned int"s: overflow yields UINT_MAX. */
#define SAT_MUL(X, Y)                                                   \
    ((Y) == 0 ? 0                                                       \
     : (X) <= UINT_MAX / (Y) ? (unsigned int) (X) * (unsigned int) (Y)  \
     : UINT_MAX)
static inline unsigned int
sat_mul(unsigned int x, unsigned int y)
{
    return SAT_MUL(x, y);
}

/* Saturating addition: overflow yields UINT_MAX. */
static inline unsigned int
sat_add(unsigned int x, unsigned int y)
{
    return x + y >= x ? x + y : UINT_MAX;
}

static unsigned int get_conntrack_index(const struct tcp_hdr *tcph)
{
	if (tcph->tcp_flags & TCP_RST_FLAG) return TCP_RST_SET;
	else if (tcph->tcp_flags & TCP_SYN_FLAG) return (tcph->tcp_flags & TCP_ACK_FLAG ? TCP_SYNACK_SET : TCP_SYN_SET);
	else if (tcph->tcp_flags & TCP_FIN_FLAG) return TCP_FIN_SET;
	else if (tcph->tcp_flags & TCP_ACK_FLAG) return TCP_ACK_SET;
	else return TCP_NONE_SET;
}

static struct flow_entry* udpi_find_flow(struct rte_mbuf* pkt, struct eip_entry* entry, struct flow_key *key, uint32_t worker_id, struct udpi_local_stats *stats)
{
	struct flow_entry **pfentry = udpi.hash[worker_id].flow_entry;
	struct rte_hash *h = udpi.hash[worker_id].flow_hash;
	struct udpi_pkt_metadata *c = (struct udpi_pkt_metadata *) RTE_MBUF_METADATA_UINT8_PTR(pkt, UDPI_METADATA_OFFSET(0));
	struct flow_entry *fentry = NULL;
	int32_t ret;

	ret = rte_hash_lookup(h, (const void*)key);
	if (ret < 0)
	{
		/*
		if (ret != -ENOENT)
			rte_panic("udpi_find_flow lookup fail:%d\n", ret);
		*/

		uint8_t dir = 0;

		if (key->proto == IPPROTO_TCP)
		{
			uint8_t *m_data = rte_pktmbuf_mtod(pkt, uint8_t *);
			struct tcp_hdr *tcph = (struct tcp_hdr*)&m_data[c->l4_offset];
			if (tcph->tcp_flags != TCP_SYN_FLAG)
			{
				//rte_atomic64_inc(&udpi.overflow.notsyn);
				return NULL;
			}
			
			dir = (c->port_type == UDPI_PORT_INPUT);
		}
		
		if (entry->token < TOKEN_PER_C)
		{
			uint64_t elapsed_ull = (rte_get_tsc_cycles() - entry->last_conn)/tsc_mhz[worker_id];

			unsigned int elapsed = MIN(UINT_MAX, elapsed_ull);
			//unsigned int add = sat_mul(CPS*TOKEN_PER_C/MSEC_PER_SEC, elapsed);
			unsigned int add = sat_mul(CPS, elapsed);
			unsigned int tokens = sat_add(entry->token, add);
			entry->token = MIN(tokens, BURST);

			entry->last_conn = rte_get_tsc_cycles();
			if (entry->token < TOKEN_PER_C)
			{
				if ((entry->last_cps == 0) || (((rte_get_tsc_cycles() - entry->last_cps)/tsc_mhz[worker_id]) > 10*60*1000))
				{
					rte_atomic64_inc(&udpi.overflow.cps);
					entry->last_cps = rte_get_tsc_cycles();
				}
				return NULL;
			}
		}
				
		entry->token -= TOKEN_PER_C;

		ret = rte_hash_add_key(h, (const void*)key);
		if (ret < 0)
		{
			//rte_panic("udpi_find_flow add key fail:%d\n", ret);
			rte_atomic64_inc(&udpi.overflow.flow);
			return NULL;
		}
		
		stats->map_stats[c->region_type][c->operator_type].seen = 1;
		if(c->is_inline)
		{
		stats->line_stats[c->line_num].map_stats[c->region_type][c->operator_type].seen= 1;
		}
		if (key->proto == IPPROTO_UDP)
		{
			entry->stats.flows_stats.udp_flow_entries += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].flows_stats.udp_flow_entries += 1;
			stats->flows_stats.udp_flow_entries += 1;
			stats->map_stats[c->region_type][c->operator_type].flows_stats.udp_flow_entries += 1;

			entry->stats.flows_stats.flow_entries += 1;
			entry->stats.map_stats[c->region_type][c->operator_type].flows_stats.flow_entries += 1;
			stats->flows_stats.flow_entries += 1;
			stats->map_stats[c->region_type][c->operator_type].flows_stats.flow_entries += 1;
		}

		if (pfentry[ret])
			fentry = pfentry[ret];
		else
		{
			char name[26];
	
			memset(name, 0, sizeof(name));
			sprintf(name, "%x_%X_%x_%x_%x", key->upper_ip, key->lower_ip, key->upper_port, key->lower_port, key->proto);
			fentry = pfentry[ret] = rte_zmalloc(name, sizeof(struct flow_entry), 0);
			if (fentry == NULL)
			{
				//printf("haha rte_malloc flow entry not enough\n");
				fentry = pfentry[ret] = malloc(sizeof(struct flow_entry));
				if (fentry == NULL)
				{
					printf("haha malloc flow entry not enough\n");
					rte_hash_del_key(h, key);
					return NULL;
					//rte_panic("malloc flow entry fail\n");
				}
			}
		}		

		udpi_init_flowentry(fentry, entry, key, c->region_type, c->operator_type, dir);
	}
	else
		fentry = pfentry[ret];

	return fentry;
}

#define TCPOPT_NOP		1	/* Padding */
#define TCPOPT_EOL		0	/* End of options */
#define TCPOPT_TIMESTAMP 8
#define TCPOLEN_TIMESTAMP 10

static uint32_t udpi_get_timestamps(struct tcp_hdr* tcph, uint8_t type)
{
	unsigned char *ptr;
	int length;
	uint32_t ts, ts_echo;

	ptr = (unsigned char *)(tcph + 1);
	length = ((tcph->data_off & 0xf0) >> 2) - sizeof(struct tcp_hdr);
	
	while (length > 0) 
	{
		int opcode = *ptr++;
		int opsize;

		switch (opcode) {
		case TCPOPT_EOL:
			return 0;
		case TCPOPT_NOP:	/* Ref: RFC 793 section 3.1 */
			length--;
			continue;
		default:
			opsize = *ptr++;
			if (opsize < 2) /* "silly options" */
				return 0;
			if (opsize > length)
				return 0;	/* don't parse partial options */
			if ((TCPOPT_TIMESTAMP == opcode) && (opsize == TCPOLEN_TIMESTAMP))
			{//http://blog.chinaunix.net/uid-10455511-id-4020447.html
				ts = rte_be_to_cpu_32(*(uint32_t*)ptr);
				ts_echo = rte_be_to_cpu_32(*(uint32_t*)(ptr+4));			
				if (type == 0)
					return ts;
				else if (type == 1)
					return ts_echo;
				else
					return 0;
			}

			ptr += opsize-2;
			length -= opsize;
		}
	}

	return 0;
}


static inline int set_tcp_state(struct flow_entry *fentry, int dir, struct tcp_hdr *th, uint64_t tsc, uint8_t region_type, uint8_t operator_type, struct udpi_local_stats *stats)
{
	int index;
	uint8_t new_state, old_state;

	old_state = fentry->state;
	index = get_conntrack_index(th);
	new_state = tcp_conntracks[dir][index][old_state];

	if ((new_state == sIG) || (new_state == sIV))
	{
		return -1;
	}

	if ((old_state == sSR) && (new_state == sES))
	{
		struct eip_entry *entry = fentry->eip;

		entry->stats.flows_stats.flow_entries += 1;
		entry->stats.flows_stats.tcp_flow_entries +=1 ;
		entry->stats.map_stats[region_type][operator_type].flows_stats.flow_entries += 1;
		entry->stats.map_stats[region_type][operator_type].flows_stats.tcp_flow_entries += 1;

		stats->flows_stats.flow_entries += 1;
		stats->flows_stats.tcp_flow_entries += 1;
		stats->map_stats[region_type][operator_type].flows_stats.flow_entries += 1;
		stats->map_stats[region_type][operator_type].flows_stats.tcp_flow_entries += 1;
		fentry->state_ok = 1;
	}

	fentry->state = new_state;	
	fentry->last_seen = tsc;

	if ((new_state > sES) && (new_state != sS2))
		return -1;

	return 0;
}

static void udpi_connection_tracking(struct flow_entry* entry, struct rte_mbuf* pkt, uint32_t worker_id, uint64_t lcore_tsc_mhz, struct udpi_local_stats *stats)
{
	uint8_t *m_data = rte_pktmbuf_mtod(pkt, uint8_t *);
	struct udpi_pkt_metadata *c = (struct udpi_pkt_metadata *) RTE_MBUF_METADATA_UINT8_PTR(pkt, UDPI_METADATA_OFFSET(0));
	struct tcp_hdr *tcph = (struct tcp_hdr*)&m_data[c->l4_offset];

	uint16_t payload_packet_len = c->payload_len;
	uint8_t  packet_direction;
	uint16_t num_retried_bytes = 0;

	if (c->port_type == UDPI_PORT_INPUT)
		packet_direction = 0;
	else if (c->port_type == UDPI_PORT_OUTPUT)	
		packet_direction = 1;
	else 
		return;

	if (((tcph->tcp_flags & TCP_RST_FLAG) != 0) && (entry->state_ok))
	{
		struct eip_entry *eip = entry->eip;

		eip->stats.flows_stats.tcp_rst_flow_entries += 1;
		eip->stats.map_stats[c->region_type][c->operator_type].flows_stats.tcp_rst_flow_entries += 1;

		stats->flows_stats.tcp_rst_flow_entries += 1;
		stats->map_stats[c->region_type][c->operator_type].flows_stats.tcp_rst_flow_entries += 1;

		rte_hash_del_key(udpi.hash[worker_id].flow_hash, &entry->flow);
		entry->validate = 0;
		return;
	}

	if (tcph->tcp_flags == TCP_SYN_FLAG)
	{
		struct eip_entry *eip = entry->eip;
		if (c->port_type == UDPI_PORT_INPUT)
		{
			eip->stats.input_stats.l4.tcp_syn_pkts += 1;
			eip->stats.map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_syn_pkts += 1;
		}
		else
		{
			eip->stats.output_stats.l4.tcp_syn_pkts += 1;
			eip->stats.map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_syn_pkts += 1;
		}
	}

//tcp state
	uint8_t dir;
	if (packet_direction == 0 && entry->dir_origin)
	{
		entry->eip->stats.port_stats->port_list[c->port_dst] += 1;
		entry->eip->stats.port_stats->port_list_seen[c->port_dst] = 1;
		dir = TCP_DIR_ORIGIN;
	}
	else if (packet_direction == 0 && entry->dir_origin == 0)
	{
		entry->eip->stats.port_stats->port_list[c->port_src] += 1;
		entry->eip->stats.port_stats->port_list_seen[c->port_src] = 1;
		dir = TCP_DIR_REPLY;
	}
	else if (packet_direction && entry->dir_origin)
	{
		entry->eip->stats.port_stats->port_list[c->port_src] += 1;
		entry->eip->stats.port_stats->port_list_seen[c->port_src] = 1;
		dir = TCP_DIR_REPLY;
	}
	else if (packet_direction && entry->dir_origin == 0)
	{
		entry->eip->stats.port_stats->port_list[c->port_dst] += 1;
		entry->eip->stats.port_stats->port_list_seen[c->port_dst] = 1;
		dir = TCP_DIR_ORIGIN;
	}

	int ret = set_tcp_state(entry, dir, tcph, c->tsc, c->region_type, c->operator_type, stats);
	if (ret < 0)
		return;
		
//tcp retransmit
	if (entry->next_tcp_seq_nr[0] == 0 && entry->next_tcp_seq_nr[1] == 0)
	{
		if ((tcph->tcp_flags & TCP_ACK_FLAG) != 0) 
		{
			entry->next_tcp_seq_nr[packet_direction] = rte_be_to_cpu_32(tcph->sent_seq) + ((tcph->tcp_flags & TCP_SYN_FLAG) ? 1 : payload_packet_len);
			entry->next_tcp_seq_nr[1 - packet_direction] = rte_be_to_cpu_32(tcph->recv_ack);
		}
	}
	else if (payload_packet_len > 0) 
	{
		if (((uint32_t)(rte_be_to_cpu_32(tcph->sent_seq) - entry->next_tcp_seq_nr[packet_direction])) > UDPI_MAX_TCP_RETRANSMISSION_WINDOW_SIZE) 
		{
			if (c->port_type == UDPI_PORT_INPUT)
			{
				entry->eip->stats.input_stats.l4.tcp_retransmit_pkts += 1;
				entry->eip->stats.map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_retransmit_pkts += 1;

				stats->input_stats.l4.tcp_retransmit_pkts += 1;
				stats->map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_retransmit_pkts += 1;

				if(c->is_inline)
				{
				stats->line_stats[c->line_num].input_stats.l4.tcp_retransmit_pkts += 1;
				stats->line_stats[c->line_num].map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_retransmit_pkts += 1;
				}
			}
			else if (c->port_type == UDPI_PORT_OUTPUT)	
			{	
				entry->eip->stats.output_stats.l4.tcp_retransmit_pkts += 1;
				entry->eip->stats.map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_retransmit_pkts += 1;

				stats->map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_retransmit_pkts += 1;
				stats->output_stats.l4.tcp_retransmit_pkts += 1;
				if(c->is_inline)
				{
				stats->line_stats[c->line_num].output_stats.l4.tcp_retransmit_pkts += 1;
				stats->line_stats[c->line_num].map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_retransmit_pkts += 1;
				}
			}

			num_retried_bytes = payload_packet_len;
			// CHECK IF PARTIAL RETRY IS HAPPENING 
			if ((entry->next_tcp_seq_nr[packet_direction] - rte_be_to_cpu_32(tcph->sent_seq) < payload_packet_len)) 
			{
	  			// num_retried_bytes actual_payload_len hold info about the partial retry
 				//  analyzer which require this info can make use of this info
 				//  Other analyzer can use payload_packet_len 
	  			num_retried_bytes = entry->next_tcp_seq_nr[packet_direction] - rte_be_to_cpu_32(tcph->sent_seq);
	  			//actual_payload_len = payload_packet_len - num_retried_bytes;
	  			entry->next_tcp_seq_nr[packet_direction] = rte_be_to_cpu_32(tcph->sent_seq) + payload_packet_len;
			}

			if (c->port_type == UDPI_PORT_INPUT)
			{
				entry->eip->stats.input_stats.l4.tcp_retransmit_bytes += num_retried_bytes;
				entry->eip->stats.map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_retransmit_bytes += num_retried_bytes;
				stats->input_stats.l4.tcp_retransmit_bytes += num_retried_bytes;
				stats->map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_retransmit_bytes += num_retried_bytes;
				if(c->is_inline)
				{
				stats->line_stats[c->line_num].input_stats.l4.tcp_retransmit_bytes += num_retried_bytes;
				stats->line_stats[c->line_num].map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_retransmit_bytes += num_retried_bytes;
				}
			}
			else if (c->port_type == UDPI_PORT_OUTPUT)	
			{
				entry->eip->stats.output_stats.l4.tcp_retransmit_bytes += num_retried_bytes;
				entry->eip->stats.map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_retransmit_bytes += num_retried_bytes;

				stats->output_stats.l4.tcp_retransmit_bytes += num_retried_bytes;
				stats->map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_retransmit_bytes += num_retried_bytes;
				if(c->is_inline)
				{
				stats->line_stats[c->line_num].output_stats.l4.tcp_retransmit_bytes += num_retried_bytes;
				stats->line_stats[c->line_num].map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_retransmit_bytes += num_retried_bytes;
				}
			}
		}
		else
		{
			entry->next_tcp_seq_nr[packet_direction] = rte_be_to_cpu_32(tcph->sent_seq) + payload_packet_len;
		}
	}

//tcp delay times
	if (c->port_type == UDPI_PORT_OUTPUT && payload_packet_len > 0)
	{
		uint32_t ts = udpi_get_timestamps(tcph, 0);

		if (ts != 0 && ts > entry->ts.last_finished_stamps)
		{
			entry->ts.last_time_stamps = ts;
			entry->ts.last_seen = c->tsc;
		}
	}
	else if ((c->port_type == UDPI_PORT_INPUT) && ((tcph->tcp_flags & TCP_ACK_FLAG) != 0))
	{
		uint32_t echo_ts = udpi_get_timestamps(tcph, 1);
		if (echo_ts && entry->ts.last_time_stamps == echo_ts)
		{
			uint64_t delay_ms = (c->tsc - entry->ts.last_seen)/lcore_tsc_mhz;
		
			entry->eip->stats.delay_stats.tcp_delay_flows += 1;
			entry->eip->stats.map_stats[c->region_type][c->operator_type].delay_stats.tcp_delay_flows += 1;	
			stats->delay_stats.tcp_delay_flows += 1;
			stats->map_stats[c->region_type][c->operator_type].delay_stats.tcp_delay_flows += 1;
			if(c->is_inline)
			{
			stats->line_stats[c->line_num].delay_stats.tcp_delay_flows += 1;
			stats->line_stats[c->line_num].map_stats[c->region_type][c->operator_type].delay_stats.tcp_delay_flows += 1;
			}
			entry->eip->stats.delay_stats.tcp_delay_ms += delay_ms;
			entry->eip->stats.map_stats[c->region_type][c->operator_type].delay_stats.tcp_delay_ms += delay_ms;
			stats->delay_stats.tcp_delay_ms += delay_ms;
			stats->map_stats[c->region_type][c->operator_type].delay_stats.tcp_delay_ms += delay_ms;
			if(c->is_inline)
			{
			stats->line_stats[c->line_num].delay_stats.tcp_delay_ms += delay_ms;
			stats->line_stats[c->line_num].map_stats[c->region_type][c->operator_type].delay_stats.tcp_delay_ms += delay_ms;
			}
			entry->ts.last_finished_stamps = entry->ts.last_time_stamps;
			entry->ts.last_time_stamps = 0;
		}
	}

//http
	if (c->port_type == UDPI_PORT_INPUT && (c->port_dst == PROTOCOL_HTTP || c->port_dst == 8080))
	{
		if (payload_packet_len > 4)
		{
			char *header = (char*)&m_data[c->l4_offset + ((tcph->data_off & 0xf0) >> 2)];
			if ((strncmp(header, "GET ", 4) == 0) || (strncmp(header, "POST ", 5) == 0))
			{
				//entry->http_request = 1;
				entry->last_http_request = c->tsc;
			}
		}
	}
	else if (c->port_type == UDPI_PORT_OUTPUT && (c->port_src == PROTOCOL_HTTP || c->port_src == 8080))
	{
		if (payload_packet_len > 12)
		{
			char *header = (char*)&m_data[c->l4_offset + ((tcph->data_off & 0xf0) >> 2)];
			if (strncmp(header, "HTTP/1.", 7) == 0)
			{
				if (entry->last_http_request)
				{
					entry->eip->stats.http_stats.http_delay_ms += (c->tsc - entry->last_http_request)/lcore_tsc_mhz;
					entry->eip->stats.http_stats.http_delay_flows += 1;
				}
				
				entry->last_http_request = 0;
				
				header += 9;
				char data[4];
				strncpy(data, header, 3);
				data[3] = '\0';

				int value = atoi(data);
				if (value < 600 && value >= 100)
				{
					int item = value/100;
					entry->eip->stats.http_stats.http_return[item - 1] += 1;
					entry->eip->stats.http_stats.http_code[value] += 1;
					entry->eip->stats.http_stats.http_code_seen[value] = 1;
				}	
			}
		}
	}
}

static void udpi_packet_processing(struct rte_mbuf* pkt, uint32_t worker_id, uint64_t lcore_tsc_hz, struct udpi_local_stats *stats)
{
	struct udpi_pkt_metadata *c =
                (struct udpi_pkt_metadata *) RTE_MBUF_METADATA_UINT8_PTR(pkt, UDPI_METADATA_OFFSET(0));
	struct eip_key key;

	c->tsc = rte_get_tsc_cycles();
	
	if (c->port_type == UDPI_PORT_INPUT)
	{
		key.eip = c->ip_dst;	
	}	
	else
	{
		key.eip = c->ip_src;	
	}	

	struct eip_entry *entry = udpi_find_eip(&key, worker_id);
	if (!entry)
		return;

	udpi_update_eipstats(entry, pkt);
	
	struct flow_key fkey;

	if(c->ip_src < c->ip_dst) 
	{
		fkey.lower_ip = c->ip_src;
		fkey.upper_ip = c->ip_dst;
		if (c->port_src < c->port_dst)
			fkey.diff = 0;
		else
			fkey.diff = 1;
	} 
	else 
	{
		fkey.lower_ip = c->ip_dst;
		fkey.upper_ip = c->ip_src;
		if (c->port_src > c->port_dst)
			fkey.diff = 0;
		else
			fkey.diff = 1;
	}

	if(c->port_src < c->port_dst) 
	{
		fkey.lower_port = c->port_src;
		fkey.upper_port = c->port_dst;
	} 
	else 
	{
		fkey.lower_port = c->port_dst;
		fkey.upper_port = c->port_src;
	}
	
	fkey.proto = c->proto;

	struct flow_entry *fentry = udpi_find_flow(pkt, entry, &fkey, worker_id, stats);
	if (!fentry)
		return;

   	if (c->proto == IPPROTO_TCP)
		udpi_connection_tracking(fentry, pkt, worker_id, lcore_tsc_hz/MSEC_PER_SEC, stats);
	else
		fentry->last_seen = c->tsc;
}

static void udpi_update_allstats(struct udpi_local_stats *stats)
{
	rte_atomic64_add(&(udpi.stats.input_stats.l4.tcp_retransmit_pkts), stats->input_stats.l4.tcp_retransmit_pkts);
	rte_atomic64_add(&(udpi.stats.output_stats.l4.tcp_retransmit_pkts), stats->output_stats.l4.tcp_retransmit_pkts);
	rte_atomic64_add(&(udpi.stats.input_stats.l4.tcp_retransmit_bytes), stats->input_stats.l4.tcp_retransmit_bytes);
	rte_atomic64_add(&(udpi.stats.output_stats.l4.tcp_retransmit_bytes), stats->output_stats.l4.tcp_retransmit_bytes);
	
	rte_atomic64_add(&(udpi.stats.flows_stats.flow_entries), stats->flows_stats.flow_entries);
	rte_atomic64_add(&(udpi.stats.flows_stats.closed_flow_entries), stats->flows_stats.closed_flow_entries);
	rte_atomic64_add(&(udpi.stats.flows_stats.tcp_flow_entries), stats->flows_stats.tcp_flow_entries);
	rte_atomic64_add(&(udpi.stats.flows_stats.tcp_rst_flow_entries), stats->flows_stats.tcp_rst_flow_entries);
	rte_atomic64_add(&(udpi.stats.flows_stats.tcp_closed_flow_entries), stats->flows_stats.tcp_closed_flow_entries);
	rte_atomic64_add(&(udpi.stats.flows_stats.udp_flow_entries), stats->flows_stats.udp_flow_entries);
	rte_atomic64_add(&(udpi.stats.flows_stats.udp_closed_flow_entries), stats->flows_stats.udp_closed_flow_entries);

	rte_atomic64_add(&(udpi.stats.delay_stats.tcp_delay_ms), stats->delay_stats.tcp_delay_ms);
	rte_atomic64_add(&(udpi.stats.delay_stats.tcp_delay_flows), stats->delay_stats.tcp_delay_flows);
	

	int i, j;
	for (i=0; i<UDPI_REGION_STATS; i++)
		for(j=0; j<UDPI_OPERATOR_STATS; j++)
		{
			if (stats->map_stats[i][j].seen)
			{
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l4.tcp_retransmit_pkts), stats->map_stats[i][j].input_stats.l4.tcp_retransmit_pkts);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l4.tcp_retransmit_pkts), stats->map_stats[i][j].output_stats.l4.tcp_retransmit_pkts);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l4.tcp_retransmit_bytes), stats->map_stats[i][j].input_stats.l4.tcp_retransmit_bytes);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l4.tcp_retransmit_bytes), stats->map_stats[i][j].output_stats.l4.tcp_retransmit_bytes);
				
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].flows_stats.flow_entries), stats->map_stats[i][j].flows_stats.flow_entries);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].flows_stats.closed_flow_entries), stats->map_stats[i][j].flows_stats.closed_flow_entries);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].flows_stats.tcp_flow_entries), stats->map_stats[i][j].flows_stats.tcp_flow_entries);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].flows_stats.tcp_rst_flow_entries), stats->map_stats[i][j].flows_stats.tcp_rst_flow_entries);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].flows_stats.tcp_closed_flow_entries), stats->map_stats[i][j].flows_stats.tcp_closed_flow_entries);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].flows_stats.udp_flow_entries), stats->map_stats[i][j].flows_stats.udp_flow_entries);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].flows_stats.udp_closed_flow_entries), stats->map_stats[i][j].flows_stats.udp_closed_flow_entries);
				
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].delay_stats.tcp_delay_ms), stats->map_stats[i][j].delay_stats.tcp_delay_ms);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].delay_stats.tcp_delay_flows), stats->map_stats[i][j].delay_stats.tcp_delay_flows);
			}
		}

	//line
	int k =0;
	for (k=0; k<UDPI_LINE_STATS; k++)
	{
		rte_atomic64_add(&(udpi.stats.line_stats[k].input_stats.l4.tcp_retransmit_pkts), stats->line_stats[k].input_stats.l4.tcp_retransmit_pkts);
		rte_atomic64_add(&(udpi.stats.line_stats[k].output_stats.l4.tcp_retransmit_pkts), stats->line_stats[k].output_stats.l4.tcp_retransmit_pkts);
		rte_atomic64_add(&(udpi.stats.line_stats[k].input_stats.l4.tcp_retransmit_bytes), stats->line_stats[k].input_stats.l4.tcp_retransmit_bytes);
		rte_atomic64_add(&(udpi.stats.line_stats[k].output_stats.l4.tcp_retransmit_bytes), stats->line_stats[k].output_stats.l4.tcp_retransmit_bytes);

		rte_atomic64_add(&(udpi.stats.line_stats[k].delay_stats.tcp_delay_ms), stats->line_stats[k].delay_stats.tcp_delay_ms);
		rte_atomic64_add(&(udpi.stats.line_stats[k].delay_stats.tcp_delay_flows), stats->line_stats[k].delay_stats.tcp_delay_flows);

		for (i=0; i<UDPI_REGION_STATS; i++)
		for(j=0; j<UDPI_OPERATOR_STATS; j++)
		{
			if (stats->line_stats[k].map_stats[i][j].seen)
			{
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].input_stats.l4.tcp_retransmit_pkts), stats->line_stats[k].map_stats[i][j].input_stats.l4.tcp_retransmit_pkts);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].output_stats.l4.tcp_retransmit_pkts), stats->line_stats[k].map_stats[i][j].output_stats.l4.tcp_retransmit_pkts);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].input_stats.l4.tcp_retransmit_bytes), stats->line_stats[k].map_stats[i][j].input_stats.l4.tcp_retransmit_bytes);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].output_stats.l4.tcp_retransmit_bytes), stats->line_stats[k].map_stats[i][j].output_stats.l4.tcp_retransmit_bytes);

				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].delay_stats.tcp_delay_ms), stats->line_stats[k].map_stats[i][j].delay_stats.tcp_delay_ms);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].delay_stats.tcp_delay_flows), stats->line_stats[k].map_stats[i][j].delay_stats.tcp_delay_flows);

			
			}
		}
	}

}

#define PREFETCH_OFFSET 3

struct last_cycle {
	uint64_t core_handle_cycles;
	uint64_t core_total_cycles;
};
//static struct last_cycle last_cycle[RTE_MAX_LCORE];

void udpi_main_loop_ipv4_dpi(void)
{
	uint32_t core_id = rte_lcore_id();
	
	struct udpi_core_params *core_params 
		= udpi_get_core_params(core_id);

	if((!core_params) 
		|| (core_params->core_type != UDPI_CORE_IPV4_DPI))
	{
		rte_panic("Core %u misconfiguration\n", core_id);
		return;
	}

	RTE_LOG(INFO, USER1, 
		"Core %u is doing DPI for worker_id %u\n", core_id, core_params->id);
	
	struct rte_mbuf *pkts[udpi.bsz_hwq_rd];
	int j = 0, n_mbufs = 0;
	uint32_t k = 0;
	uint32_t steps = UDPI_BASE_STEPS;
	uint64_t count = 0;
	
	uint64_t lcore_tsc_hz, new_cycles, end_cycles, s_cycles;
	lcore_tsc_hz = rte_get_timer_hz();
	tsc_mhz[core_params->id] = lcore_tsc_hz/MSEC_PER_SEC;
	struct flow_entry **pbfentry = udpi.hash[core_params->id].flow_entry;
	struct flow_entry *bfentry = NULL;

	struct udpi_local_stats stats;
	memset(&stats, 0, sizeof(struct udpi_local_stats));

	uint64_t prev_tsc = rte_get_tsc_cycles();
	s_cycles = prev_tsc;
	uint64_t cur_tsc = prev_tsc, last_tsc = 0, interval = 0, diff_tsc = 0;
	char is_busy = false;
	uint64_t timer_precision = lcore_tsc_hz;

	while (1)
	{
		last_tsc = cur_tsc;
		cur_tsc = rte_get_tsc_cycles();
		interval = cur_tsc - last_tsc;
		if (is_busy)
		{
			core_params->core_handle_cycles += interval;
		}
		core_params->core_total_cycles += interval;
		
		is_busy = false;

		diff_tsc = cur_tsc - prev_tsc;
		if (unlikely(diff_tsc >= timer_precision)) {
			is_busy = true;
			prev_tsc = cur_tsc;
			udpi_update_allstats(&stats);
			memset(&stats, 0, sizeof(struct udpi_local_stats));
		}
		//上面语句:? tick 同步全局数据
		n_mbufs = rte_ring_dequeue_burst(udpi.rings[core_params->id], (void **)pkts, udpi.bsz_swq_rd);
		if(n_mbufs < 1)
		{
			continue;
			//goto recycle;
		}

		is_busy = true;
		
		for (j = 0; j < PREFETCH_OFFSET && j < n_mbufs; j++) {
			rte_prefetch0(rte_pktmbuf_mtod(pkts[j], void *));
			rte_prefetch0(&pkts[j]->cacheline1);
		}

		for (j = 0; j < (n_mbufs - PREFETCH_OFFSET); j++) {
			rte_prefetch0(rte_pktmbuf_mtod(pkts[j + PREFETCH_OFFSET], void *));
			rte_prefetch0(&pkts[j + PREFETCH_OFFSET]->cacheline1);
			udpi_packet_processing(pkts[j], core_params->id, lcore_tsc_hz, &stats);	
			rte_pktmbuf_free(pkts[j]);
		}

		for (; j < n_mbufs; j++) {
			udpi_packet_processing(pkts[j], core_params->id, lcore_tsc_hz, &stats);	
			rte_pktmbuf_free(pkts[j]);
		}

//recycle:
		{
			new_cycles = cur_tsc;	

			uint32_t i = 0;			
			for(i=0; i<steps; )
			{
				bfentry = *pbfentry;
				if (bfentry && bfentry->validate)
				{
					count++;
				 	if (bfentry->flow.proto == IPPROTO_UDP)  
					{
						if (new_cycles - bfentry->last_seen >= 300*lcore_tsc_hz)
						{
							struct eip_entry *eip = bfentry->eip;
							
							eip->stats.flows_stats.closed_flow_entries += 1;
							eip->stats.map_stats[bfentry->region][bfentry->isp].flows_stats.closed_flow_entries += 1;
							stats.flows_stats.closed_flow_entries += 1;
							stats.map_stats[bfentry->region][bfentry->isp].flows_stats.closed_flow_entries += 1;

							eip->stats.flows_stats.udp_closed_flow_entries += 1;
							eip->stats.map_stats[bfentry->region][bfentry->isp].flows_stats.udp_closed_flow_entries += 1;
							stats.flows_stats.udp_closed_flow_entries += 1;
							stats.map_stats[bfentry->region][bfentry->isp].flows_stats.udp_closed_flow_entries += 1;

							rte_hash_del_key(udpi.hash[core_params->id].flow_hash, &bfentry->flow);
							bfentry->validate = 0;
						}
					}
					else if (new_cycles - bfentry->last_seen >= udpi_tcp_timeouts[bfentry->state]*lcore_tsc_hz)
					{
						if (bfentry->state_ok)
						{
							struct eip_entry *eip = bfentry->eip;

							eip->stats.flows_stats.closed_flow_entries += 1;
							eip->stats.map_stats[bfentry->region][bfentry->isp].flows_stats.closed_flow_entries += 1;
							stats.flows_stats.closed_flow_entries += 1;
							stats.map_stats[bfentry->region][bfentry->isp].flows_stats.closed_flow_entries += 1;

							eip->stats.flows_stats.tcp_closed_flow_entries += 1;	
							eip->stats.map_stats[bfentry->region][bfentry->isp].flows_stats.tcp_closed_flow_entries += 1;
							stats.flows_stats.tcp_closed_flow_entries += 1;
							stats.map_stats[bfentry->region][bfentry->isp].flows_stats.tcp_closed_flow_entries += 1;
						}

						//rte_atomic64_inc(&udpi.closed_flows[bfentry->state]);
						rte_hash_del_key(udpi.hash[core_params->id].flow_hash, &bfentry->flow);
						bfentry->validate = 0;
					}
				}

				k++;
				i++;

				if (k == udpi.flow_params.entries)
				{
					pbfentry = udpi.hash[core_params->id].flow_entry;
					k = 0;

					end_cycles = rte_get_tsc_cycles();	
					uint64_t times = (end_cycles - s_cycles)/lcore_tsc_hz;

					if (udpi.debug)
					{
						printf("end one trip %lu s for core %u count %lu step %u\n", times, core_params->id, count, steps);
					}
					
					if (times > 60)
						steps = steps << 1;
					else if (times < 30)
					{
						steps = steps >> 1;
						if (steps < 1)
							steps = 2;
					}	

					count = 0;
					s_cycles = end_cycles;
				}
				else
				{
					pbfentry ++;
				}
				
			}			
		}
	}
	
	return;
}



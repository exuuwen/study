/*-
 *
 * Copyright(c) UCloud. All rights reserved.
 * All rights reserved.
 *
 */

#include <rte_malloc.h>
#include <rte_lcore.h>
#include <rte_port.h>
#include <rte_cycles.h>
#include <rte_byteorder.h>
#include <rte_ether.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_jhash.h>

#include <unistd.h>

#include "log.h"
#include "main.h"

#define PREFETCH_OFFSET 3


static inline void udpi_pkt_metadata_fill(struct rte_mbuf *m, uint32_t queue_id, __attribute__((unused))enum udpi_port_type port_type)
{
	uint8_t *m_data = rte_pktmbuf_mtod(m, uint8_t *);
	struct udpi_pkt_metadata *c =
		(struct udpi_pkt_metadata *) RTE_MBUF_METADATA_UINT8_PTR(m, 0);

	struct ether_hdr *ether_hdr = (struct ether_hdr *)m_data;

	if (ether_hdr->ether_type != rte_cpu_to_be_16(ETHER_TYPE_IPv4))
	{
		return;
	}

	struct ipv4_hdr *ip_hdr =
		(struct ipv4_hdr *) &m_data[sizeof(struct ether_hdr)];

	struct trace_flow flow;
	/* TTL and Header Checksum are set to 0 */
	flow.srcip =c->ip_src = rte_be_to_cpu_32(ip_hdr->src_addr);
	flow.dstip = c->ip_dst = rte_be_to_cpu_32(ip_hdr->dst_addr);
	flow.proto = c->proto = ip_hdr->next_proto_id;

	uint8_t hlen = (ip_hdr->version_ihl & 0xf) * 4;

	if (c->proto == IPPROTO_UDP)
	{
		struct udp_hdr *uhr = (struct udp_hdr *)&m_data[sizeof(struct ether_hdr) + hlen];
		flow.srcport = c->port_src = rte_be_to_cpu_16(uhr->src_port);
		flow.dstport = c->port_dst = rte_be_to_cpu_16(uhr->dst_port);

		if (UDPI_FLOW_TRACED(flow))
		{

			UDPI_PACKET_DUMP(m, "udp ip_src 0x%x, ip_dst 0x%x, port_src %d, port_dst %d on queue %u\n", c->ip_src, c->ip_dst, c->port_src, c->port_dst, queue_id);
		}
	}
	else if (c->proto == IPPROTO_TCP)
	{
		struct tcp_hdr *thr = (struct tcp_hdr *)&m_data[sizeof(struct ether_hdr) + hlen];
		c->port_src = rte_be_to_cpu_16(thr->src_port);
		c->port_dst = rte_be_to_cpu_16(thr->dst_port);

		if (UDPI_FLOW_TRACED(flow))
		{
			UDPI_PACKET_DUMP(m, "tcp ip_src 0x%x, ip_dst 0x%x, port_src %d, port_dst %d on queue %u\n", c->ip_src, c->ip_dst, c->port_src, c->port_dst, queue_id);
		}
	}
	else
	{
		if (UDPI_FLOW_TRACED(flow))
		{
			UDPI_PACKET_DUMP(m, "proto %d ip_src 0x%x, ip_dst 0x%x on queue %u\n", c->proto, c->ip_src, c->ip_dst, queue_id);
		}
	}

	c->tsc = rte_get_tsc_cycles();

	return;
}

static void udpi_check_msg(uint8_t id)
{
	void *msg;
	int ret;
	struct udpi_msg_req *req;

	/* Read request message */
	ret = rte_ring_sc_dequeue(udpi.msg_rings[id], &msg);
	if (ret != 0)
	{
		return;
	}

	req = (struct udpi_msg_req *)rte_ctrlmbuf_data((struct rte_mbuf *)msg);
	switch (req->type) {
		case UDPI_MSG_SET_LOG_LEVEL:
		{
			uint8_t level = req->set_log_level.level;
			if (level < 8)
				set_local_log_level(level);
			break;
		}
		case UDPI_MSG_SET_TRACE:
		{
			struct trace_flow flow = req->set_trace.flow;
			enable_trace(flow);	
			break;
		}
		case UDPI_MSG_DISABLE_TRACE:
		{
			disable_trace();	
			break;
			
		}
		default:
		{
			break;
		}
	}

	/* Free message buffer */
	rte_ctrlmbuf_free((struct rte_mbuf *)msg);
}

void udpi_main_loop_ipv4_rx(void)
{
	uint32_t core_id = rte_lcore_id();
	
	struct udpi_core_params *core_params 
		= udpi_get_core_params(core_id);

	if((!core_params) 
		|| (core_params->core_type != UDPI_CORE_IPV4_RX))
	{
		rte_panic("Core %u misconfiguration\n", core_id);
		return;
	}

	RTE_LOG(INFO, USER1, 
		"Core %u is doing RX for port %u id %u\n", core_id, core_params->port_id, core_params->id);

	if (core_params->port_id >= udpi.n_ports)
	{
		rte_panic("Core %u misconfiguration port_id %u\n", core_id, core_params->port_id);
		return;
	}

	enum udpi_port_type port_type = udpi.ports[core_params->port_id].port_type;
	if (port_type == UDPI_PORT_NONE)
	{
		rte_panic("port %u misconfiguration port_type %u\n", core_params->port_id, port_type);
		return;
	}
	
	UDPI_LOG_COPY_DPDK_LEVEL();	

	UDPI_INFO("log in %u\n", core_id);
	

	struct rte_mbuf *pkts[udpi.bsz_hwq_rd];
	
	const uint64_t timer_precision = rte_get_tsc_hz();
	uint64_t prev_tsc = rte_get_tsc_cycles();
	uint64_t cur_tsc = prev_tsc, last_tsc = 0, interval = 0, diff_tsc = 0;
	bool is_busy = false;

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
		if (unlikely(diff_tsc > timer_precision)) {
			is_busy = true;
			prev_tsc = cur_tsc;
			udpi_check_msg(core_params->id);
			UDPI_DEBUG("debug in %u\n", core_id);
			//do somthing
		}

		int n_mbufs = rte_eth_rx_burst(core_params->port_id, core_params->id, pkts, udpi.bsz_hwq_rd);
		if(n_mbufs < 1)
		{
			continue;
		}

		is_busy = true;
			
		int i = 0;
		for (i = 0; i < PREFETCH_OFFSET && i < n_mbufs; i++) {
			rte_prefetch0(rte_pktmbuf_mtod(pkts[i], void *));
			rte_prefetch0(&pkts[i]->cacheline1);
		}	

		for (i = 0; i < (n_mbufs - PREFETCH_OFFSET); i++) {
			rte_prefetch0(rte_pktmbuf_mtod(pkts[i + PREFETCH_OFFSET], void *));
			rte_prefetch0(&pkts[i + PREFETCH_OFFSET]->cacheline1);
			udpi_pkt_metadata_fill(pkts[i], core_params->id, port_type);
		}
		
		for (; i < n_mbufs; i++) {
			udpi_pkt_metadata_fill(pkts[i], core_params->id, port_type);
		}

	}
	
	return;
}


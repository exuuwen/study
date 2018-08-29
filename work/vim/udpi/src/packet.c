/*-
 *
 * Copyright(c) UCloud. All rights reserved.
 * All rights reserved.
 *
 */

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
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
#include "socket.h"

#define PREFETCH_OFFSET 3

struct packet_header {
	uint32_t tot_len;
	uint32_t cmd;
	uint32_t seq;
	uint16_t length;
}__attribute__((__packed__)); 

static struct packet_header header[UDPI_MAX_PACKET];

static void udpi_packet_mbuf(struct rte_mbuf *pkt, uint32_t id)
{
	if (udpi.packet_fds[id] < 0)
	{
		rte_atomic64_inc(&udpi.overflow.packet_fail);
		return;
	}

	uint8_t *data = rte_pktmbuf_mtod(pkt, uint8_t *);

	if (data)
	{
		int ret;
		uint16_t length = pkt->pkt_len;
		header[id].length = rte_cpu_to_be_16(length);
		header[id].tot_len = rte_cpu_to_be_32(length + sizeof((header[id])));
			
		ret = udpi_socket_packet_write(id, (uint8_t*)(&header[id]), sizeof(header[id]));
		if (ret < 0)
		{
			printf("packet cmd write fail\n");
			rte_atomic64_inc(&udpi.overflow.packet_fail);
			return;
		}
		ret = udpi_socket_packet_write(id, data, length);
		if (ret < 0)
		{
			printf("packet write fail\n");
			rte_atomic64_inc(&udpi.overflow.packet_fail);
			return;
		}
	}
}

void udpi_main_loop_packet(void)
{
	uint32_t core_id = rte_lcore_id();
	
	struct udpi_core_params *core_params 
		= udpi_get_core_params(core_id);

	if((!core_params) 
		|| (core_params->core_type != UDPI_CORE_PACKET))
	{
		rte_panic("Core %u misconfiguration\n", core_id);
		return;
	}


	if (core_params->id >= UDPI_MAX_PACKET)
	{
		rte_panic("Core %u misconfiguration id %u\n", core_id, core_params->id);
		return;
	}

	uint32_t id = core_params->id;
	
	RTE_LOG(INFO, USER1, "Core %u is doing PACKET with id %u\n", core_id, id);

	udpi_socket_packet_create(id);
	
	struct rte_mbuf *pkts[udpi.bsz_hwq_rd];
	int j = 0, n_mbufs = 0;
	
	uint64_t lcore_tsc_hz = rte_get_timer_hz();

	uint64_t prev_tsc = rte_get_tsc_cycles();
	uint64_t cur_tsc = prev_tsc, last_tsc = 0, interval = 0, diff_tsc = 0;
	char is_busy = false;
	uint64_t timer_precision = lcore_tsc_hz;

	header[0].cmd = rte_cpu_to_be_32(4098);
	header[1].cmd = rte_cpu_to_be_32(4098);
	header[2].cmd = rte_cpu_to_be_32(4098);
	header[3].cmd = rte_cpu_to_be_32(4098);

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
			if (udpi.packet_fds[id] > 0)
				udpi_socket_packet_read(id);
			else 
				udpi_socket_packet_create(id);
		}

		n_mbufs = rte_ring_dequeue_burst(udpi.packet_ring[id], (void **)pkts, udpi.bsz_swq_rd*2);
		if(n_mbufs < 1)
		{
			continue;
		}

		is_busy = true;
		
		for (j = 0; j < PREFETCH_OFFSET && j < n_mbufs; j++) {
			rte_prefetch0(rte_pktmbuf_mtod(pkts[j], void *));
			rte_prefetch0(&pkts[j]->cacheline1);
		}

		for (j = 0; j < (n_mbufs - PREFETCH_OFFSET); j++) {
			rte_prefetch0(rte_pktmbuf_mtod(pkts[j + PREFETCH_OFFSET], void *));
			rte_prefetch0(&pkts[j + PREFETCH_OFFSET]->cacheline1);
			udpi_packet_mbuf(pkts[j], id);	
			rte_pktmbuf_free(pkts[j]);
		}

		for (; j < n_mbufs; j++) {
			udpi_packet_mbuf(pkts[j], id);	
			rte_pktmbuf_free(pkts[j]);
		}

	}
	
	return;
}



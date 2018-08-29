/*-
 *
 * Copyright(c) UCloud. All rights reserved.
 * All rights reserved.
 *
 */

#include <rte_cycles.h>
#include <rte_debug.h>
#include <rte_malloc.h>
#include <rte_malloc.h>
#include <rte_hash.h>
#include <rte_jhash.h>

#include "main.h"
#include "dpi.h"
#include "stats.h"
#include "socket.h"
#include "task.h"

struct udpi_params udpi = {
	/* Ports */
	.rsz_hwq_rx = 4096,
	.rsz_hwq_tx = 4096,
	.bsz_hwq_rd = 512,
	.bsz_hwq_wr = 64,

	.n_workers = 0,
	.n_dumpers = 0,

	.debug = 0,
	.hash_id = 0,
	.comp_hash_id = 0,

	/* SWQs */
	.rsz_swq = 8192,
	.bsz_swq_rd = 256,
	.bsz_swq_wr = 64,

	/* Buffer pool */
	.pool_buffer_size = 2048 + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM,
	.pool_size = 256 * 1024,
	.pool_cache_size = 256,

	/* Message buffer pool */
	.msg_pool_buffer_size = 256,
	.msg_pool_size = 1024,
	.msg_pool_cache_size = 64,

	/* Hash params*/
	.eip_params = {
		.entries = 16384,
//		.bucket_entries = 16,
		.key_len = sizeof(struct eip_key), /* eip */
		.hash_func = rte_jhash,
		.hash_func_init_val = 0,
		.socket_id = 0,
	},

	.flow_params = {
		.entries = 1 << 25,
//		.bucket_entries = 16,
		.key_len = sizeof(struct flow_key), /* 14 */
		.hash_func = rte_jhash,
		.hash_func_init_val = 0,
		.socket_id = 0,
	},

	.addr_params = {
		.entries = 32768*2,
//		.bucket_entries = 16,
		.key_len = sizeof(uint32_t), /* eip net */
		.hash_func = rte_jhash,
		.hash_func_init_val = 0,
		.socket_id = 0,
	},

	.comp_addr_params = {
		.entries = 32768,
//		.bucket_entries = 16,
		.key_len = sizeof(uint32_t), /* eip net */
		.hash_func = rte_jhash,
		.hash_func_init_val = 0,
		.socket_id = 0,
	},
};

struct udpi_core_params *
udpi_get_core_params(uint32_t core_id)
{
	uint32_t i;

	for (i = 0; i < RTE_MAX_LCORE; i++) {
		struct udpi_core_params *p = &udpi.cores[i];

		if (p->core_id != core_id)
			continue;

		return p;
	}

	return NULL;
}

static void udpi_init_mbuf_pools(void)
{
	/* Init the buffer pool */
	RTE_LOG(INFO, MEMPOOL, "Creating the mbuf pool ...\n");

	/* Init the indirect buffer pool */
	/*RTE_LOG(INFO, MEMPOOL, "Creating the indirect mbuf pool ...\n");
	udpi.indirect_pool = rte_mempool_create("indirect mempool", 
		udpi.pool_size, 
		sizeof(struct rte_mbuf), udpi.pool_cache_size, 0, NULL, NULL,
		rte_pktmbuf_init, NULL, rte_socket_id(), 0);
	if(NULL == udpi.indirect_pool)
	{
		rte_panic("Cannot create indirect mbuf pool\n");
	}*/
	
	/* Init the message buffer pool */
	RTE_LOG(INFO, MEMPOOL, "Creating the message mbuf pool ...\n");
	udpi.msg_pool = rte_mempool_create("msg mempool ", 
		udpi.msg_pool_size,
		udpi.msg_pool_buffer_size,
		udpi.msg_pool_cache_size,
		0, NULL, NULL, 
		rte_ctrlmbuf_init, NULL,
		rte_socket_id(), 0);
	if(NULL == udpi.msg_pool)
	{
		rte_panic("Cannot create message mbuf pool\n");
	}

	return;
}

static void udpi_ports_check_link(void)
{
	struct rte_eth_link link;
	uint32_t port_id;

	for (port_id=0; port_id<udpi.n_ports; port_id++)
	{
		memset(&link, 0, sizeof(struct rte_eth_link));
		rte_eth_link_get_nowait(port_id, &link);
		RTE_LOG(INFO, PORT, "Port %u (%u Gbps) %s\n",
			port_id, link.link_speed / 1000, link.link_status ? "UP":"DOWN");
	}
		
	return;
}

static void udpi_init_ports(void)
{
	/* Init NIC ports, then start the ports */
	int32_t ret = -1;
	uint32_t queue_id = 0;
	uint32_t lcore_id = 0;
	uint32_t i, j;


	/* Init port */
	for (j=0; j<udpi.n_ports; j++)
	{
		RTE_LOG(INFO, PORT, "Initializing NIC port %u ...\n", j);
		fflush(stdout);
		struct rte_eth_dev_info info;
		rte_eth_dev_info_get(j, &info);

		struct rte_eth_txconf tx_conf;
		tx_conf = info.default_txconf;
		//tx_conf.txq_flags = 0;

		struct rte_eth_rxconf rx_conf;
		rx_conf = info.default_rxconf;
		rx_conf.rx_drop_en = 1;

		struct rte_eth_conf port_conf;
		memset(&port_conf, 0, sizeof(port_conf));
		port_conf.rxmode.mq_mode = ETH_MQ_RX_RSS;
		port_conf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_IPV4;
		port_conf.rxmode.hw_vlan_strip = 1;
		port_conf.rxmode.hw_strip_crc = 1;
		port_conf.rxmode.hw_vlan_extend = 1;

		ret = rte_eth_dev_configure(j, udpi.ports[j].n_queues, udpi.ports[j].n_queues, &port_conf);
		if(0 > ret)
		{
			rte_panic("Cannot init NIC port %u (%d)\n", j, ret);
		}

		for (i=0; i<udpi.n_cores; i++) 
		{
			if (udpi.cores[i].core_type != UDPI_CORE_IPV4_RX || udpi.cores[i].port_id != j)
				continue;
        
			lcore_id = udpi.cores[i].core_id;
			queue_id = udpi.cores[i].id;

			char name[32];
			snprintf(name, sizeof(name), "mempool_%u_%u", j, queue_id);
			/*
				udpi.pool[queue_id] = rte_mempool_create(name, 
				udpi.pool_size,
				udpi.pool_buffer_size, 
				RTE_MEMPOOL_CACHE_MAX_SIZE, 
				sizeof(struct rte_pktmbuf_pool_private),
				rte_pktmbuf_pool_init, 
				NULL, 
				rte_pktmbuf_init, 
				NULL, 
				rte_lcore_to_socket_id(lcore_id), 
				0);
			*/
			udpi.pool[queue_id] = rte_pktmbuf_pool_create(name, udpi.pool_size, udpi.pool_cache_size, 0, udpi.pool_buffer_size, rte_lcore_to_socket_id(lcore_id));

			if(NULL == udpi.pool[queue_id])
			{
				rte_panic("Cannot create mbuf pool\n");
			}

			ret = rte_eth_rx_queue_setup(j, queue_id, udpi.rsz_hwq_rx, 
				rte_lcore_to_socket_id(lcore_id), &rx_conf, udpi.pool[queue_id]);
			if (0 > ret)
			{
				rte_panic("Cannot init Rx for port %u with qid %u (%d)\n", j, queue_id, ret);
			}

			ret = rte_eth_dev_set_rx_queue_stats_mapping(j, queue_id, queue_id);
			if (ret < 0) 
			{
				rte_panic("Failed to set rx_queue_stats_mapping of p%u_q%u. error = %d.",
					j, queue_id, ret);
			}

			ret = rte_eth_tx_queue_setup(j, queue_id, udpi.rsz_hwq_tx, rte_lcore_to_socket_id(lcore_id), &tx_conf);
			if (0 > ret)
			{
				rte_panic("Cannot init Tx for port %u with qid %u (%d)\n", j, queue_id, ret);
			}
	
			ret = rte_eth_dev_set_tx_queue_stats_mapping(j, queue_id, queue_id);
			if (ret < 0) 
			{
				rte_panic("Failed to set tx_queue_stats_mapping of p%u_q%u. error = %d.", j, queue_id, ret);
			}
		}  

		/* Start port */
		ret = rte_eth_dev_start(j);
		if(0 > ret)
		{
			rte_panic("Cannot start port %u (%d)\n", j, ret);
		}

		rte_eth_promiscuous_enable(j);
	}

	udpi_ports_check_link();

	return;
}

static void udpi_init_rings(void)
{
	uint32_t n_swq, i;

	n_swq = udpi.n_workers;
	RTE_LOG(INFO, USER1, "Initializing %u SW rings for dpi\n", n_swq);

	udpi.rings = (struct rte_ring**)rte_malloc_socket(NULL, n_swq * sizeof(struct rte_ring *),
		RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (udpi.rings == NULL)
		rte_panic("Cannot allocate memory to store ring pointers\n");

	for (i = 0; i < n_swq; i++) {
		struct rte_ring *ring;
		char name[32];

		snprintf(name, sizeof(name), "udpi_ring_%u", i);

		ring = rte_ring_create(
			name,
			udpi.rsz_swq,
			rte_socket_id(),
			RING_F_SC_DEQ);

		if (ring == NULL)
			rte_panic("Cannot create ring %u\n", i);

		udpi.rings[i] = ring;
	}

	n_swq = udpi.n_dumpers - 1;
	RTE_LOG(INFO, USER1, "Initializing %u SW rings for stats\n", n_swq);

	udpi.stats_rings = (struct rte_ring**)rte_malloc_socket(NULL, n_swq * sizeof(struct rte_ring *),
		RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (udpi.stats_rings == NULL)
		rte_panic("Cannot allocate memory to store ring pointers\n");

	for (i = 0; i < n_swq; i++) {
		struct rte_ring *ring;
		char name[32];

		snprintf(name, sizeof(name), "udpi_stats_ing_%u", i);

		ring = rte_ring_create(
			name,
			16,
			rte_socket_id(),
			RING_F_SC_DEQ|RING_F_SP_ENQ);

		if (ring == NULL)
			rte_panic("Cannot create stats ring %u\n", i);

		udpi.stats_rings[i] = ring;
	}


	n_swq = udpi.n_packets;
	RTE_LOG(INFO, USER1, "Initializing %u SW rings for packets\n", n_swq);

	for (i = 0; i< n_swq; i++) {
		char name[32];
		snprintf(name, sizeof(name), "udpi_packet_ring%u", i);
		udpi.packet_ring[i] = rte_ring_create(
			name,
			udpi.rsz_swq*16,
			rte_socket_id(),
			RING_F_SC_DEQ);

		if (udpi.packet_ring[i] == NULL)
			rte_panic("Cannot create packet ring %u\n", i);
	}
}

static void udpi_init_hash(void)
{
	uint32_t i;

	for(i=0; i<udpi.n_workers; i++)
	{
		char name[15];
		memset(name, 0, sizeof(name));
		sprintf(name, "eip_hash_%x", i);

		struct rte_hash_parameters eip_params;
		eip_params = udpi.eip_params;
		eip_params.name = name;

		udpi.hash[i].eip_hash = rte_hash_create(&eip_params);
		if (udpi.hash[i].eip_hash == NULL)
			rte_panic("Cannot create eip hash for worker %u\n", i);

		udpi.hash[i].eip_entry = (struct eip_entry*)rte_zmalloc(name, sizeof(struct eip_entry)*(udpi.eip_params.entries), 0);
		if (udpi.hash[i].eip_entry == NULL)
			rte_panic("Cannot malloc eip hash entry for worker %u\n", i);
		
		memset(name, 0, sizeof(name));
		sprintf(name, "flow_hash_%x", i);

		struct rte_hash_parameters flow_params;
		flow_params = udpi.flow_params;
        	flow_params.name = name;

		udpi.hash[i].flow_hash = rte_hash_create(&flow_params);
		if (udpi.hash[i].flow_hash == NULL)
			rte_panic("Cannot create flow hash for worker %u\n", i);

		udpi.hash[i].flow_entry = (struct flow_entry**)rte_zmalloc(name, sizeof(struct flow_entry*)*(udpi.flow_params.entries), 0);
		if (udpi.hash[i].flow_entry == NULL)
			rte_panic("Cannot malloc flow hash entry for worker %u\n", i);
		
		rte_atomic64_init(&udpi.hash[i].eip_entries);
	}

	char name[20];
	memset(name, 0, sizeof(name));
	sprintf(name, "addr_hash_0");

	struct rte_hash_parameters addr_params;
	addr_params = udpi.addr_params;
	addr_params.name = name;

	udpi.addr_hash[0] = rte_hash_create(&addr_params);
	if (udpi.addr_hash[0] == NULL)
		rte_panic("Cannot create addr hash 0\n");

	sprintf(name, "addr_hash_1");

	addr_params = udpi.addr_params;
	addr_params.name = name;

	udpi.addr_hash[1] = rte_hash_create(&addr_params);
	if (udpi.addr_hash[1] == NULL)
		rte_panic("Cannot create addr hash 1\n");

	memset(name, 0, sizeof(name));
	sprintf(name, "comp_addr_hash_0");

	struct rte_hash_parameters comp_addr_params;
	comp_addr_params = udpi.comp_addr_params;
	comp_addr_params.name = name;

	udpi.comp_addr_hash[0] = rte_hash_create(&comp_addr_params);
	if (udpi.comp_addr_hash[0] == NULL)
		rte_panic("Cannot create comp addr hash 0\n");

	sprintf(name, "comp_addr_hash_1");

	comp_addr_params = udpi.comp_addr_params;
	comp_addr_params.name = name;

	udpi.comp_addr_hash[1] = rte_hash_create(&comp_addr_params);
	if (udpi.comp_addr_hash[1] == NULL)
		rte_panic("Cannot create comp addr hash 1\n");
}

static void udpi_init_stats(void)
{
	udpi_stats_init(&udpi.stats);
	rte_atomic64_init(&udpi.eip_entries);
}

static void udpi_init_closed_flows(void)
{
	int i;
	
	for (i=0; i<TCP_CONNTRACK_MAX; i++)
		rte_atomic64_init(&udpi.closed_flows[i]);
}

static void udpi_init_overflow(void)
{
	 rte_atomic64_init(&udpi.overflow.eip);
	 rte_atomic64_init(&udpi.overflow.flow);
	 rte_atomic64_init(&udpi.overflow.notsyn);
	 rte_atomic64_init(&udpi.overflow.cps);
	 rte_atomic64_init(&udpi.overflow.packet);
	 rte_atomic64_init(&udpi.overflow.packet_packet);
	 rte_atomic64_init(&udpi.overflow.packet_mem);
	 rte_atomic64_init(&udpi.overflow.packet_fail);
}

void udpi_init(void)
{
	udpi_init_mbuf_pools();
	udpi_init_rings();
	udpi_init_hash();
	udpi_init_stats();
	udpi_init_ports();
	udpi_init_socket();
	udpi_init_closed_flows();
	udpi_init_overflow();
	
	udpi_task_iplist();
	udpi_task_complist();
	
	RTE_LOG(INFO, USER1, "Initialization completed\n");

	return;
}

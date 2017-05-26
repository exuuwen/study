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

#include "log.h"
#include "main.h"

struct udpi_params udpi = {
	/* Ports */
	.rsz_hwq_rx = 4096,
	.rsz_hwq_tx = 128,
	.bsz_hwq_rd = 512,
	.bsz_hwq_wr = 64,

	.port_conf = {
		.rxmode = {
			//.split_hdr_size = 0,
			.header_split = 0, 								/* Header Split disabled */
			//.hw_ip_checksum = 1, 							/* IP checksum offload enabled */
			.hw_vlan_filter = 0, 							/* VLAN filtering disabled */
			.jumbo_frame = 0,	 							/* Jumbo Frame Support enabled */
			//.max_rx_pkt_len = 9000,
			.hw_strip_crc = 1, 								/* CRC stripped by hardware */
			.mq_mode = ETH_MQ_RX_RSS, 							/* CRC stripped by hardware */
		},
		.rx_adv_conf = {
			.rss_conf = {
				.rss_key = NULL,
				.rss_hf = ETH_RSS_PROTO_MASK,
			},
		},
		.txmode = {
			.mq_mode = ETH_MQ_TX_NONE,
		},
		.fdir_conf = {
			.mode = RTE_FDIR_MODE_PERFECT,
			.pballoc = RTE_FDIR_PBALLOC_64K,
			.status = RTE_FDIR_REPORT_STATUS,
			.drop_queue = 127,
			.mask = {
				.ipv4_mask     = {
					.src_ip = 0xFFFFFFFF,
					.dst_ip = 0xFFFFFFFF,
				},
			},
		}
	},

	.flexbytes_enable = 1,

	/* SWQs */
	.rsz_swq = 8192,
	.bsz_swq_rd = 256,
	.bsz_swq_wr = 64,

	/* Buffer pool */
	.pool_buffer_size = 2048 + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM,
	.pool_size = 512 * 4096,
	.pool_cache_size = 256,

	/* Message buffer pool */
	.msg_pool_buffer_size = 256,
	.msg_pool_size = 1024,
	.msg_pool_cache_size = 64,
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
	udpi.pool = rte_mempool_create("mempool", 
		udpi.pool_size,
		udpi.pool_buffer_size, 
		udpi.pool_cache_size, 
		sizeof(struct rte_pktmbuf_pool_private),
		rte_pktmbuf_pool_init, 
		NULL, 
		rte_pktmbuf_init, 
		NULL, 
		rte_socket_id(), 
		0);
	if(NULL == udpi.pool)
	{
		rte_panic("Cannot create mbuf pool\n");
	}

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

static void
udpi_update_reta(uint8_t port_id, uint16_t queues)
{
	struct rte_eth_dev_info dev_info;
	struct rte_eth_rss_reta_entry64 reta_conf[UDPI_RETA_SIZE_MAX];
	uint32_t i;
	int status;

    	/* Get RETA size */
	memset(&dev_info, 0, sizeof(dev_info));
	rte_eth_dev_info_get(port_id, &dev_info);

	if (dev_info.reta_size == 0)
		rte_panic("port %u RSS setup error (null RETA size)\n", port_id);

	if (dev_info.reta_size > ETH_RSS_RETA_SIZE_512)
		rte_panic("Port %u RSS setup error (RETA size too big)\n", port_id);

	RTE_LOG(INFO, USER1, "Port %u reta_size %u queues %u\n", port_id, dev_info.reta_size, queues);

	/* Setup RETA contents */
	memset(reta_conf, 0, sizeof(reta_conf));

	for (i = 0; i < dev_info.reta_size; i++)
		reta_conf[i / RTE_RETA_GROUP_SIZE].mask = UINT64_MAX;

	for (i = 0; i < dev_info.reta_size; i++) {
		uint32_t reta_id = i / RTE_RETA_GROUP_SIZE;
		uint32_t reta_pos = i % RTE_RETA_GROUP_SIZE;
		uint16_t rss_qs = i % queues;

		reta_conf[reta_id].reta[reta_pos] = rss_qs;
	}

	/* RETA update */
	status = rte_eth_dev_rss_reta_update(port_id,
		reta_conf,
		dev_info.reta_size);
	if (status != 0)
		rte_panic("port %u: RSS setup error (RETA update failed)\n", port_id);
}

static void udpi_add_fdir(uint32_t port_id, uint8_t queue_id, uint32_t item_id)
{
	struct rte_eth_fdir_filter entry;
	int ret = 0;

	ret = rte_eth_dev_filter_supported(port_id, RTE_ETH_FILTER_FDIR);
	if (ret < 0) {
		rte_panic("flow director is not supported on port %u.\n",
			port_id);
		return;
	}

	memset(&entry, 0, sizeof(struct rte_eth_fdir_filter));
	
	if (udpi.flexbytes_enable) {
		entry.input.flow_type = RTE_ETH_FLOW_NONFRAG_IPV4_OTHER;
		entry.input.flow.ip4_flow.dst_ip = inet_addr("172.168.1.62");
		entry.input.flow.ip4_flow.src_ip = inet_addr("172.168.1.61");
		entry.input.flow_ext.flexbytes[0] = 0x08;
		entry.input.flow_ext.flexbytes[1] = 0x68;

	}
	else {
		entry.input.flow_type = RTE_ETH_FLOW_NONFRAG_IPV4_UDP;
		entry.input.flow.udp4_flow.ip.dst_ip = inet_addr("172.168.1.62");
		entry.input.flow.udp4_flow.ip.src_ip = inet_addr("172.168.1.61");
		entry.input.flow.udp4_flow.dst_port = htons(5001);
		entry.input.flow.udp4_flow.src_port = htons(2152);
	}

	printf("%x,%x,%x,%X\n", entry.input.flow.ip4_flow.dst_ip, entry.input.flow.ip4_flow.src_ip, entry.input.flow.udp4_flow.dst_port, entry.input.flow.udp4_flow.src_port);

	entry.action.report_status = RTE_ETH_FDIR_REPORT_ID;
	entry.action.behavior = RTE_ETH_FDIR_ACCEPT;
	entry.action.rx_queue = queue_id;
	entry.soft_id = item_id;

	ret = rte_eth_dev_filter_ctrl(port_id, RTE_ETH_FILTER_FDIR,
					     RTE_ETH_FILTER_ADD, &entry);
	if (ret < 0)
		rte_panic("flow director programming error: (%s)\n",
			strerror(-ret));
}

static void udpi_init_ports(void)
{
	/* Init NIC ports, then start the ports */
	int32_t ret = -1;
	uint32_t queue_id = 0;
	uint32_t lcore_id = 0;
	uint32_t i, j;

	if (udpi.flexbytes_enable)
        {
		struct rte_eth_fdir_flex_conf flex_conf;
		flex_conf.nb_payloads = 1;
		flex_conf.nb_flexmasks = 1;
		flex_conf.flex_set[0].type = RTE_ETH_RAW_PAYLOAD;
		flex_conf.flex_set[0].src_offset[0] = 34;
		flex_conf.flex_set[0].src_offset[1] = 35;

		flex_conf.flex_mask[0].flow_type = RTE_ETH_FLOW_UNKNOWN;
		flex_conf.flex_mask[0].mask[0] = 0xff;
		flex_conf.flex_mask[0].mask[1] = 0xff;

		udpi.port_conf.fdir_conf.flex_conf = flex_conf;
	}
	else
	{
		 udpi.port_conf.fdir_conf.mask.src_port_mask = 0xffff;
		 udpi.port_conf.fdir_conf.mask.dst_port_mask = 0xffff;
	}
	

	/* Init port */
	for (j=0; j<udpi.n_ports; j++)
	{
		RTE_LOG(INFO, PORT, "Initializing NIC port %u ...\n", j);
		fflush(stdout);

		ret = rte_eth_dev_configure(j, udpi.ports[j].n_queues, udpi.ports[j].n_queues, &udpi.port_conf);
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

			ret = rte_eth_rx_queue_setup(j, queue_id, udpi.rsz_hwq_rx, 
				rte_lcore_to_socket_id(lcore_id), NULL, udpi.pool);
			if (0 > ret)
			{
				rte_panic("Cannot init Rx for port %u with qid %u (%d)\n", j, queue_id, ret);
			}
			
			ret = rte_eth_dev_set_rx_queue_stats_mapping(j, queue_id, queue_id);
			if (ret < 0) 
			{
				rte_panic("Failed to set tx_queue_stats_mapping of p%u_q%u. error = %d.",
					j, queue_id, ret);
			}
                
			ret = rte_eth_tx_queue_setup(j, queue_id, udpi.rsz_hwq_tx, rte_lcore_to_socket_id(lcore_id), NULL);
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

		udpi_update_reta(j,  udpi.ports[j].n_queues - 1);

		udpi_add_fdir(j, 3, 1);

		rte_eth_promiscuous_enable(j);
	}

	udpi_ports_check_link();

	return;
}

static void udpi_init_rings(void)
{
	uint32_t n_swq, i;

	n_swq = udpi.n_workers ;
	RTE_LOG(INFO, USER1, "Initializing %u SW rings for ctrlmsg\n", n_swq);

	udpi.msg_rings = (struct rte_ring**)rte_malloc_socket(NULL, n_swq * sizeof(struct rte_ring *),
		RTE_CACHE_LINE_SIZE, rte_socket_id());
	if (udpi.msg_rings == NULL)
		rte_panic("Cannot allocate memory to store ring pointers\n");

	for (i = 0; i < n_swq; i++) {
		struct rte_ring *ring;
		char name[32];

		snprintf(name, sizeof(name), "udpi_ctrlmsg_%u", i);

		ring = rte_ring_create(
			name,
			16,
			rte_socket_id(),
			RING_F_SC_DEQ|RING_F_SP_ENQ);

		if (ring == NULL)
			rte_panic("Cannot create ctrlmsg ring %u\n", i);

		udpi.msg_rings[i] = ring;
	}
}

static void udpi_init_log(void)
{
	set_dpdk_log_level(udpi.log.level);
	UDPI_LOG_COPY_DPDK_LEVEL();	
	udpi.log.is_trace_enabled = 0;
}

static void udpi_init_pdump(void)
{
	rte_pdump_init("/root/watch_dir/pdump_unix");
}

void udpi_init(void)
{
	udpi_init_pdump();
	udpi_init_log();
	udpi_init_mbuf_pools();
	udpi_init_ports();
	udpi_init_rings();
	
	RTE_LOG(INFO, USER1, "Initialization completed\n");

	return;
}

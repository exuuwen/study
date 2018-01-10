
    .rx_adv_conf = {
        .rss_conf = {
            .rss_key = NULL,
            .rss_hf = ETH_RSS_IP | ETH_RSS_UDP | ETH_RSS_TCP | ETH_RSS_SCTP,
        },
    },

static int 
dpdk_flow_set_ipaddr(uint8_t port_id, ovs_be32 ipaddr, uint16_t queue_id /*uint32_t tunnel_id,*/)
{
    int ret;

    struct rte_eth_tunnel_filter_conf conf;

	memset(&conf, 0, sizeof(conf));

	/*
	//0x3e8
    //struct in_addr new_addr;
	uint32_t tenant_id_be = 0;
    uint8_t tni[3] = {0, 0, 0x03};

    rte_memcpy(((uint8_t *)&tenant_id_be + 1), tni, 3);
    conf.tenant_id = rte_be_to_cpu_32(tenant_id_be);
	*/

    conf.ip_type = 0;//I40E_TUNNEL_IPTYPE_IPV4;
    conf.queue_id = queue_id;
    conf.tunnel_type = 4;//I40E_TUNNEL_TYPE_NVGRE;

     //conf.inner_mac = info->macaddr;
     //conf.filter_type = ETH_TUNNEL_FILTER_IMAC;
     //conf.filter_type = ETH_TUNNEL_FILTER_TENID |  ETH_TUNNEL_FILTER_IMAC;

    conf.ip_addr.ipv4_addr = ipaddr;
    conf.filter_type = ETH_TUNNEL_FILTER_IIP;


	ret = rte_eth_dev_filter_ctrl(port_id, RTE_ETH_FILTER_TUNNEL,
					     RTE_ETH_FILTER_ADD, &conf);
    if (ret < 0) {
        VLOG_INFO("flow ip addr 0x%x filter ctl fail %d\n", ipaddr, ret);
        return -1;
    }

	VLOG_INFO("flow ipaddr 0x%x set is correct\n", ipaddr);

    return ret;
}

static int 
dpdk_flowdir_set(uint8_t port_id, uint32_t tunnel_id, uint16_t queue_id, uint32_t item_id)
{
	struct rte_eth_fdir_filter entry;
	int ret = 0;

	ret = rte_eth_dev_filter_supported(port_id, RTE_ETH_FILTER_FDIR);
	if (ret < 0) {
		VLOG_INFO("flow director is not supported on port %u.\n",
			port_id);
		return -1;
	}

	struct rte_eth_fdir_filter_info info; 

	memset(&info, 0, sizeof(info));
	info.info_type = RTE_ETH_FDIR_FILTER_INPUT_SET_SELECT;
	info.info.input_set_conf.flow_type = RTE_ETH_FLOW_NONFRAG_IPV4_UDP;
	info.info.input_set_conf.field[0] = RTE_ETH_INPUT_SET_NONE;
	info.info.input_set_conf.inset_size = 1;
	info.info.input_set_conf.op = RTE_ETH_INPUT_SET_SELECT;
	ret = rte_eth_dev_filter_ctrl(port_id, RTE_ETH_FILTER_FDIR,
	        RTE_ETH_FILTER_SET, &info);
	if (ret < 0) {
		VLOG_INFO("flow director input_set clear fail %u.\n",
			port_id);
		return -1;
	}

	memset(&info, 0, sizeof(info));
	info.info_type = RTE_ETH_FDIR_FILTER_INPUT_SET_SELECT;
	info.info.input_set_conf.flow_type = RTE_ETH_FLOW_NONFRAG_IPV4_UDP;
	info.info.input_set_conf.field[0] = RTE_ETH_INPUT_SET_L3_SRC_IP4;
	//info.info.input_set_conf.field[1] = RTE_ETH_INPUT_SET_L3_DST_IP4;
	info.info.input_set_conf.inset_size = 1;
	info.info.input_set_conf.op = RTE_ETH_INPUT_SET_ADD;
	rte_eth_dev_filter_ctrl(port_id, RTE_ETH_FILTER_FDIR,
	        RTE_ETH_FILTER_SET, &info);
	if (ret < 0) {
		VLOG_INFO("flow director input_set add fail %u.\n",
			port_id);
		return -1;
	}

	memset(&entry, 0, sizeof(struct rte_eth_fdir_filter));
	
	entry.input.flow_type = RTE_ETH_FLOW_NONFRAG_IPV4_UDP;
	//entry.input.flow.ip4_flow.dst_ip = inet_addr("10.0.0.9");
	//entry.input.flow.ip4_flow.src_ip = inet_addr("10.0.0.7");
	//entry.input.flow.udp4_flow.ip.dst_ip = inet_addr("172.168.0.203");
	//entry.input.flow.udp4_flow.ip.src_ip = inet_addr("172.168.0.72");
	//entry.input.flow.udp4_flow.ip.dst_ip = inet_addr("10.0.0.9");
	entry.input.flow.udp4_flow.ip.src_ip = inet_addr("10.0.0.7");
	//entry.input.flow.udp4_flow.dst_port = htons(5001);
	//entry.input.flow.udp4_flow.src_port = htons(54727);
	//entry.input.flow_ext.flexbytes[0] = 0x08;
	//entry.input.flow_ext.flexbytes[1] = 0x68;


	entry.action.report_status = RTE_ETH_FDIR_REPORT_ID;
	entry.action.behavior = RTE_ETH_FDIR_ACCEPT;
	entry.action.rx_queue = queue_id;
	entry.soft_id = item_id;

	ret = rte_eth_dev_filter_ctrl(port_id, RTE_ETH_FILTER_FDIR,
					     RTE_ETH_FILTER_ADD, &entry);
	if (ret < 0)
		VLOG_INFO("flow director programming error: (%s)\n",
			strerror(-ret));
    else
		VLOG_INFO("set flow correct....\n");

	return ret;
}


static int 
dpdk_flow_set_mac(uint8_t port_id, struct ether_addr mac, uint16_t queue_id)
{
    /*struct rte_flow_item_nvgre flow_item_eth_type_tunnel, flow_item_eth_mask_type_tunnel,
	flow_item_eth_type_tunnel.tni = xxx.
	flow_item_eth_mask_type_tunnel.tni = ff,ff,ff
	*/

	static struct rte_flow_item_eth flow_item_eth_type_eth, flow_item_eth_mask_type_eth;
	
	memset(&flow_item_eth_type_eth, 0, sizeof(flow_item_eth_type_eth));
	memset(&flow_item_eth_mask_type_eth, 0, sizeof(flow_item_eth_mask_type_eth));

	flow_item_eth_type_eth.dst = mac;
	flow_item_eth_mask_type_eth.dst = broadcast_mac;

    static struct rte_flow_item flow_item_tunnel[] = {
        {
            .type = RTE_FLOW_ITEM_TYPE_ETH,
            .spec = NULL,
            .last = NULL,
            .mask = NULL,
        },
        {
            .type = RTE_FLOW_ITEM_TYPE_IPV4,
            .spec = NULL,
            .last = NULL,
            .mask = NULL,
        },
        {
            .type = RTE_FLOW_ITEM_TYPE_NVGRE,
            .spec = NULL,
            .last = NULL,
            .mask = NULL,
        },
        {
            .type = RTE_FLOW_ITEM_TYPE_ETH,
            .spec = &flow_item_eth_type_eth,
            .last = NULL,
            .mask = &flow_item_eth_mask_type_eth,
        },
        {
            .type = RTE_FLOW_ITEM_TYPE_END,
            .spec = NULL,
            .last = NULL,
            .mask = NULL,
        }
    };

    const struct rte_flow_attr flow_attr_tunnel = {
        .group = 0,
	    .priority = 0,
	    .ingress = 1,
	    .egress = 0,
	    .reserved = 0,
    };

    struct rte_flow_action_queue queue_conf_tunnel = {
        .index = queue_id,
    };

    const struct rte_flow_action flow_action_tunnel[] = {
        {    
            .type = RTE_FLOW_ACTION_TYPE_PF,
        },   
        {    
            .type = RTE_FLOW_ACTION_TYPE_QUEUE,
            .conf = &queue_conf_tunnel
        },   
        {    
            .type = RTE_FLOW_ACTION_TYPE_END,
        }    
    };   

	struct rte_flow_error error;

    if (rte_flow_create(port_id, &flow_attr_tunnel, flow_item_tunnel, flow_action_tunnel, &error) == NULL) {
        VLOG_ERR("flow create fail:%d---> %s\n", error.type, error.message);
		return -1;
    }

    VLOG_INFO("flow create mac correct 0x%x\n", mac.addr_bytes[5]);
	
	return 0;
}

static int
dpdk_ip_rss_set(uint8_t port_id, uint8_t type) 
{
    struct rte_eth_fdir_filter_info info; 
    int ret = 0;

    memset(&info, 0, sizeof(info));
    info.info_type = RTE_ETH_HASH_FILTER_INPUT_SET_SELECT;
    info.info.input_set_conf.flow_type = type;
    info.info.input_set_conf.field[0] = RTE_ETH_INPUT_SET_NONE;
    info.info.input_set_conf.inset_size = 1;
    info.info.input_set_conf.op = RTE_ETH_INPUT_SET_SELECT;
    ret = rte_eth_dev_filter_ctrl(port_id, RTE_ETH_FILTER_HASH,
            RTE_ETH_FILTER_SET, &info);
    if (ret < 0) {
        VLOG_INFO("filter hash clear %d fail %u.\n", type, port_id);
        return ret;
    }

    memset(&info, 0, sizeof(info));
    info.info_type = RTE_ETH_HASH_FILTER_INPUT_SET_SELECT;
    info.info.input_set_conf.flow_type = type;
    info.info.input_set_conf.field[0] = RTE_ETH_INPUT_SET_L3_DST_IP4;
    info.info.input_set_conf.inset_size = 1;
    info.info.input_set_conf.op = RTE_ETH_INPUT_SET_ADD;
    ret = rte_eth_dev_filter_ctrl(port_id, RTE_ETH_FILTER_HASH,
            RTE_ETH_FILTER_SET, &info);
    if (ret < 0) {
        VLOG_INFO("filter hash input_set add %d fail %u.\n", type, port_id);
        return ret;
    }

    return ret;
}

static bool 
dpdk_rss_nwdst_set(uint8_t port_id)
{
    int ret = 0;

    ret = rte_eth_dev_filter_supported(port_id, RTE_ETH_FILTER_HASH);
    if (ret < 0) {
        VLOG_INFO("filter hash is not supported on port %u.\n", port_id);
        return false;
    }
	
    ret = dpdk_ip_rss_set(port_id, RTE_ETH_FLOW_FRAG_IPV4); 
    if (ret < 0) {
        return false;
    }

    ret = dpdk_ip_rss_set(port_id, RTE_ETH_FLOW_NONFRAG_IPV4_TCP); 
    if (ret < 0) {
        return false;
    }

    ret = dpdk_ip_rss_set(port_id, RTE_ETH_FLOW_NONFRAG_IPV4_UDP); 
    if (ret < 0) {
        return false;
    }

    ret = dpdk_ip_rss_set(port_id, RTE_ETH_FLOW_NONFRAG_IPV4_SCTP); 
    if (ret < 0) {
        return false;
    }

    ret = dpdk_ip_rss_set(port_id, RTE_ETH_FLOW_NONFRAG_IPV4_OTHER); 
    if (ret < 0) {
        return false;
    }

    return true;
}

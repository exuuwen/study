merged_flow->merged_match.mask.ct_zone = 0;  why

1.
dpcls_flow ---> flow_put---ufid_msg_queue---> e2e_cache_ovs_flow->node insert on flow_map

struct e2e_cache_match {
    struct flow flow; 
    struct flow_wildcards wc;
};

/*
 * A mapping from ufid to flow for e2e cache.
 */
struct e2e_cache_ovs_flow {
    struct hmap_node node; 
    ovs_u128 ufid; 
    unsigned int merge_tid;
    struct nlattr *actions;
    enum e2e_offload_state offload_state;
    uint16_t actions_size;
    struct ovs_list associated_merged_flows;
    struct e2e_cache_match match[0];
};

output action ---> merge_flows---trace_msg_queue---->e2e_cache_merged_flow

ovs_list_push_back(&flows[j]->associated_merged_flows, &merged_flow->associated_flows[i].list);

merged_flow.node.in_hmap insert to merged_flows_map


struct flow2flow_item {
    struct ovs_list list;
    struct e2e_cache_ovs_flow *mt_flow;
    uint16_t index;
};

/*
 * Merged flow structure.           
 */
struct e2e_cache_merged_flow {
    union {
        struct hmap_node in_hmap;
        struct ovs_list  in_list;
    } node;
    ovs_u128 ufid;
    struct dp_netdev *dp;
    struct nlattr *actions;
    uint16_t actions_size;
    uint16_t associated_flows_len;
   
    struct merged_match merged_match;
    struct flow2flow_item associated_flows[0];
}; 


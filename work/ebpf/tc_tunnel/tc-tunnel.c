#include <linux/bpf.h>
#include <linux/pkt_cls.h>
#include <iproute2/bpf_elf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/types.h>

#include "bpf_helpers.h"
#include "maps.h"

#ifndef __section
# define __section(NAME)                  \
   __attribute__((section(NAME), used))
#endif

#ifndef __inline
# define __inline                         \
   inline __attribute__((always_inline))
#endif

#ifndef BPF_FUNC
# define BPF_FUNC(NAME, ...)              \
   (*NAME)(__VA_ARGS__) = (void *)BPF_FUNC_##NAME
#endif

__section("ingress_gre")
int tc_ingress_gre(struct __sk_buff *skb)
{
    struct bpf_tunnel_key key;
    struct tunnel_key tkey;
    struct tunnel_info *info;

    void *data = (void *)(long)skb->data;
    struct ethhdr *eth = data;
    void *data_end = (void *)(long)skb->data_end;

    if (data + sizeof(*eth) > data_end)
        return TC_ACT_OK;

    if (eth->h_proto == __builtin_bswap16(ETH_P_IP)) {
        struct iphdr *iph = data + sizeof(*eth);

        if (data + sizeof(*eth) + sizeof(*iph) > data_end)
            return TC_ACT_OK;

        __builtin_memset(&tkey, 0x0, sizeof(tkey));
        bpf_skb_get_tunnel_key(skb, &key, sizeof(key), 0);  

        tkey.vni = key.tunnel_id;
        tkey.ip_dst = __builtin_bswap32(iph->daddr);

        info = bpf_map_lookup_elem(&tunnel_map, &tkey);
        if (info)
            return bpf_redirect(info->ifindex, 0);
    }

    return TC_ACT_OK;
}

__section("ingress_veth")
int tc_ingress_veth(struct __sk_buff *skb)
{
    struct bpf_tunnel_key key;
    struct tunnel_key tkey;
    struct tunnel_info *info;

    __builtin_memset(&key, 0x0, sizeof(key));
    __builtin_memset(&tkey, 0x0, sizeof(tkey));

    tkey.ifindex = skb->ifindex;

    void *data = (void *)(long)skb->data;
    struct ethhdr *eth = data;
    void *data_end = (void *)(long)skb->data_end;

    if (data + sizeof(*eth) > data_end)
        return TC_ACT_OK;

    if (eth->h_proto == __builtin_bswap16(ETH_P_IP)) {
        struct iphdr *iph = data + sizeof(*eth);

        if (data + sizeof(*eth) + sizeof(*iph) > data_end)
            return TC_ACT_OK;

        __builtin_memset(&tkey, 0x0, sizeof(tkey));

        tkey.ifindex = skb->ifindex;
        tkey.ip_dst = __builtin_bswap32(iph->daddr);

        info = bpf_map_lookup_elem(&tunnel_map, &tkey);

        if (info) {
            key.tunnel_id = info->vni;
            key.remote_ipv4 = info->tun_dst;

            bpf_skb_set_tunnel_key(skb, &key, sizeof(key), BPF_F_ZERO_CSUM_TX);  
            return bpf_redirect(info->ifindex, 0);
        }
    }
    
    return TC_ACT_OK;
}

char __license[] __section("license") = "GPL";

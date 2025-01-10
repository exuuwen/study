#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/types.h>
#include <bpf/bpf_helpers.h>

#include <stdint.h>

#ifndef __section
# define __section(NAME)                  \
   __attribute__((section(NAME), used))
#endif

#ifndef __inline
# define __inline                         \
   inline __attribute__((always_inline))
#endif

#ifndef memset
# define memset(dest, chr, n)   __builtin_memset((dest), (chr), (n))
#endif

#ifndef memcpy
# define memcpy(dest, src, n)   __builtin_memcpy((dest), (src), (n))
#endif

#define IP_MF       0x2000      /* Flag: "More Fragments"   */
#define IP_OFFSET   0x1FFF      /* "Fragment Offset" part   */

#define GRETAP_FLAGS 0x2000

//clang -O2 -Wall -g -D__x86_64__ -target bpf -c xdp-tunnel.c -o xdp-tunnel.o 
//ip link set dev vnet0/veth0 xdp obj xdp-tunnel.o sec progvirt
//ip link set dev eth2/br0 xdp obj xdp-tunnel.o sec procnic
//ip link set dev vnet0/br0/eth2/veth0 xdp off

struct grehdr {
    unsigned short flags;  
    unsigned short proto;
    unsigned int key;
};

static __always_inline int handle_ipv4(struct xdp_md *xdp)
{
	void *data_end = (void *)(long)xdp->data_end;
	void *data = (void *)(long)xdp->data;
	struct ethhdr *new_eth;
	struct iphdr *iph = data + sizeof(struct ethhdr);
        struct grehdr *greh;
	unsigned short *next_iph_u16;
	unsigned short payload_len;
	unsigned int csum = 0;
	int i;
	unsigned char src_mac[6] = {0x3c, 0xfd, 0xfe, 0xad, 0xfc, 0x8d};
	unsigned char dst_mac[6] = {0x68, 0x05, 0xca, 0x34, 0x8f, 0x20};

	if ((void*)(iph + 1) > data_end)
		return XDP_DROP;
	payload_len = __builtin_bswap16(iph->tot_len);

	if (bpf_xdp_adjust_head(xdp, 0 - (int)(sizeof(struct ethhdr) +  sizeof(struct iphdr) + sizeof(struct grehdr))))
		return XDP_DROP;

	data = (void *)(long)xdp->data;
	data_end = (void *)(long)xdp->data_end;

	new_eth = data;
	iph = data + sizeof(*new_eth);
	greh = data + sizeof(*new_eth) + sizeof(*iph);

	if ((void*)(new_eth + 1) > data_end ||
	    (void*)(iph + 1) > data_end ||
	    (void*)(greh + 1) > data_end)
		return XDP_DROP;

	memcpy(new_eth->h_source, src_mac, sizeof(new_eth->h_source));
	memcpy(new_eth->h_dest, dst_mac, sizeof(new_eth->h_dest));
	new_eth->h_proto = __builtin_bswap16(ETH_P_IP);

	iph->version = 4;
	iph->ihl = sizeof(*iph) >> 2;
	iph->frag_off =	0;
	iph->protocol = IPPROTO_GRE;
	iph->check = 0;
	iph->tos = 0;
	iph->tot_len = __builtin_bswap16(payload_len + sizeof(*iph) + sizeof(*greh) + sizeof(*new_eth));
	iph->daddr = __builtin_bswap32(0xaca801d0); //172.168.1.208
	iph->saddr = __builtin_bswap32(0xaca801f1); //172.168.1.241
	iph->ttl = 64;

	next_iph_u16 = (unsigned short *)iph;
#pragma clang loop unroll(full)
	for (i = 0; i < sizeof(*iph) >> 1; i++)
		csum += *next_iph_u16++;

	iph->check = ~((csum & 0xffff) + (csum >> 16));

	greh->flags = __builtin_bswap16(GRETAP_FLAGS);
	greh->proto = __builtin_bswap16(ETH_P_TEB);
	greh->key = __builtin_bswap32(1000);

	return bpf_redirect(6, 0);//eth2 or br0
}

__section("progvirt")
int xdp_vnet(struct xdp_md *xdp)
{
	void *data = (void *)(long)xdp->data;
	struct ethhdr *eth = data;
	void *data_end = (void *)(long)xdp->data_end;
	__u16 h_proto;

	if ((void*)(eth + 1) > data_end)
		return XDP_DROP;

	h_proto = eth->h_proto;

	if (h_proto == __builtin_bswap16(ETH_P_IP))
		return handle_ipv4(xdp);
	else
		return XDP_PASS;

}

static __always_inline int handle_gretap(struct xdp_md *xdp, uint32_t key)
{
	struct ethhdr *inner_eth;
	struct iphdr *inner_iph;
	void *data = (void *)(long)xdp->data;
	void *data_end = (void *)(long)xdp->data_end;

	inner_eth = data;
	inner_iph = data + sizeof(*inner_eth);

	if ((void*)(inner_eth + 1) > data_end ||
	    (void*)(inner_iph + 1) > data_end)
		return XDP_DROP;

	if (key == __builtin_bswap32(1000))
		return bpf_redirect(19, 0); //v-nic
	else
		return XDP_DROP;
}

static __always_inline int ip_is_fragment(const struct iphdr *iph)
{
    return (iph->frag_off & __builtin_bswap16(IP_MF | IP_OFFSET)) != 0;
}

__section("prognic")
int xdp_nic(struct xdp_md *xdp)
{
	void *data = (void *)(long)xdp->data;
	void *data_end = (void *)(long)xdp->data_end;

	struct ethhdr *eth = data;
	__u16 h_proto;

	if ((void*)(eth + 1) > data_end)
		return XDP_DROP;

	h_proto = eth->h_proto;

	if (h_proto == __builtin_bswap16(ETH_P_IP)) {
		struct iphdr *iph = data + sizeof(struct ethhdr);
		if ((void*)(iph + 1) > data_end)
			return XDP_DROP;

		if (iph->protocol == IPPROTO_GRE && !(ip_is_fragment(iph))) {
			struct grehdr *greh = data + sizeof(struct ethhdr) + sizeof(struct iphdr);	
			uint32_t key = greh->key;
			if ((void*)(greh + 1) > data_end)
				return XDP_DROP;
			if (greh->flags == __builtin_bswap16(GRETAP_FLAGS) && greh->proto == __builtin_bswap16(ETH_P_TEB)) {
				if (bpf_xdp_adjust_head(xdp, 0 + (int)(sizeof(struct ethhdr) +  sizeof(struct iphdr) + sizeof(struct grehdr))))
					return XDP_DROP;
				return handle_gretap(xdp, key); //with key
			} 
		}
	}

	return XDP_PASS;
}


char __license[] __section("license") = "GPL";

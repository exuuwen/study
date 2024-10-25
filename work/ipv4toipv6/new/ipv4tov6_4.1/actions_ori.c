/*
 * Copyright (c) 2007-2015 Nicira, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/openvswitch.h>
#include <linux/netfilter_ipv6.h>
#include <linux/sctp.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/icmpv6.h>
#include <linux/in6.h>
#include <linux/if_arp.h>
#include <linux/if_vlan.h>

#include <net/dst.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/checksum.h>
#include <net/dsfield.h>
#include <net/mpls.h>
#include <net/sctp/checksum.h>

#include "datapath.h"
#include "conntrack.h"
#include "gso.h"
#include "vport.h"

extern uint32_t ovs_ipv4_to_ipv6;

static int do_execute_actions(struct datapath *dp, struct sk_buff *skb,
			      struct sw_flow_key *key,
			      const struct nlattr *attr, int len);

struct deferred_action {
	struct sk_buff *skb;
	const struct nlattr *actions;

	/* Store pkt_key clone when creating deferred action. */
	struct sw_flow_key pkt_key;
};

#define MAX_L2_LEN	(VLAN_ETH_HLEN + 3 * MPLS_HLEN)
struct ovs_frag_data {
	unsigned long dst;
	struct vport *vport;
	struct ovs_gso_cb cb;
	__be16 inner_protocol;
	__u16 vlan_tci;
	__be16 vlan_proto;
	unsigned int l2_len;
	u8 l2_data[MAX_L2_LEN];
};

static DEFINE_PER_CPU(struct ovs_frag_data, ovs_frag_data_storage);

#define DEFERRED_ACTION_FIFO_SIZE 10
#define OVS_RECURSION_LIMIT 4
#define OVS_DEFERRED_ACTION_THRESHOLD (OVS_RECURSION_LIMIT - 2)
struct action_fifo {
	int head;
	int tail;
	/* Deferred action fifo queue storage. */
	struct deferred_action fifo[DEFERRED_ACTION_FIFO_SIZE];
};

struct recirc_keys {
	struct sw_flow_key key[OVS_DEFERRED_ACTION_THRESHOLD];
};

static struct action_fifo __percpu *action_fifos;
static struct recirc_keys __percpu *recirc_keys;
static DEFINE_PER_CPU(int, exec_actions_level);

static void action_fifo_init(struct action_fifo *fifo)
{
	fifo->head = 0;
	fifo->tail = 0;
}

static bool action_fifo_is_empty(const struct action_fifo *fifo)
{
	return (fifo->head == fifo->tail);
}

static struct deferred_action *action_fifo_get(struct action_fifo *fifo)
{
	if (action_fifo_is_empty(fifo))
		return NULL;

	return &fifo->fifo[fifo->tail++];
}

static struct deferred_action *action_fifo_put(struct action_fifo *fifo)
{
	if (fifo->head >= DEFERRED_ACTION_FIFO_SIZE - 1)
		return NULL;

	return &fifo->fifo[fifo->head++];
}

/* Return queue entry if fifo is not full */
static struct deferred_action *add_deferred_actions(struct sk_buff *skb,
						    const struct sw_flow_key *key,
						    const struct nlattr *attr)
{
	struct action_fifo *fifo;
	struct deferred_action *da;

	fifo = this_cpu_ptr(action_fifos);
	da = action_fifo_put(fifo);
	if (da) {
		da->skb = skb;
		da->actions = attr;
		da->pkt_key = *key;
	}

	return da;
}

static void invalidate_flow_key(struct sw_flow_key *key)
{
	key->eth.type = htons(0);
}

static bool is_flow_key_valid(const struct sw_flow_key *key)
{
	return !!key->eth.type;
}

static void update_ethertype(struct sk_buff *skb, struct ethhdr *hdr,
			     __be16 ethertype)
{
	if (skb->ip_summed == CHECKSUM_COMPLETE) {
		__be16 diff[] = { ~(hdr->h_proto), ethertype };

		skb->csum = ~csum_partial((char *)diff, sizeof(diff),
					~skb->csum);
	}

	hdr->h_proto = ethertype;
}

static int push_mpls(struct sk_buff *skb, struct sw_flow_key *key,
		     const struct ovs_action_push_mpls *mpls)
{
	__be32 *new_mpls_lse;

	/* Networking stack do not allow simultaneous Tunnel and MPLS GSO. */
	if (skb->encapsulation)
		return -ENOTSUPP;

	if (skb_cow_head(skb, MPLS_HLEN) < 0)
		return -ENOMEM;

	skb_push(skb, MPLS_HLEN);
	memmove(skb_mac_header(skb) - MPLS_HLEN, skb_mac_header(skb),
		skb->mac_len);
	skb_reset_mac_header(skb);

	new_mpls_lse = (__be32 *)skb_mpls_header(skb);
	*new_mpls_lse = mpls->mpls_lse;

	skb_postpush_rcsum(skb, new_mpls_lse, MPLS_HLEN);

	update_ethertype(skb, eth_hdr(skb), mpls->mpls_ethertype);
	if (!ovs_skb_get_inner_protocol(skb))
		ovs_skb_set_inner_protocol(skb, skb->protocol);
	skb->protocol = mpls->mpls_ethertype;

	invalidate_flow_key(key);
	return 0;
}

static int pop_mpls(struct sk_buff *skb, struct sw_flow_key *key,
		    const __be16 ethertype)
{
	struct ethhdr *hdr;
	int err;

	err = skb_ensure_writable(skb, skb->mac_len + MPLS_HLEN);
	if (unlikely(err))
		return err;

	skb_postpull_rcsum(skb, skb_mpls_header(skb), MPLS_HLEN);

	memmove(skb_mac_header(skb) + MPLS_HLEN, skb_mac_header(skb),
		skb->mac_len);

	__skb_pull(skb, MPLS_HLEN);
	skb_reset_mac_header(skb);

	/* skb_mpls_header() is used to locate the ethertype
	 * field correctly in the presence of VLAN tags.
	 */
	hdr = (struct ethhdr *)(skb_mpls_header(skb) - ETH_HLEN);
	update_ethertype(skb, hdr, ethertype);
	if (eth_p_mpls(skb->protocol))
		skb->protocol = ethertype;

	invalidate_flow_key(key);
	return 0;
}

static int set_mpls(struct sk_buff *skb, struct sw_flow_key *flow_key,
		    const __be32 *mpls_lse, const __be32 *mask)
{
	__be32 *stack;
	__be32 lse;
	int err;

	err = skb_ensure_writable(skb, skb->mac_len + MPLS_HLEN);
	if (unlikely(err))
		return err;

	stack = (__be32 *)skb_mpls_header(skb);
	lse = OVS_MASKED(*stack, *mpls_lse, *mask);
	if (skb->ip_summed == CHECKSUM_COMPLETE) {
		__be32 diff[] = { ~(*stack), lse };

		skb->csum = ~csum_partial((char *)diff, sizeof(diff),
					  ~skb->csum);
	}

	*stack = lse;
	flow_key->mpls.top_lse = lse;
	return 0;
}

static int pop_vlan(struct sk_buff *skb, struct sw_flow_key *key)
{
	int err;

	err = skb_vlan_pop(skb);
	if (skb_vlan_tag_present(skb))
		invalidate_flow_key(key);
	else
		key->eth.tci = 0;
	return err;
}

static int push_vlan(struct sk_buff *skb, struct sw_flow_key *key,
		     const struct ovs_action_push_vlan *vlan)
{
	if (skb_vlan_tag_present(skb))
		invalidate_flow_key(key);
	else
		key->eth.tci = vlan->vlan_tci;
	return skb_vlan_push(skb, vlan->vlan_tpid,
			     ntohs(vlan->vlan_tci) & ~VLAN_TAG_PRESENT);
}

static void ip_select_fb_ident(struct iphdr *iph)
{
	static DEFINE_SPINLOCK(ip_fb_id_lock);
	static u64 ip_fallback_id = 120;
	u64 salt;

	spin_lock_bh(&ip_fb_id_lock);
	salt = ip_fallback_id + 1;
	iph->id = htons(salt & 0xFFFF);
	ip_fallback_id = salt;
	spin_unlock_bh(&ip_fb_id_lock);
}

static inline __wsum check_diff_v4_to_v6(const __be32 * new_src, const __be32 * new_dst, 
					const __be32 old_src, const __be32 old_dst,
					__wsum oldsum)
{
	__be32 diff[10] = { ~old_src, ~old_dst, new_src[3], new_src[2], 
			new_src[1], new_src[0], new_dst[3], new_dst[2], 
			new_dst[1], new_dst[0]
	};

	return csum_partial(diff, sizeof(diff), oldsum);
}

static inline __wsum check_diff_v6_to_v4(const __be32 new_src, const __be32 new_dst, 
					const __be32 *old_src, const __be32 *old_dst,
					__wsum oldsum)
{
	__be32 diff[10] = { ~old_src[3], ~old_src[2], ~old_src[1], ~old_src[0], 
			~old_dst[3], ~old_dst[2], ~old_dst[1], ~old_dst[0],
			new_src, new_dst
	};

	return csum_partial(diff, sizeof(diff), oldsum);
}

/* Returns the least-significant 32 bits of a __be64. */
static __be32 be64_get_low32(__be64 x)
{
#ifdef __BIG_ENDIAN
	return (__force __be32)x;
#else
	return (__force __be32)((__force u64)x >> 32);
#endif
}

static inline void ipv6_addr_copy(struct in6_addr *a1, const struct in6_addr *a2)
{
	memcpy(a1, a2, sizeof(struct in6_addr));
}

static int new_sample(struct sk_buff *skb, struct sw_flow_key *key)
{
	uint8_t type;
	struct ethhdr *eth;
	struct ethhdr old_eth;

	eth = eth_hdr(skb);
	if (eth->h_proto == ntohs(ETH_P_IP))
		type = 1;
	else if (eth->h_proto == ntohs(ETH_P_IPV6))
		type = 2;
	else
		return 0;

	memcpy(&old_eth, eth, sizeof(struct ethhdr));

	if (type == 1)
	{
		struct ethhdr *new_eth; 
		struct ipv6hdr *hdr;
		struct iphdr *nh;

		uint8_t ttl;
		uint8_t proto;
		uint32_t srcip_be;
		uint32_t dstip_be;

		int trans_hlen = 0;
		int err;
		int min_headroom;
		uint8_t is_frag = 0;
		uint8_t has_trans_head = 1;
		uint16_t frag_off = 0;
		uint16_t id_be = 0;
		uint8_t has_more = 0;
		uint32_t network_hlen = sizeof(struct ipv6hdr);

		uint16_t payload_len;

		struct in6_addr saddr, daddr;

		uint32_t key_be;

		key_be = be64_get_low32(key->tun_key.tun_id);
		if (key_be == 0)
			return 0;

		nh = ip_hdr(skb);
		ttl = nh->ttl;
		proto = nh->protocol;
		srcip_be = nh->saddr;
		dstip_be = nh->daddr;

		if (ip_is_fragment(nh))
		{
			is_frag = 1;
			frag_off = (ntohs(nh->frag_off) & IP_OFFSET) << 3;
			if (frag_off)
				has_trans_head = 0;
			if (nh->frag_off & htons(IP_MF))
				has_more = 1;
			id_be = nh->id;
			network_hlen += sizeof(struct frag_hdr);
		}
		
		if (proto == IPPROTO_TCP)
		{
			if (has_trans_head)
				trans_hlen = sizeof(struct tcphdr);
		}
		else if (proto == IPPROTO_UDP)
		{
			if (has_trans_head)
				trans_hlen = sizeof(struct udphdr);
		}
		else if (proto == IPPROTO_ICMP)
		{
			if (is_frag)
				return -1;
			trans_hlen = sizeof(struct icmphdr);
		}
		else 
			return 0;

		err = skb_ensure_writable(skb, skb_transport_offset(skb) + trans_hlen);
		if (unlikely(err))
			return 0;

		if (!pskb_may_pull(skb, (skb_transport_offset(skb) + trans_hlen)))
			return 0;

		if (proto == IPPROTO_ICMP)
		{
			struct icmphdr *icmph;
			icmph = icmp_hdr(skb);

			if (icmph->type == ICMP_ECHO)
				icmph->type = ICMPV6_ECHO_REQUEST;
			else if (icmph->type == ICMP_ECHOREPLY)
				icmph->type = ICMPV6_ECHO_REPLY;
			else
				return 0;

			proto = IPPROTO_ICMPV6;
		}

		__skb_pull(skb, skb_transport_offset(skb));
		payload_len = skb->len;

		min_headroom = network_hlen + (skb->dev ? LL_RESERVED_SPACE(skb->dev) : sizeof(struct ethhdr));
		if (skb_headroom(skb) < min_headroom || skb_header_cloned(skb)) 
		{
			int head_delta = SKB_DATA_ALIGN(min_headroom -
						skb_headroom(skb) +
						16);
			err = pskb_expand_head(skb, max_t(int, head_delta, 0),
					0, GFP_ATOMIC);
			if (unlikely(err))
				return 0;
		}

		__skb_push(skb, network_hlen);
		skb_reset_network_header(skb);
		hdr = ipv6_hdr(skb);

		if (is_frag)
		{
			struct frag_hdr *fh = (struct frag_hdr*)(skb_network_header(skb) + sizeof(struct ipv6hdr));	
			fh->nexthdr = proto;
			fh->reserved = 0;
			fh->frag_off = htons(frag_off);
			if (has_more)
				fh->frag_off |= htons(IP6_MF);
			fh->identification = id_be | (key_be & 0xff00); 
			hdr->nexthdr = IPPROTO_FRAGMENT;
		}
		else
			hdr->nexthdr = proto;

		*(__be32 *)hdr = htonl(0x60000000);
		hdr->payload_len = htons(payload_len + (is_frag ? sizeof(struct frag_hdr) : 0));
		hdr->hop_limit = ttl;
		
		saddr.s6_addr32[3] = key_be;
		saddr.s6_addr32[2] = srcip_be;
		saddr.s6_addr16[3] = htons(0x1000);
		saddr.s6_addr16[2] = htons(0x2004);
		saddr.s6_addr16[1] = htons(0xda8);
		saddr.s6_addr16[0] = htons(0x2003);

		daddr.s6_addr32[3] = dstip_be;
		daddr.s6_addr16[5] = htons(0x116);
		daddr.s6_addr16[4] = htons(0x202);
		daddr.s6_addr16[3] = htons(0x1000);
		daddr.s6_addr16[2] = htons(0x2004);
		daddr.s6_addr16[1] = htons(0xda8);
		daddr.s6_addr16[0] = htons(0x2001);
		
		ipv6_addr_copy(&hdr->saddr, &saddr);
		ipv6_addr_copy(&hdr->daddr, &daddr);
		skb->protocol = htons(ETH_P_IPV6);

		if (proto == IPPROTO_TCP && has_trans_head)
		{
			struct tcphdr *tcph;
			tcph = tcp_hdr(skb);
			
			//impossible case
			if (skb->ip_summed == CHECKSUM_PARTIAL)
			{
				//impossible frag
  				tcph->check = 0;
				tcph->check = ~csum_ipv6_magic(&saddr, &daddr,
                                                      skb->len - sizeof(struct ipv6hdr),
                                                      IPPROTO_TCP, 0);
                		skb->csum_start = skb_transport_header(skb) - skb->head;
                		skb->csum_offset = offsetof(struct tcphdr, check);
			}
			else
			{
				tcph->check = csum_fold(check_diff_v4_to_v6(saddr.in6_u.u6_addr32, daddr.in6_u.u6_addr32, srcip_be, dstip_be, ~csum_unfold(tcph->check)));
				skb->ip_summed = CHECKSUM_NONE;
			}
		}
		else if (proto == IPPROTO_UDP && has_trans_head)
		{
			struct udphdr *udph;
			udph = udp_hdr(skb);
			
			if (skb->ip_summed == CHECKSUM_PARTIAL)
			{
				//impossible frag
  				udph->check = 0;
				udph->check = ~csum_ipv6_magic(&saddr, &daddr,
                                                      skb->len - sizeof(struct ipv6hdr),
                                                      IPPROTO_UDP, 0);
                		skb->csum_start = skb_transport_header(skb) - skb->head;
                		skb->csum_offset = offsetof(struct udphdr, check);
			}
			else
			{
				udph->check = csum_fold(check_diff_v4_to_v6(saddr.in6_u.u6_addr32, daddr.in6_u.u6_addr32, srcip_be, dstip_be, ~csum_unfold(udph->check)));
				skb->ip_summed = CHECKSUM_NONE;
			}
		}
		else if (proto == IPPROTO_ICMPV6)
		{
			struct icmp6hdr *icmph;
			icmph = icmp6_hdr(skb);

			icmph->icmp6_cksum = 0;
			skb->csum = skb_checksum(skb, sizeof(struct ipv6hdr), skb->len - sizeof(struct ipv6hdr), 0);
			icmph->icmp6_cksum = csum_ipv6_magic(&saddr, &daddr,
                                                      skb->len - sizeof(struct ipv6hdr),
                                                      IPPROTO_ICMPV6, skb->csum);

			skb->ip_summed = CHECKSUM_NONE;
		}

		__skb_push(skb, sizeof(struct ethhdr));
		skb_reset_mac_header(skb);

		new_eth = eth_hdr(skb);
		memcpy(new_eth, &old_eth, sizeof(struct ethhdr));
		new_eth->h_proto = htons(ETH_P_IPV6);
	}
	else if (type == 2)
	{
		struct ethhdr *new_eth; 
		struct ipv6hdr *hdr; 
		struct iphdr *iph; 
		
		struct in6_addr saddr, daddr;

		uint8_t proto;
		uint8_t ttl;
		uint32_t dstip_be;
		uint32_t srcip_be;

		int trans_hlen = 0;
		int err;
		uint32_t key_be;

		uint8_t is_frag = 0;
		uint8_t has_more = 0;
		uint8_t has_trans_head = 1;
		uint16_t frag_off = 0;
		uint16_t id_be = 0;

		hdr = ipv6_hdr(skb);
		proto = hdr->nexthdr;
		ttl = hdr->hop_limit;
		ipv6_addr_copy(&saddr, &hdr->saddr);
		ipv6_addr_copy(&daddr, &hdr->daddr);

		dstip_be = daddr.s6_addr32[2];
		srcip_be = saddr.s6_addr32[3];

		key_be = daddr.s6_addr32[3];
		skb->mark = ntohl(key_be);

		if (proto == IPPROTO_FRAGMENT)
		{
			struct frag_hdr *fh = (struct frag_hdr*)(skb_network_header(skb) + sizeof(struct ipv6hdr));			
			is_frag = 1;
			if (fh->frag_off & htons(IP6_MF))
				has_more = 1;
			frag_off = ntohs(fh->frag_off) & ~0x7;
			if (frag_off)
				has_trans_head = 0;
			id_be = fh->identification & 0xff;

			proto = fh->nexthdr;
		}

		if (proto == IPPROTO_TCP)
		{
			if (has_trans_head)
				trans_hlen = sizeof(struct tcphdr);
			else
				trans_hlen = sizeof(struct frag_hdr);
		}
		else if (proto == IPPROTO_UDP)
		{
			if (has_trans_head)
				trans_hlen = sizeof(struct udphdr);
			else
				trans_hlen = sizeof(struct frag_hdr);
		}
		else if (proto == IPPROTO_ICMPV6)
		{
			if (is_frag)
				return -1;
			trans_hlen = sizeof(struct icmp6hdr);
		}
		else
			return 0;

		err = skb_ensure_writable(skb, skb_transport_offset(skb) + trans_hlen);
		if (unlikely(err))
			return 0;

		if (!pskb_may_pull(skb, (skb_transport_offset(skb) + trans_hlen)))
			return 0;

		if (proto == IPPROTO_ICMPV6)
		{
			struct icmp6hdr *icmph;
			icmph = icmp6_hdr(skb);

			if (icmph->icmp6_type == ICMPV6_ECHO_REQUEST)
				icmph->icmp6_type = ICMP_ECHO;
			else if (icmph->icmp6_type == ICMPV6_ECHO_REPLY)
				icmph->icmp6_type = ICMP_ECHOREPLY;
			else
				return 0;

			proto = IPPROTO_ICMP;
		}

		__skb_pull(skb, skb_transport_offset(skb) + ((!has_trans_head) ? sizeof(struct frag_hdr) : 0));

		__skb_push(skb, sizeof(struct iphdr));
		skb_reset_network_header(skb);

		iph = ip_hdr(skb);
		iph->version	=	4;
		iph->ihl	=	sizeof(struct iphdr) >> 2;
		iph->frag_off	=	htons((frag_off >> 3));
		if (has_more)
			iph->frag_off |= htons(IP_MF);
		iph->protocol	=	proto;
		iph->tos	=	0;
		iph->daddr	=	dstip_be;
		iph->saddr	=	srcip_be;
		iph->ttl	=	ttl;
		if (id_be)
		{
			iph->id = id_be;	
		}
		else
			ip_select_fb_ident(iph);

		iph->tot_len = htons(skb->len);
		ip_send_check(iph);

		skb->protocol = htons(ETH_P_IP);

		if (proto == IPPROTO_TCP && has_trans_head)
		{
			struct tcphdr *tcph;
			//uint32_t tcph_len;
			tcph = tcp_hdr(skb);
			//tcph_len = tcph->doff * 4;

			if (skb->ip_summed == CHECKSUM_PARTIAL)
			{
				//unsigned int mss = skb_shinfo(skb)->gso_size;
				//impossible frag
				if (skb_shinfo(skb)->gso_type & SKB_GSO_TCPV6)
				{
					//printk("type %d, segs %d, mss %d\n", skb_shinfo(skb)->gso_type, skb_shinfo(skb)->gso_segs, mss);
					//just for 279 kernel
					skb_shinfo(skb)->gso_type &= ~SKB_GSO_TCPV6;
					skb_shinfo(skb)->gso_type |= SKB_GSO_TCPV4;
				}

  				tcph->check = 0;
				tcph->check = ~csum_tcpudp_magic(srcip_be, dstip_be,
                                                      skb->len - sizeof(struct iphdr),
                                                      IPPROTO_TCP, 0);
                		skb->csum_start = skb_transport_header(skb) - skb->head;
                		skb->csum_offset = offsetof(struct tcphdr, check);
			}
			else			
			{
				tcph->check = csum_fold(check_diff_v6_to_v4(srcip_be, dstip_be, saddr.in6_u.u6_addr32, daddr.in6_u.u6_addr32, ~csum_unfold(tcph->check)));
				skb->ip_summed = CHECKSUM_NONE;
			}
		}
		else if (proto == IPPROTO_UDP && has_trans_head)
		{
			struct udphdr *udph;
			udph = udp_hdr(skb);

			if (skb->ip_summed == CHECKSUM_PARTIAL)
			{
				//impossible frag
  				udph->check = 0;
				udph->check = ~csum_tcpudp_magic(srcip_be, dstip_be,
                                                      skb->len - sizeof(struct iphdr),
                                                      IPPROTO_UDP, 0);
                		skb->csum_start = skb_transport_header(skb) - skb->head;
                		skb->csum_offset = offsetof(struct udphdr, check);
			}
			else
			{
				
				udph->check = csum_fold(check_diff_v6_to_v4(srcip_be, dstip_be, saddr.in6_u.u6_addr32, daddr.in6_u.u6_addr32, ~csum_unfold(udph->check)));

				skb->ip_summed = CHECKSUM_NONE;
			}
		}
		else if (proto == IPPROTO_ICMP)
		{
			struct icmphdr *icmph;
			icmph = icmp_hdr(skb);

			icmph->checksum = 0;
			skb->csum = skb_checksum(skb, sizeof(struct iphdr), skb->len - sizeof(struct iphdr), 0);
			//icmpv4 checksum no psheader
			icmph->checksum = csum_fold(skb->csum);
                                                      
			skb->ip_summed = CHECKSUM_NONE;
		}

		__skb_push(skb, sizeof(struct ethhdr));
		skb_reset_mac_header(skb);

		new_eth = eth_hdr(skb);
		memcpy(new_eth, &old_eth, sizeof(struct ethhdr));
		new_eth->h_proto = htons(ETH_P_IP);
	}
		
	return 0;
}

/* 'src' is already properly masked. */
static void ether_addr_copy_masked(u8 *dst_, const u8 *src_, const u8 *mask_)
{
	u16 *dst = (u16 *)dst_;
	const u16 *src = (const u16 *)src_;
	const u16 *mask = (const u16 *)mask_;

	OVS_SET_MASKED(dst[0], src[0], mask[0]);
	OVS_SET_MASKED(dst[1], src[1], mask[1]);
	OVS_SET_MASKED(dst[2], src[2], mask[2]);
}

static int set_eth_addr(struct sk_buff *skb, struct sw_flow_key *flow_key,
			const struct ovs_key_ethernet *key,
			const struct ovs_key_ethernet *mask)
{
	int err;

	err = skb_ensure_writable(skb, ETH_HLEN);
	if (unlikely(err))
		return err;

	skb_postpull_rcsum(skb, eth_hdr(skb), ETH_ALEN * 2);

	ether_addr_copy_masked(eth_hdr(skb)->h_source, key->eth_src,
			       mask->eth_src);
	ether_addr_copy_masked(eth_hdr(skb)->h_dest, key->eth_dst,
			       mask->eth_dst);

	skb_postpush_rcsum(skb, eth_hdr(skb), ETH_ALEN * 2);

	ether_addr_copy(flow_key->eth.src, eth_hdr(skb)->h_source);
	ether_addr_copy(flow_key->eth.dst, eth_hdr(skb)->h_dest);
	return 0;
}

static void update_ip_l4_checksum(struct sk_buff *skb, struct iphdr *nh,
				  __be32 addr, __be32 new_addr)
{
	int transport_len = skb->len - skb_transport_offset(skb);

	if (nh->frag_off & htons(IP_OFFSET))
		return;

	if (nh->protocol == IPPROTO_TCP) {
		if (likely(transport_len >= sizeof(struct tcphdr)))
			inet_proto_csum_replace4(&tcp_hdr(skb)->check, skb,
						 addr, new_addr, true);
	} else if (nh->protocol == IPPROTO_UDP) {
		if (likely(transport_len >= sizeof(struct udphdr))) {
			struct udphdr *uh = udp_hdr(skb);

			if (uh->check || skb->ip_summed == CHECKSUM_PARTIAL) {
				inet_proto_csum_replace4(&uh->check, skb,
							 addr, new_addr, true);
				if (!uh->check)
					uh->check = CSUM_MANGLED_0;
			}
		}
	}

}

static void set_ip_addr(struct sk_buff *skb, struct iphdr *nh,
			__be32 *addr, __be32 new_addr)
{
	update_ip_l4_checksum(skb, nh, *addr, new_addr);
	csum_replace4(&nh->check, *addr, new_addr);
	skb_clear_hash(skb);
	*addr = new_addr;
}

static void update_ipv6_checksum(struct sk_buff *skb, u8 l4_proto,
				 __be32 addr[4], const __be32 new_addr[4])
{
	int transport_len = skb->len - skb_transport_offset(skb);

	if (l4_proto == NEXTHDR_TCP) {
		if (likely(transport_len >= sizeof(struct tcphdr)))
			inet_proto_csum_replace16(&tcp_hdr(skb)->check, skb,
						  addr, new_addr, true);
	} else if (l4_proto == NEXTHDR_UDP) {
		if (likely(transport_len >= sizeof(struct udphdr))) {
			struct udphdr *uh = udp_hdr(skb);

			if (uh->check || skb->ip_summed == CHECKSUM_PARTIAL) {
				inet_proto_csum_replace16(&uh->check, skb,
							  addr, new_addr, true);
				if (!uh->check)
					uh->check = CSUM_MANGLED_0;
			}
		}
	} else if (l4_proto == NEXTHDR_ICMP) {
		if (likely(transport_len >= sizeof(struct icmp6hdr)))
			inet_proto_csum_replace16(&icmp6_hdr(skb)->icmp6_cksum,
						  skb, addr, new_addr, true);
	}
}

static void mask_ipv6_addr(const __be32 old[4], const __be32 addr[4],
			   const __be32 mask[4], __be32 masked[4])
{
	masked[0] = OVS_MASKED(old[0], addr[0], mask[0]);
	masked[1] = OVS_MASKED(old[1], addr[1], mask[1]);
	masked[2] = OVS_MASKED(old[2], addr[2], mask[2]);
	masked[3] = OVS_MASKED(old[3], addr[3], mask[3]);
}

static void set_ipv6_addr(struct sk_buff *skb, u8 l4_proto,
			  __be32 addr[4], const __be32 new_addr[4],
			  bool recalculate_csum)
{
	if (likely(recalculate_csum))
		update_ipv6_checksum(skb, l4_proto, addr, new_addr);

	skb_clear_hash(skb);
	memcpy(addr, new_addr, sizeof(__be32[4]));
}

static void set_ipv6_fl(struct ipv6hdr *nh, u32 fl, u32 mask)
{
	/* Bits 21-24 are always unmasked, so this retains their values. */
	OVS_SET_MASKED(nh->flow_lbl[0], (u8)(fl >> 16), (u8)(mask >> 16));
	OVS_SET_MASKED(nh->flow_lbl[1], (u8)(fl >> 8), (u8)(mask >> 8));
	OVS_SET_MASKED(nh->flow_lbl[2], (u8)fl, (u8)mask);
}

static void set_ip_ttl(struct sk_buff *skb, struct iphdr *nh, u8 new_ttl,
		       u8 mask)
{
	new_ttl = OVS_MASKED(nh->ttl, new_ttl, mask);

	csum_replace2(&nh->check, htons(nh->ttl << 8), htons(new_ttl << 8));
	nh->ttl = new_ttl;
}

static int set_ipv4(struct sk_buff *skb, struct sw_flow_key *flow_key,
		    const struct ovs_key_ipv4 *key,
		    const struct ovs_key_ipv4 *mask)
{
	struct iphdr *nh;
	__be32 new_addr;
	int err;

	err = skb_ensure_writable(skb, skb_network_offset(skb) +
				  sizeof(struct iphdr));
	if (unlikely(err))
		return err;

	nh = ip_hdr(skb);

	/* Setting an IP addresses is typically only a side effect of
	 * matching on them in the current userspace implementation, so it
	 * makes sense to check if the value actually changed.
	 */
	if (mask->ipv4_src) {
		new_addr = OVS_MASKED(nh->saddr, key->ipv4_src, mask->ipv4_src);

		if (unlikely(new_addr != nh->saddr)) {
			set_ip_addr(skb, nh, &nh->saddr, new_addr);
			flow_key->ipv4.addr.src = new_addr;
		}
	}
	if (mask->ipv4_dst) {
		new_addr = OVS_MASKED(nh->daddr, key->ipv4_dst, mask->ipv4_dst);

		if (unlikely(new_addr != nh->daddr)) {
			set_ip_addr(skb, nh, &nh->daddr, new_addr);
			flow_key->ipv4.addr.dst = new_addr;
		}
	}
	if (mask->ipv4_tos) {
		ipv4_change_dsfield(nh, ~mask->ipv4_tos, key->ipv4_tos);
		flow_key->ip.tos = nh->tos;
	}
	if (mask->ipv4_ttl) {
		set_ip_ttl(skb, nh, key->ipv4_ttl, mask->ipv4_ttl);
		flow_key->ip.ttl = nh->ttl;
	}

	return 0;
}

static bool is_ipv6_mask_nonzero(const __be32 addr[4])
{
	return !!(addr[0] | addr[1] | addr[2] | addr[3]);
}

static int set_ipv6(struct sk_buff *skb, struct sw_flow_key *flow_key,
		    const struct ovs_key_ipv6 *key,
		    const struct ovs_key_ipv6 *mask)
{
	struct ipv6hdr *nh;
	int err;

	err = skb_ensure_writable(skb, skb_network_offset(skb) +
				  sizeof(struct ipv6hdr));
	if (unlikely(err))
		return err;

	nh = ipv6_hdr(skb);

	/* Setting an IP addresses is typically only a side effect of
	 * matching on them in the current userspace implementation, so it
	 * makes sense to check if the value actually changed.
	 */
	if (is_ipv6_mask_nonzero(mask->ipv6_src)) {
		__be32 *saddr = (__be32 *)&nh->saddr;
		__be32 masked[4];

		mask_ipv6_addr(saddr, key->ipv6_src, mask->ipv6_src, masked);

		if (unlikely(memcmp(saddr, masked, sizeof(masked)))) {
			set_ipv6_addr(skb, flow_key->ip.proto, saddr, masked,
				      true);
			memcpy(&flow_key->ipv6.addr.src, masked,
			       sizeof(flow_key->ipv6.addr.src));
		}
	}
	if (is_ipv6_mask_nonzero(mask->ipv6_dst)) {
		unsigned int offset = 0;
		int flags = IP6_FH_F_SKIP_RH;
		bool recalc_csum = true;
		__be32 *daddr = (__be32 *)&nh->daddr;
		__be32 masked[4];

		mask_ipv6_addr(daddr, key->ipv6_dst, mask->ipv6_dst, masked);

		if (unlikely(memcmp(daddr, masked, sizeof(masked)))) {
			if (ipv6_ext_hdr(nh->nexthdr))
				recalc_csum = (ipv6_find_hdr(skb, &offset,
							     NEXTHDR_ROUTING,
							     NULL, &flags)
					       != NEXTHDR_ROUTING);

			set_ipv6_addr(skb, flow_key->ip.proto, daddr, masked,
				      recalc_csum);
			memcpy(&flow_key->ipv6.addr.dst, masked,
			       sizeof(flow_key->ipv6.addr.dst));
		}
	}
	if (mask->ipv6_tclass) {
		ipv6_change_dsfield(nh, ~mask->ipv6_tclass, key->ipv6_tclass);
		flow_key->ip.tos = ipv6_get_dsfield(nh);
	}
	if (mask->ipv6_label) {
		set_ipv6_fl(nh, ntohl(key->ipv6_label),
			    ntohl(mask->ipv6_label));
		flow_key->ipv6.label =
		    *(__be32 *)nh & htonl(IPV6_FLOWINFO_FLOWLABEL);
	}
	if (mask->ipv6_hlimit) {
		OVS_SET_MASKED(nh->hop_limit, key->ipv6_hlimit,
			       mask->ipv6_hlimit);
		flow_key->ip.ttl = nh->hop_limit;
	}
	return 0;
}

/* Must follow skb_ensure_writable() since that can move the skb data. */
static void set_tp_port(struct sk_buff *skb, __be16 *port,
			__be16 new_port, __sum16 *check)
{
	inet_proto_csum_replace2(check, skb, *port, new_port, false);
	*port = new_port;
}

static int set_udp(struct sk_buff *skb, struct sw_flow_key *flow_key,
		   const struct ovs_key_udp *key,
		   const struct ovs_key_udp *mask)
{
	struct udphdr *uh;
	__be16 src, dst;
	int err;

	err = skb_ensure_writable(skb, skb_transport_offset(skb) +
				  sizeof(struct udphdr));
	if (unlikely(err))
		return err;

	uh = udp_hdr(skb);
	/* Either of the masks is non-zero, so do not bother checking them. */
	src = OVS_MASKED(uh->source, key->udp_src, mask->udp_src);
	dst = OVS_MASKED(uh->dest, key->udp_dst, mask->udp_dst);

	if (uh->check && skb->ip_summed != CHECKSUM_PARTIAL) {
		if (likely(src != uh->source)) {
			set_tp_port(skb, &uh->source, src, &uh->check);
			flow_key->tp.src = src;
		}
		if (likely(dst != uh->dest)) {
			set_tp_port(skb, &uh->dest, dst, &uh->check);
			flow_key->tp.dst = dst;
		}

		if (unlikely(!uh->check))
			uh->check = CSUM_MANGLED_0;
	} else {
		uh->source = src;
		uh->dest = dst;
		flow_key->tp.src = src;
		flow_key->tp.dst = dst;
	}

	skb_clear_hash(skb);

	return 0;
}

static int set_tcp(struct sk_buff *skb, struct sw_flow_key *flow_key,
		   const struct ovs_key_tcp *key,
		   const struct ovs_key_tcp *mask)
{
	struct tcphdr *th;
	__be16 src, dst;
	int err;

	err = skb_ensure_writable(skb, skb_transport_offset(skb) +
				  sizeof(struct tcphdr));
	if (unlikely(err))
		return err;

	th = tcp_hdr(skb);
	src = OVS_MASKED(th->source, key->tcp_src, mask->tcp_src);
	if (likely(src != th->source)) {
		set_tp_port(skb, &th->source, src, &th->check);
		flow_key->tp.src = src;
	}
	dst = OVS_MASKED(th->dest, key->tcp_dst, mask->tcp_dst);
	if (likely(dst != th->dest)) {
		set_tp_port(skb, &th->dest, dst, &th->check);
		flow_key->tp.dst = dst;
	}
	skb_clear_hash(skb);

	return 0;
}

static int set_sctp(struct sk_buff *skb, struct sw_flow_key *flow_key,
		    const struct ovs_key_sctp *key,
		    const struct ovs_key_sctp *mask)
{
	unsigned int sctphoff = skb_transport_offset(skb);
	struct sctphdr *sh;
	__le32 old_correct_csum, new_csum, old_csum;
	int err;

	err = skb_ensure_writable(skb, sctphoff + sizeof(struct sctphdr));
	if (unlikely(err))
		return err;

	sh = sctp_hdr(skb);
	old_csum = sh->checksum;
	old_correct_csum = sctp_compute_cksum(skb, sctphoff);

	sh->source = OVS_MASKED(sh->source, key->sctp_src, mask->sctp_src);
	sh->dest = OVS_MASKED(sh->dest, key->sctp_dst, mask->sctp_dst);

	new_csum = sctp_compute_cksum(skb, sctphoff);

	/* Carry any checksum errors through. */
	sh->checksum = old_csum ^ old_correct_csum ^ new_csum;

	skb_clear_hash(skb);
	flow_key->tp.src = sh->source;
	flow_key->tp.dst = sh->dest;

	return 0;
}

static int ovs_vport_output(OVS_VPORT_OUTPUT_PARAMS)
{
	struct ovs_frag_data *data = this_cpu_ptr(&ovs_frag_data_storage);
	struct vport *vport = data->vport;

	if (skb_cow_head(skb, data->l2_len) < 0) {
		kfree_skb(skb);
		return -ENOMEM;
	}

	__skb_dst_copy(skb, data->dst);
	*OVS_GSO_CB(skb) = data->cb;
	ovs_skb_set_inner_protocol(skb, data->inner_protocol);
	skb->vlan_tci = data->vlan_tci;
	skb->vlan_proto = data->vlan_proto;

	/* Reconstruct the MAC header.  */
	skb_push(skb, data->l2_len);
	memcpy(skb->data, &data->l2_data, data->l2_len);
	skb_postpush_rcsum(skb, skb->data, data->l2_len);
	skb_reset_mac_header(skb);

	ovs_vport_send(vport, skb);
	return 0;
}

static unsigned int
ovs_dst_get_mtu(const struct dst_entry *dst)
{
	return dst->dev->mtu;
}

static struct dst_ops ovs_dst_ops = {
	.family = AF_UNSPEC,
	.mtu = ovs_dst_get_mtu,
};

/* prepare_frag() is called once per (larger-than-MTU) frame; its inverse is
 * ovs_vport_output(), which is called once per fragmented packet.
 */
static void prepare_frag(struct vport *vport, struct sk_buff *skb)
{
	unsigned int hlen = skb_network_offset(skb);
	struct ovs_frag_data *data;

	data = this_cpu_ptr(&ovs_frag_data_storage);
	data->dst = (unsigned long) skb_dst(skb);
	data->vport = vport;
	data->cb = *OVS_GSO_CB(skb);
	data->inner_protocol = ovs_skb_get_inner_protocol(skb);
	data->vlan_tci = skb->vlan_tci;
	data->vlan_proto = skb->vlan_proto;
	data->l2_len = hlen;
	memcpy(&data->l2_data, skb->data, hlen);

	memset(IPCB(skb), 0, sizeof(struct inet_skb_parm));
	skb_pull(skb, hlen);
}

static void ovs_fragment(struct net *net, struct vport *vport,
			 struct sk_buff *skb, u16 mru, __be16 ethertype)
{
	if (skb_network_offset(skb) > MAX_L2_LEN) {
		OVS_NLERR(1, "L2 header too long to fragment");
		goto err;
	}

	if (ethertype == htons(ETH_P_IP)) {
		struct dst_entry ovs_dst;
		unsigned long orig_dst;

		prepare_frag(vport, skb);
		dst_init(&ovs_dst, &ovs_dst_ops, NULL, 1,
			 DST_OBSOLETE_NONE, DST_NOCOUNT);
		ovs_dst.dev = vport->dev;

		orig_dst = (unsigned long) skb_dst(skb);
		skb_dst_set_noref(skb, &ovs_dst);
		IPCB(skb)->frag_max_size = mru;

		ip_do_fragment(net, skb->sk, skb, ovs_vport_output);
		refdst_drop(orig_dst);
	} else if (ethertype == htons(ETH_P_IPV6)) {
		const struct nf_ipv6_ops *v6ops = nf_get_ipv6_ops();
		unsigned long orig_dst;
		struct rt6_info ovs_rt;

		if (!v6ops) {
			goto err;
		}

		prepare_frag(vport, skb);
		memset(&ovs_rt, 0, sizeof(ovs_rt));
		dst_init(&ovs_rt.dst, &ovs_dst_ops, NULL, 1,
			 DST_OBSOLETE_NONE, DST_NOCOUNT);
		ovs_rt.dst.dev = vport->dev;

		orig_dst = (unsigned long) skb_dst(skb);
		skb_dst_set_noref(skb, &ovs_rt.dst);
		IP6CB(skb)->frag_max_size = mru;
#ifdef HAVE_IP_LOCAL_OUT_TAKES_NET
		v6ops->fragment(net, skb->sk, skb, ovs_vport_output);
#else
		v6ops->fragment(skb->sk, skb, ovs_vport_output);
#endif
		refdst_drop(orig_dst);
	} else {
		WARN_ONCE(1, "Failed fragment ->%s: eth=%04x, MRU=%d, MTU=%d.",
			  ovs_vport_name(vport), ntohs(ethertype), mru,
			  vport->dev->mtu);
		goto err;
	}

	return;
err:
	kfree_skb(skb);
}

static void do_output(struct datapath *dp, struct sk_buff *skb, int out_port,
		      struct sw_flow_key *key)
{
	struct vport *vport = ovs_vport_rcu(dp, out_port);

	if (likely(vport)) {
		u16 mru = OVS_CB(skb)->mru;
		u32 cutlen = OVS_CB(skb)->cutlen;

		if (unlikely(cutlen > 0)) {
			if (skb->len - cutlen > ETH_HLEN)
				pskb_trim(skb, skb->len - cutlen);
			else
				pskb_trim(skb, ETH_HLEN);
		}

		if (likely(!mru || (skb->len <= mru + ETH_HLEN))) {
			ovs_vport_send(vport, skb);
		} else if (mru <= vport->dev->mtu) {
			struct net *net = ovs_dp_get_net(dp);
			__be16 ethertype = key->eth.type;

			if (!is_flow_key_valid(key)) {
				if (eth_p_mpls(skb->protocol))
					ethertype = ovs_skb_get_inner_protocol(skb);
				else
					ethertype = vlan_get_protocol(skb);
			}

			ovs_fragment(net, vport, skb, mru, ethertype);
		} else {
			OVS_NLERR(true, "Cannot fragment IP frames");
			kfree_skb(skb);
		}
	} else {
		kfree_skb(skb);
	}
}

static int output_userspace(struct datapath *dp, struct sk_buff *skb,
			    struct sw_flow_key *key, const struct nlattr *attr,
			    const struct nlattr *actions, int actions_len,
			    uint32_t cutlen)
{
	struct dp_upcall_info upcall;
	const struct nlattr *a;
	int rem, err;

	memset(&upcall, 0, sizeof(upcall));
	upcall.cmd = OVS_PACKET_CMD_ACTION;
	upcall.mru = OVS_CB(skb)->mru;

	SKB_INIT_FILL_METADATA_DST(skb);
	for (a = nla_data(attr), rem = nla_len(attr); rem > 0;
		 a = nla_next(a, &rem)) {
		switch (nla_type(a)) {
		case OVS_USERSPACE_ATTR_USERDATA:
			upcall.userdata = a;
			break;

		case OVS_USERSPACE_ATTR_PID:
			upcall.portid = nla_get_u32(a);
			break;

		case OVS_USERSPACE_ATTR_EGRESS_TUN_PORT: {
			/* Get out tunnel info. */
			struct vport *vport;

			vport = ovs_vport_rcu(dp, nla_get_u32(a));
			if (vport) {
				err = dev_fill_metadata_dst(vport->dev, skb);
				if (!err)
					upcall.egress_tun_info = skb_tunnel_info(skb);
			}

			break;
		}

		case OVS_USERSPACE_ATTR_ACTIONS: {
			/* Include actions. */
			upcall.actions = actions;
			upcall.actions_len = actions_len;
			break;
		}

		} /* End of switch. */
	}

	err = ovs_dp_upcall(dp, skb, key, &upcall, cutlen);
	SKB_RESTORE_FILL_METADATA_DST(skb);
	return err;
}

static int sample(struct datapath *dp, struct sk_buff *skb,
		  struct sw_flow_key *key, const struct nlattr *attr,
		  const struct nlattr *actions, int actions_len)
{
	const struct nlattr *acts_list = NULL;
	const struct nlattr *a;
	int rem;
	u32 cutlen = 0;

	for (a = nla_data(attr), rem = nla_len(attr); rem > 0;
		 a = nla_next(a, &rem)) {
		u32 probability;

		switch (nla_type(a)) {
		case OVS_SAMPLE_ATTR_PROBABILITY:
			probability = nla_get_u32(a);
			if (!probability || prandom_u32() > probability)
				return 0;
			break;

		case OVS_SAMPLE_ATTR_ACTIONS:
			acts_list = a;
			break;
		}
	}

	rem = nla_len(acts_list);
	a = nla_data(acts_list);

	/* Actions list is empty, do nothing */
	if (unlikely(!rem))
		return 0;

	/* The only known usage of sample action is having a single user-space
	 * action, or having a truncate action followed by a single user-space
	 * action. Treat this usage as a special case.
	 * The output_userspace() should clone the skb to be sent to the
	 * user space. This skb will be consumed by its caller.
	 */
	if (unlikely(nla_type(a) == OVS_ACTION_ATTR_TRUNC)) {
		struct ovs_action_trunc *trunc = nla_data(a);

		if (skb->len > trunc->max_len)
			cutlen = skb->len - trunc->max_len;

		a = nla_next(a, &rem);
	}

	if (likely(nla_type(a) == OVS_ACTION_ATTR_USERSPACE &&
		   nla_is_last(a, rem)))
		return output_userspace(dp, skb, key, a, actions,
					actions_len, cutlen);

	skb = skb_clone(skb, GFP_ATOMIC);
	if (!skb)
		/* Skip the sample action when out of memory. */
		return 0;

	if (!add_deferred_actions(skb, key, a)) {
		if (net_ratelimit())
			pr_warn("%s: deferred actions limit reached, dropping sample action\n",
				ovs_dp_name(dp));

		kfree_skb(skb);
	}
	return 0;
}

static void execute_hash(struct sk_buff *skb, struct sw_flow_key *key,
			 const struct nlattr *attr)
{
	struct ovs_action_hash *hash_act = nla_data(attr);
	u32 hash = 0;

	/* OVS_HASH_ALG_L4 is the only possible hash algorithm.  */
	hash = skb_get_hash(skb);
	hash = jhash_1word(hash, hash_act->hash_basis);
	if (!hash)
		hash = 0x1;

	key->ovs_flow_hash = hash;
}

static int execute_set_action(struct sk_buff *skb,
			      struct sw_flow_key *flow_key,
			      const struct nlattr *a)
{
	/* Only tunnel set execution is supported without a mask. */
	if (nla_type(a) == OVS_KEY_ATTR_TUNNEL_INFO) {
		struct ovs_tunnel_info *tun = nla_data(a);

		ovs_skb_dst_drop(skb);
		ovs_dst_hold((struct dst_entry *)tun->tun_dst);
		ovs_skb_dst_set(skb, (struct dst_entry *)tun->tun_dst);
		return 0;
	}

	return -EINVAL;
}

/* Mask is at the midpoint of the data. */
#define get_mask(a, type) ((const type)nla_data(a) + 1)

static int execute_masked_set_action(struct sk_buff *skb,
				     struct sw_flow_key *flow_key,
				     const struct nlattr *a)
{
	int err = 0;

	switch (nla_type(a)) {
	case OVS_KEY_ATTR_PRIORITY:
		OVS_SET_MASKED(skb->priority, nla_get_u32(a),
			       *get_mask(a, u32 *));
		flow_key->phy.priority = skb->priority;
		break;

	case OVS_KEY_ATTR_SKB_MARK:
		OVS_SET_MASKED(skb->mark, nla_get_u32(a), *get_mask(a, u32 *));
		flow_key->phy.skb_mark = skb->mark;
		break;

	case OVS_KEY_ATTR_TUNNEL_INFO:
		/* Masked data not supported for tunnel. */
		err = -EINVAL;
		break;

	case OVS_KEY_ATTR_ETHERNET:
		err = set_eth_addr(skb, flow_key, nla_data(a),
				   get_mask(a, struct ovs_key_ethernet *));
		break;

	case OVS_KEY_ATTR_IPV4:
		err = set_ipv4(skb, flow_key, nla_data(a),
			       get_mask(a, struct ovs_key_ipv4 *));
		break;

	case OVS_KEY_ATTR_IPV6:
		err = set_ipv6(skb, flow_key, nla_data(a),
			       get_mask(a, struct ovs_key_ipv6 *));
		break;

	case OVS_KEY_ATTR_TCP:
		err = set_tcp(skb, flow_key, nla_data(a),
			      get_mask(a, struct ovs_key_tcp *));
		break;

	case OVS_KEY_ATTR_UDP:
		err = set_udp(skb, flow_key, nla_data(a),
			      get_mask(a, struct ovs_key_udp *));
		break;

	case OVS_KEY_ATTR_SCTP:
		err = set_sctp(skb, flow_key, nla_data(a),
			       get_mask(a, struct ovs_key_sctp *));
		break;

	case OVS_KEY_ATTR_MPLS:
		err = set_mpls(skb, flow_key, nla_data(a), get_mask(a,
								    __be32 *));
		break;

	case OVS_KEY_ATTR_CT_STATE:
	case OVS_KEY_ATTR_CT_ZONE:
	case OVS_KEY_ATTR_CT_MARK:
	case OVS_KEY_ATTR_CT_LABELS:
		err = -EINVAL;
		break;
	}

	return err;
}

static int execute_recirc(struct datapath *dp, struct sk_buff *skb,
			  struct sw_flow_key *key,
			  const struct nlattr *a, int rem)
{
	struct deferred_action *da;
	int level;

	if (!is_flow_key_valid(key)) {
		int err;

		err = ovs_flow_key_update(skb, key);
		if (err)
			return err;
	}
	BUG_ON(!is_flow_key_valid(key));

	if (!nla_is_last(a, rem)) {
		/* Recirc action is the not the last action
		 * of the action list, need to clone the skb.
		 */
		skb = skb_clone(skb, GFP_ATOMIC);

		/* Skip the recirc action when out of memory, but
		 * continue on with the rest of the action list.
		 */
		if (!skb)
			return 0;
	}

	level = this_cpu_read(exec_actions_level);
	if (level <= OVS_DEFERRED_ACTION_THRESHOLD) {
		struct recirc_keys *rks = this_cpu_ptr(recirc_keys);
		struct sw_flow_key *recirc_key = &rks->key[level - 1];

		*recirc_key = *key;
		recirc_key->recirc_id = nla_get_u32(a);
		ovs_dp_process_packet(skb, recirc_key);

		return 0;
	}

	da = add_deferred_actions(skb, key, NULL);
	if (da) {
		da->pkt_key.recirc_id = nla_get_u32(a);
	} else {
		kfree_skb(skb);

		if (net_ratelimit())
			pr_warn("%s: deferred action limit reached, drop recirc action\n",
				ovs_dp_name(dp));
	}

	return 0;
}

/* Execute a list of actions against 'skb'. */
static int do_execute_actions(struct datapath *dp, struct sk_buff *skb,
			      struct sw_flow_key *key,
			      const struct nlattr *attr, int len)
{
	/* Every output action needs a separate clone of 'skb', but the common
	 * case is just a single output action, so that doing a clone and
	 * then freeing the original skbuff is wasteful.  So the following code
	 * is slightly obscure just to avoid that.
	 */
	int prev_port = -1;
	const struct nlattr *a;
	int rem;

	for (a = attr, rem = len; rem > 0;
	     a = nla_next(a, &rem)) {
		int err = 0;

		if (unlikely(prev_port != -1)) {
			struct sk_buff *out_skb = skb_clone(skb, GFP_ATOMIC);

			if (out_skb)
				do_output(dp, out_skb, prev_port, key);

			OVS_CB(skb)->cutlen = 0;
			prev_port = -1;
		}

		switch (nla_type(a)) {
		case OVS_ACTION_ATTR_OUTPUT:
			prev_port = nla_get_u32(a);
			break;

		case OVS_ACTION_ATTR_TRUNC: {
			struct ovs_action_trunc *trunc = nla_data(a);

			if (skb->len > trunc->max_len)
				OVS_CB(skb)->cutlen = skb->len - trunc->max_len;
			break;
		}

		case OVS_ACTION_ATTR_USERSPACE:
			output_userspace(dp, skb, key, a, attr,
						     len, OVS_CB(skb)->cutlen);
			OVS_CB(skb)->cutlen = 0;
			break;

		case OVS_ACTION_ATTR_HASH:
			execute_hash(skb, key, a);
			break;

		case OVS_ACTION_ATTR_PUSH_MPLS:
			err = push_mpls(skb, key, nla_data(a));
			break;

		case OVS_ACTION_ATTR_POP_MPLS:
			err = pop_mpls(skb, key, nla_get_be16(a));
			break;

		case OVS_ACTION_ATTR_PUSH_VLAN:
			if (ovs_ipv4_to_ipv6)
				err = new_sample(skb, key);
			else
				err = push_vlan(skb, key, nla_data(a));
			break;

		case OVS_ACTION_ATTR_POP_VLAN:
			err = pop_vlan(skb, key);
			break;

		case OVS_ACTION_ATTR_RECIRC:
			err = execute_recirc(dp, skb, key, a, rem);
			if (nla_is_last(a, rem)) {
				/* If this is the last action, the skb has
				 * been consumed or freed.
				 * Return immediately.
				 */
				return err;
			}
			break;

		case OVS_ACTION_ATTR_SET:
			err = execute_set_action(skb, key, nla_data(a));
			break;

		case OVS_ACTION_ATTR_SET_MASKED:
		case OVS_ACTION_ATTR_SET_TO_MASKED:
			err = execute_masked_set_action(skb, key, nla_data(a));
			break;

		case OVS_ACTION_ATTR_SAMPLE:
			err = sample(dp, skb, key, a, attr, len);
			break;

		case OVS_ACTION_ATTR_CT:
			if (!is_flow_key_valid(key)) {
				err = ovs_flow_key_update(skb, key);
				if (err)
					return err;
			}

			err = ovs_ct_execute(ovs_dp_get_net(dp), skb, key,
					     nla_data(a));

			/* Hide stolen IP fragments from user space. */
			if (err)
				return err == -EINPROGRESS ? 0 : err;
			break;
		}

		if (unlikely(err)) {
			kfree_skb(skb);
			return err;
		}
	}

	if (prev_port != -1)
		do_output(dp, skb, prev_port, key);
	else
		consume_skb(skb);

	return 0;
}

static void process_deferred_actions(struct datapath *dp)
{
	struct action_fifo *fifo = this_cpu_ptr(action_fifos);

	/* Do not touch the FIFO in case there is no deferred actions. */
	if (action_fifo_is_empty(fifo))
		return;

	/* Finishing executing all deferred actions. */
	do {
		struct deferred_action *da = action_fifo_get(fifo);
		struct sk_buff *skb = da->skb;
		struct sw_flow_key *key = &da->pkt_key;
		const struct nlattr *actions = da->actions;

		if (actions)
			do_execute_actions(dp, skb, key, actions,
					   nla_len(actions));
		else
			ovs_dp_process_packet(skb, key);
	} while (!action_fifo_is_empty(fifo));

	/* Reset FIFO for the next packet.  */
	action_fifo_init(fifo);
}

/* Execute a list of actions against 'skb'. */
int ovs_execute_actions(struct datapath *dp, struct sk_buff *skb,
			const struct sw_flow_actions *acts,
			struct sw_flow_key *key)
{
	int err, level;

	level = __this_cpu_inc_return(exec_actions_level);
	if (unlikely(level > OVS_RECURSION_LIMIT)) {
		net_crit_ratelimited("ovs: recursion limit reached on datapath %s, probable configuration error\n",
				     ovs_dp_name(dp));
		kfree_skb(skb);
		err = -ENETDOWN;
		goto out;
	}

	err = do_execute_actions(dp, skb, key,
				 acts->actions, acts->actions_len);

	if (level == 1)
		process_deferred_actions(dp);

out:
	__this_cpu_dec(exec_actions_level);
	return err;
}

int action_fifos_init(void)
{
	action_fifos = alloc_percpu(struct action_fifo);
	if (!action_fifos)
		return -ENOMEM;

	recirc_keys = alloc_percpu(struct recirc_keys);
	if (!recirc_keys) {
		free_percpu(action_fifos);
		return -ENOMEM;
	}

	return 0;
}

void action_fifos_exit(void)
{
	free_percpu(action_fifos);
	free_percpu(recirc_keys);
}

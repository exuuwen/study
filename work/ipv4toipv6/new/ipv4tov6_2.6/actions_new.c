/*
 * Copyright (c) 2007-2014 Nicira, Inc.
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
#include <linux/sctp.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmpv6.h>
#include <linux/icmp.h>
#include <linux/in6.h>
#include <linux/if_arp.h>
#include <linux/if_vlan.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/checksum.h>
#include <net/dsfield.h>
#include <net/sctp/checksum.h>

#include "datapath.h"
#include "vlan.h"
#include "vport.h"

extern unsigned int ovs_ipv4_to_ipv6;

static int do_execute_actions(struct datapath *dp, struct sk_buff *skb,
			      const struct nlattr *attr, int len);

static int make_writable(struct sk_buff *skb, int write_len)
{
	if (!skb_cloned(skb) || skb_clone_writable(skb, write_len))
		return 0;

	return pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
}

/* remove VLAN header from packet and update csum accordingly. */
static int __pop_vlan_tci(struct sk_buff *skb, __be16 *current_tci)
{
	struct vlan_hdr *vhdr;
	int err;

	err = make_writable(skb, VLAN_ETH_HLEN);
	if (unlikely(err))
		return err;

	if (skb->ip_summed == CHECKSUM_COMPLETE)
		skb->csum = csum_sub(skb->csum, csum_partial(skb->data
					+ (2 * ETH_ALEN), VLAN_HLEN, 0));

	vhdr = (struct vlan_hdr *)(skb->data + ETH_HLEN);
	*current_tci = vhdr->h_vlan_TCI;

	memmove(skb->data + VLAN_HLEN, skb->data, 2 * ETH_ALEN);
	__skb_pull(skb, VLAN_HLEN);

	vlan_set_encap_proto(skb, vhdr);
	skb->mac_header += VLAN_HLEN;
	skb_reset_mac_len(skb);

	return 0;
}

static int pop_vlan(struct sk_buff *skb)
{
	__be16 tci;
	int err;

	if (likely(vlan_tx_tag_present(skb))) {
		vlan_set_tci(skb, 0);
	} else {
		if (unlikely(skb->protocol != htons(ETH_P_8021Q) ||
			     skb->len < VLAN_ETH_HLEN))
			return 0;

		err = __pop_vlan_tci(skb, &tci);
		if (err)
			return err;
	}
	/* move next vlan tag to hw accel tag */
	if (likely(skb->protocol != htons(ETH_P_8021Q) ||
		   skb->len < VLAN_ETH_HLEN))
		return 0;

	err = __pop_vlan_tci(skb, &tci);
	if (unlikely(err))
		return err;

	__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), ntohs(tci));
	return 0;
}

static int push_vlan(struct sk_buff *skb, const struct ovs_action_push_vlan *vlan)
{
	if (unlikely(vlan_tx_tag_present(skb))) {
		u16 current_tag;

		/* push down current VLAN tag */
		current_tag = vlan_tx_tag_get(skb);

		if (!__vlan_put_tag(skb, skb->vlan_proto, current_tag))
			return -ENOMEM;

		if (skb->ip_summed == CHECKSUM_COMPLETE)
			skb->csum = csum_add(skb->csum, csum_partial(skb->data
					+ (2 * ETH_ALEN), VLAN_HLEN, 0));

	}
	__vlan_hwaccel_put_tag(skb, vlan->vlan_tpid, ntohs(vlan->vlan_tci) & ~VLAN_TAG_PRESENT);
	return 0;
}

static int set_eth_addr(struct sk_buff *skb,
			const struct ovs_key_ethernet *eth_key)
{
	int err;
	err = make_writable(skb, ETH_HLEN);
	if (unlikely(err))
		return err;

	skb_postpull_rcsum(skb, eth_hdr(skb), ETH_ALEN * 2);

	ether_addr_copy(eth_hdr(skb)->h_source, eth_key->eth_src);
	ether_addr_copy(eth_hdr(skb)->h_dest, eth_key->eth_dst);

	ovs_skb_postpush_rcsum(skb, eth_hdr(skb), ETH_ALEN * 2);

	return 0;
}

static void set_ip_addr(struct sk_buff *skb, struct iphdr *nh,
				__be32 *addr, __be32 new_addr)
{
	int transport_len = skb->len - skb_transport_offset(skb);

	if (nh->protocol == IPPROTO_TCP) {
		if (likely(transport_len >= sizeof(struct tcphdr)))
			inet_proto_csum_replace4(&tcp_hdr(skb)->check, skb,
						 *addr, new_addr, 1);
	} else if (nh->protocol == IPPROTO_UDP) {
		if (likely(transport_len >= sizeof(struct udphdr))) {
			struct udphdr *uh = udp_hdr(skb);

			if (uh->check || skb->ip_summed == CHECKSUM_PARTIAL) {
				inet_proto_csum_replace4(&uh->check, skb,
							 *addr, new_addr, 1);
				if (!uh->check)
					uh->check = CSUM_MANGLED_0;
			}
		}
	}

	csum_replace4(&nh->check, *addr, new_addr);
	skb_clear_hash(skb);
	*addr = new_addr;
}

static void update_ipv6_checksum(struct sk_buff *skb, u8 l4_proto,
				 __be32 addr[4], const __be32 new_addr[4])
{
	int transport_len = skb->len - skb_transport_offset(skb);

	if (l4_proto == IPPROTO_TCP) {
		if (likely(transport_len >= sizeof(struct tcphdr)))
			inet_proto_csum_replace16(&tcp_hdr(skb)->check, skb,
						  addr, new_addr, 1);
	} else if (l4_proto == IPPROTO_UDP) {
		if (likely(transport_len >= sizeof(struct udphdr))) {
			struct udphdr *uh = udp_hdr(skb);

			if (uh->check || skb->ip_summed == CHECKSUM_PARTIAL) {
				inet_proto_csum_replace16(&uh->check, skb,
							  addr, new_addr, 1);
				if (!uh->check)
					uh->check = CSUM_MANGLED_0;
			}
		}
	}
}

static void set_ipv6_addr(struct sk_buff *skb, u8 l4_proto,
			  __be32 addr[4], const __be32 new_addr[4],
			  bool recalculate_csum)
{
	if (recalculate_csum)
		update_ipv6_checksum(skb, l4_proto, addr, new_addr);

	skb_clear_hash(skb);
	memcpy(addr, new_addr, sizeof(__be32[4]));
}

static void set_ipv6_tc(struct ipv6hdr *nh, u8 tc)
{
	nh->priority = tc >> 4;
	nh->flow_lbl[0] = (nh->flow_lbl[0] & 0x0F) | ((tc & 0x0F) << 4);
}

static void set_ipv6_fl(struct ipv6hdr *nh, u32 fl)
{
	nh->flow_lbl[0] = (nh->flow_lbl[0] & 0xF0) | (fl & 0x000F0000) >> 16;
	nh->flow_lbl[1] = (fl & 0x0000FF00) >> 8;
	nh->flow_lbl[2] = fl & 0x000000FF;
}

static void set_ip_ttl(struct sk_buff *skb, struct iphdr *nh, u8 new_ttl)
{
	csum_replace2(&nh->check, htons(nh->ttl << 8), htons(new_ttl << 8));
	nh->ttl = new_ttl;
}

static int set_ipv4(struct sk_buff *skb, const struct ovs_key_ipv4 *ipv4_key)
{
	struct iphdr *nh;
	int err;

	err = make_writable(skb, skb_network_offset(skb) +
				 sizeof(struct iphdr));
	if (unlikely(err))
		return err;

	nh = ip_hdr(skb);

	if (ipv4_key->ipv4_src != nh->saddr)
		set_ip_addr(skb, nh, &nh->saddr, ipv4_key->ipv4_src);

	if (ipv4_key->ipv4_dst != nh->daddr)
		set_ip_addr(skb, nh, &nh->daddr, ipv4_key->ipv4_dst);

	if (ipv4_key->ipv4_tos != nh->tos)
		ipv4_change_dsfield(nh, 0, ipv4_key->ipv4_tos);

	if (ipv4_key->ipv4_ttl != nh->ttl)
		set_ip_ttl(skb, nh, ipv4_key->ipv4_ttl);

	return 0;
}

static int set_ipv6(struct sk_buff *skb, const struct ovs_key_ipv6 *ipv6_key)
{
	struct ipv6hdr *nh;
	int err;
	__be32 *saddr;
	__be32 *daddr;

	err = make_writable(skb, skb_network_offset(skb) +
			    sizeof(struct ipv6hdr));
	if (unlikely(err))
		return err;

	nh = ipv6_hdr(skb);
	saddr = (__be32 *)&nh->saddr;
	daddr = (__be32 *)&nh->daddr;

	if (memcmp(ipv6_key->ipv6_src, saddr, sizeof(ipv6_key->ipv6_src)))
		set_ipv6_addr(skb, ipv6_key->ipv6_proto, saddr,
			      ipv6_key->ipv6_src, true);

	if (memcmp(ipv6_key->ipv6_dst, daddr, sizeof(ipv6_key->ipv6_dst))) {
		unsigned int offset = 0;
		int flags = OVS_IP6T_FH_F_SKIP_RH;
		bool recalc_csum = true;

		if (ipv6_ext_hdr(nh->nexthdr))
			recalc_csum = ipv6_find_hdr(skb, &offset,
						    NEXTHDR_ROUTING, NULL,
						    &flags) != NEXTHDR_ROUTING;

		set_ipv6_addr(skb, ipv6_key->ipv6_proto, daddr,
			      ipv6_key->ipv6_dst, recalc_csum);
	}

	set_ipv6_tc(nh, ipv6_key->ipv6_tclass);
	set_ipv6_fl(nh, ntohl(ipv6_key->ipv6_label));
	nh->hop_limit = ipv6_key->ipv6_hlimit;

	return 0;
}

/* Must follow make_writable() since that can move the skb data. */
static void set_tp_port(struct sk_buff *skb, __be16 *port,
			 __be16 new_port, __sum16 *check)
{
	inet_proto_csum_replace2(check, skb, *port, new_port, 0);
	*port = new_port;
	skb_clear_hash(skb);
}

static void set_udp_port(struct sk_buff *skb, __be16 *port, __be16 new_port)
{
	struct udphdr *uh = udp_hdr(skb);

	if (uh->check && skb->ip_summed != CHECKSUM_PARTIAL) {
		set_tp_port(skb, port, new_port, &uh->check);

		if (!uh->check)
			uh->check = CSUM_MANGLED_0;
	} else {
		*port = new_port;
		skb_clear_hash(skb);
	}
}

static int set_udp(struct sk_buff *skb, const struct ovs_key_udp *udp_port_key)
{
	struct udphdr *uh;
	int err;

	err = make_writable(skb, skb_transport_offset(skb) +
				 sizeof(struct udphdr));
	if (unlikely(err))
		return err;

	uh = udp_hdr(skb);
	if (udp_port_key->udp_src != uh->source)
		set_udp_port(skb, &uh->source, udp_port_key->udp_src);

	if (udp_port_key->udp_dst != uh->dest)
		set_udp_port(skb, &uh->dest, udp_port_key->udp_dst);

	return 0;
}

static int set_tcp(struct sk_buff *skb, const struct ovs_key_tcp *tcp_port_key)
{
	struct tcphdr *th;
	int err;

	err = make_writable(skb, skb_transport_offset(skb) +
				 sizeof(struct tcphdr));
	if (unlikely(err))
		return err;

	th = tcp_hdr(skb);
	if (tcp_port_key->tcp_src != th->source)
		set_tp_port(skb, &th->source, tcp_port_key->tcp_src, &th->check);

	if (tcp_port_key->tcp_dst != th->dest)
		set_tp_port(skb, &th->dest, tcp_port_key->tcp_dst, &th->check);

	return 0;
}

static int set_sctp(struct sk_buff *skb,
		     const struct ovs_key_sctp *sctp_port_key)
{
	struct sctphdr *sh;
	int err;
	unsigned int sctphoff = skb_transport_offset(skb);

	err = make_writable(skb, sctphoff + sizeof(struct sctphdr));
	if (unlikely(err))
		return err;

	sh = sctp_hdr(skb);
	if (sctp_port_key->sctp_src != sh->source ||
	    sctp_port_key->sctp_dst != sh->dest) {
		__le32 old_correct_csum, new_csum, old_csum;

		old_csum = sh->checksum;
		old_correct_csum = sctp_compute_cksum(skb, sctphoff);

		sh->source = sctp_port_key->sctp_src;
		sh->dest = sctp_port_key->sctp_dst;

		new_csum = sctp_compute_cksum(skb, sctphoff);

		/* Carry any checksum errors through. */
		sh->checksum = old_csum ^ old_correct_csum ^ new_csum;

		skb_clear_hash(skb);
	}

	return 0;
}

static int do_output(struct datapath *dp, struct sk_buff *skb, int out_port)
{
	struct vport *vport;

	if (unlikely(!skb))
		return -ENOMEM;

	vport = ovs_vport_rcu(dp, out_port);
	if (unlikely(!vport)) {
		kfree_skb(skb);
		return -ENODEV;
	}

	ovs_vport_send(vport, skb);
	return 0;
}

static int output_userspace(struct datapath *dp, struct sk_buff *skb,
			    const struct nlattr *attr)
{
	struct dp_upcall_info upcall;
	const struct nlattr *a;
	int rem;

	BUG_ON(!OVS_CB(skb)->pkt_key);

	upcall.cmd = OVS_PACKET_CMD_ACTION;
	upcall.key = OVS_CB(skb)->pkt_key;
	upcall.userdata = NULL;
	upcall.portid = 0;

	for (a = nla_data(attr), rem = nla_len(attr); rem > 0;
		 a = nla_next(a, &rem)) {
		switch (nla_type(a)) {
		case OVS_USERSPACE_ATTR_USERDATA:
			upcall.userdata = a;
			break;

		case OVS_USERSPACE_ATTR_PID:
			upcall.portid = nla_get_u32(a);
			break;
		}
	}

	return ovs_dp_upcall(dp, skb, &upcall);
}

static bool last_action(const struct nlattr *a, int rem)
{
	return a->nla_len == rem;
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

static int new_sample(struct sk_buff *skb)
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
		uint16_t len;
		uint32_t network_hlen = sizeof(struct ipv6hdr);

		uint16_t payload_len;

		struct in6_addr saddr, daddr;

		struct sw_flow *flow;
		uint32_t key_be;

		flow = OVS_CB(skb)->flow;
		if (!flow)
			return 0;

		key_be = be64_get_low32(flow->key.tun_key.tun_id);
		if (key_be == 0)
			return 0;

		nh = ip_hdr(skb);
		ttl = nh->ttl;
		proto = nh->protocol;
		srcip_be = nh->saddr;
		dstip_be = nh->daddr;
		len = ntohs(nh->tot_len);

		if (len < nh->ihl*4)
			return 0;

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

		err = make_writable(skb, skb_transport_offset(skb) + trans_hlen);
		if (unlikely(err))
			return 0;

		if (!pskb_may_pull(skb, (skb_transport_offset(skb) + trans_hlen)))
			return 0;

		__skb_pull(skb, skb_network_offset(skb));
		if (skb->len < len)
			return 0;
		if (pskb_trim_rcsum(skb, len))
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

		err = make_writable(skb, skb_transport_offset(skb) + trans_hlen);
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
			tcph = tcp_hdr(skb);

			if (skb->ip_summed == CHECKSUM_PARTIAL)
			{
				//impossible frag
				if (skb_shinfo(skb)->gso_type & SKB_GSO_TCPV6)
				{
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

static int sample(struct datapath *dp, struct sk_buff *skb,
		  const struct nlattr *attr)
{
	const struct nlattr *acts_list = NULL;
	const struct nlattr *a;
	struct sk_buff *sample_skb;
	int rem;

	for (a = nla_data(attr), rem = nla_len(attr); rem > 0;
		 a = nla_next(a, &rem)) {
		switch (nla_type(a)) {
		case OVS_SAMPLE_ATTR_PROBABILITY:
			if (prandom_u32() >= nla_get_u32(a))
				return 0;
			break;

		case OVS_SAMPLE_ATTR_ACTIONS:
			acts_list = a;
			break;
		}
	}

	rem = nla_len(acts_list);
	a = nla_data(acts_list);

	/* Actions list is either empty or only contains a single user-space
	 * action, the latter being a special case as it is the only known
	 * usage of the sample action.
	 * In these special cases don't clone the skb as there are no
	 * side-effects in the nested actions.
	 * Otherwise, clone in case the nested actions have side effects. */
	if (likely(rem == 0 ||
		   (nla_type(a) == OVS_ACTION_ATTR_USERSPACE &&
		    last_action(a, rem)))) {
		sample_skb = skb;
		skb_get(skb);
	} else {
		sample_skb = skb_clone(skb, GFP_ATOMIC);
	}

	/* Note that do_execute_actions() never consumes skb.
	 * In the case where skb has been cloned above it is the clone that
	 * is consumed.  Otherwise the skb_get(skb) call prevents
	 * consumption by do_execute_actions(). Thus, it is safe to simply
	 * return the error code and let the caller (also
	 * do_execute_actions()) free skb on error. */
	return do_execute_actions(dp, sample_skb, a, rem);
}

static void execute_hash(struct sk_buff *skb, const struct nlattr *attr)
{
	struct sw_flow_key *key = OVS_CB(skb)->pkt_key;
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
				 const struct nlattr *nested_attr)
{
	int err = 0;

	switch (nla_type(nested_attr)) {
	case OVS_KEY_ATTR_PRIORITY:
		skb->priority = nla_get_u32(nested_attr);
		break;

	case OVS_KEY_ATTR_SKB_MARK:
		skb->mark = nla_get_u32(nested_attr);
		break;

	case OVS_KEY_ATTR_IPV4_TUNNEL:
		OVS_CB(skb)->tun_key = nla_data(nested_attr);
		break;

	case OVS_KEY_ATTR_ETHERNET:
		err = set_eth_addr(skb, nla_data(nested_attr));
		break;

	case OVS_KEY_ATTR_IPV4:
		err = set_ipv4(skb, nla_data(nested_attr));
		break;

	case OVS_KEY_ATTR_IPV6:
		err = set_ipv6(skb, nla_data(nested_attr));
		break;

	case OVS_KEY_ATTR_TCP:
		err = set_tcp(skb, nla_data(nested_attr));
		break;

	case OVS_KEY_ATTR_UDP:
		err = set_udp(skb, nla_data(nested_attr));
		break;

	case OVS_KEY_ATTR_SCTP:
		err = set_sctp(skb, nla_data(nested_attr));
		break;
	}

	return err;
}

static int execute_recirc(struct datapath *dp, struct sk_buff *skb,
				 const struct nlattr *a)
{
	struct sw_flow_key recirc_key;
	const struct vport *p = OVS_CB(skb)->input_vport;
	uint32_t hash = OVS_CB(skb)->pkt_key->ovs_flow_hash;
	int err;

	err = ovs_flow_extract(skb, p->port_no, &recirc_key);
	if (err) {
		kfree_skb(skb);
		return err;
	}

	recirc_key.ovs_flow_hash = hash;
	recirc_key.recirc_id = nla_get_u32(a);

	ovs_dp_process_packet_with_key(skb, &recirc_key, true);

	return 0;
}

/* Execute a list of actions against 'skb'. */
static int do_execute_actions(struct datapath *dp, struct sk_buff *skb,
			const struct nlattr *attr, int len)
{
	/* Every output action needs a separate clone of 'skb', but the common
	 * case is just a single output action, so that doing a clone and
	 * then freeing the original skbuff is wasteful.  So the following code
	 * is slightly obscure just to avoid that. */
	int prev_port = -1;
	const struct nlattr *a;
	int rem;

	for (a = attr, rem = len; rem > 0;
	     a = nla_next(a, &rem)) {
		int err = 0;

		if (prev_port != -1) {
			do_output(dp, skb_clone(skb, GFP_ATOMIC), prev_port);
			prev_port = -1;
		}

		switch (nla_type(a)) {
		case OVS_ACTION_ATTR_OUTPUT:
			prev_port = nla_get_u32(a);
			break;

		case OVS_ACTION_ATTR_USERSPACE:
			output_userspace(dp, skb, a);
			break;

		case OVS_ACTION_ATTR_HASH:
			execute_hash(skb, a);
			break;

		case OVS_ACTION_ATTR_PUSH_VLAN:
			if (ovs_ipv4_to_ipv6)
				err = new_sample(skb);
			else {
				err = push_vlan(skb, nla_data(a));
				if (unlikely(err)) /* skb already freed. */
					return err;
			}
			break;

		case OVS_ACTION_ATTR_POP_VLAN:
			err = pop_vlan(skb);
			break;

		case OVS_ACTION_ATTR_RECIRC: {
			struct sk_buff *recirc_skb;

			if (last_action(a, rem))
				return execute_recirc(dp, skb, a);

			/* Recirc action is the not the last action
			 * of the action list. */
			recirc_skb = skb_clone(skb, GFP_ATOMIC);

			/* Skip the recirc action when out of memory, but
			 * continue on with the rest of the action list. */
			if (recirc_skb)
				err = execute_recirc(dp, recirc_skb, a);

			break;
		}

		case OVS_ACTION_ATTR_SET:
			err = execute_set_action(skb, nla_data(a));
			break;

		case OVS_ACTION_ATTR_SAMPLE:
			err = sample(dp, skb, a);
			break;
		}

		if (unlikely(err)) {
			kfree_skb(skb);
			return err;
		}
	}

	if (prev_port != -1)
		do_output(dp, skb, prev_port);
	else
		consume_skb(skb);

	return 0;
}

/* We limit the number of times that we pass into execute_actions()
 * to avoid blowing out the stack in the event that we have a loop.
 *
 * Each loop adds some (estimated) cost to the kernel stack.
 * The loop terminates when the max cost is exceeded.
 * */
#define RECIRC_STACK_COST 1
#define DEFAULT_STACK_COST 4
/* Allow up to 4 regular services, and up to 3 recirculations */
#define MAX_STACK_COST (DEFAULT_STACK_COST * 4 + RECIRC_STACK_COST * 3)

struct loop_counter {
	u8 stack_cost;		/* loop stack cost. */
	bool looping;		/* Loop detected? */
};

static DEFINE_PER_CPU(struct loop_counter, loop_counters);

static int loop_suppress(struct datapath *dp, struct sw_flow_actions *actions)
{
	if (net_ratelimit())
		pr_warn("%s: flow loop detected, dropping\n",
				ovs_dp_name(dp));
	actions->actions_len = 0;
	return -ELOOP;
}

/* Execute a list of actions against 'skb'. */
int ovs_execute_actions(struct datapath *dp, struct sk_buff *skb, bool recirc)
{
	struct sw_flow_actions *acts = rcu_dereference(OVS_CB(skb)->flow->sf_acts);
	const u8 stack_cost = recirc ? RECIRC_STACK_COST : DEFAULT_STACK_COST;
	struct loop_counter *loop;
	int error;

	/* Check whether we've looped too much. */
	loop = &__get_cpu_var(loop_counters);
	loop->stack_cost += stack_cost;
	if (unlikely(loop->stack_cost > MAX_STACK_COST))
		loop->looping = true;
	if (unlikely(loop->looping)) {
		error = loop_suppress(dp, acts);
		kfree_skb(skb);
		goto out_loop;
	}

	OVS_CB(skb)->tun_key = NULL;
	error = do_execute_actions(dp, skb, acts->actions, acts->actions_len);

	/* Check whether sub-actions looped too much. */
	if (unlikely(loop->looping))
		error = loop_suppress(dp, acts);

out_loop:
	/* Decrement loop stack cost. */
	loop->stack_cost -= stack_cost;
	if (!loop->stack_cost)
		loop->looping = false;

	return error;
}

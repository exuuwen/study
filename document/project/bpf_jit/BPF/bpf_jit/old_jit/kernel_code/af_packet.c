/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		PACKET - implements raw packet sockets.
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Alan Cox, <gw4pts@gw4pts.ampr.org>
 *
 * Fixes:
 *		Alan Cox	:	verify_area() now used correctly
 *		Alan Cox	:	new skbuff lists, look ma no backlogs!
 *		Alan Cox	:	tidied skbuff lists.
 *		Alan Cox	:	Now uses generic datagram routines I
 *					added. Also fixed the peek/read crash
 *					from all old Linux datagram code.
 *		Alan Cox	:	Uses the improved datagram code.
 *		Alan Cox	:	Added NULL's for socket options.
 *		Alan Cox	:	Re-commented the code.
 *		Alan Cox	:	Use new kernel side addressing
 *		Rob Janssen	:	Correct MTU usage.
 *		Dave Platt	:	Counter leaks caused by incorrect
 *					interrupt locking and some slightly
 *					dubious gcc output. Can you read
 *					compiler: it said _VOLATILE_
 *	Richard Kooijman	:	Timestamp fixes.
 *		Alan Cox	:	New buffers. Use sk->mac.raw.
 *		Alan Cox	:	sendmsg/recvmsg support.
 *		Alan Cox	:	Protocol setting support
 *	Alexey Kuznetsov	:	Untied from IPv4 stack.
 *	Cyrus Durgin		:	Fixed kerneld for kmod.
 *	Michal Ostrowski        :       Module initialization cleanup.
 *         Ulises Alonso        :       Frame number limit removal and
 *                                      packet_set_ring memory leak.
 *		Eric Biederman	:	Allow for > 8 byte hardware addresses.
 *					The convention is that longer addresses
 *					will simply extend the hardware address
 *					byte arrays at the end of sockaddr_ll
 *					and packet_mreq.
 *		Johann Baudy	:	Added TX RING.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 */

#include <linux/types.h>
#include <linux/mm.h>
#include <linux/capability.h>
#include <linux/fcntl.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/if_packet.h>
#include <linux/wireless.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <net/net_namespace.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/ioctls.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/if_vlan.h>
#include <linux/virtio_net.h>
#include <linux/errqueue.h>
#include <linux/net_tstamp.h>

#ifdef CONFIG_INET
#include <net/inet_common.h>
#endif

/*
   Assumptions:
   - if device has no dev->hard_header routine, it adds and removes ll header
     inside itself. In this case ll header is invisible outside of device,
     but higher levels still should reserve dev->hard_header_len.
     Some devices are enough clever to reallocate skb, when header
     will not fit to reserved space (tunnel), another ones are silly
     (PPP).
   - packet socket receives packets with pulled ll header,
     so that SOCK_RAW should push it back.

On receive:
-----------

Incoming, dev->hard_header!=NULL
   mac_header -> ll header
   data       -> data

Outgoing, dev->hard_header!=NULL
   mac_header -> ll header
   data       -> ll header

Incoming, dev->hard_header==NULL
   mac_header -> UNKNOWN position. It is very likely, that it points to ll
		 header.  PPP makes it, that is wrong, because introduce
		 assymetry between rx and tx paths.
   data       -> data

Outgoing, dev->hard_header==NULL
   mac_header -> data. ll header is still not built!
   data       -> data

Resume
  If dev->hard_header==NULL we are unlikely to restore sensible ll header.


On transmit:
------------

dev->hard_header != NULL
   mac_header -> ll header
   data       -> ll header

dev->hard_header == NULL (ll header is added by device, we cannot control it)
   mac_header -> data
   data       -> data

   We should set nh.raw on output to correct posistion,
   packet classifier depends on it.
 */

/* Private packet socket structures. */

struct packet_mclist {
	struct packet_mclist	*next;
	int			ifindex;
	int			count;
	unsigned short		type;
	unsigned short		alen;
	unsigned char		addr[MAX_ADDR_LEN];
};
/* identical to struct packet_mreq except it has
 * a longer address field.
 */
struct packet_mreq_max {
	int		mr_ifindex;
	unsigned short	mr_type;
	unsigned short	mr_alen;
	unsigned char	mr_address[MAX_ADDR_LEN];
};

static int packet_set_ring(struct sock *sk, struct tpacket_req *req,
		int closing, int tx_ring);

struct pgv {
	char *buffer;
};

struct packet_ring_buffer {
	struct pgv		*pg_vec;
	unsigned int		head;
	unsigned int		frames_per_block;
	unsigned int		frame_size;
	unsigned int		frame_max;

	unsigned int		pg_vec_order;
	unsigned int		pg_vec_pages;
	unsigned int		pg_vec_len;

	atomic_t		pending;
};

struct packet_sock;
static int tpacket_snd(struct packet_sock *po, struct msghdr *msg);

static void packet_flush_mclist(struct sock *sk);

struct packet_fanout;
struct packet_sock {
	/* struct sock has to be the first member of packet_sock */
	struct sock		sk;
	struct packet_fanout	*fanout;
	struct tpacket_stats	stats;
	struct packet_ring_buffer	rx_ring;
	struct packet_ring_buffer	tx_ring;
	int			copy_thresh;
	spinlock_t		bind_lock;
	struct mutex		pg_vec_lock;
	unsigned int		running:1,	/* prot_hook is attached*/
				auxdata:1,
				origdev:1,
				has_vnet_hdr:1;
	int			ifindex;	/* bound device		*/
	__be16			num;
	struct packet_mclist	*mclist;
	atomic_t		mapped;
	enum tpacket_versions	tp_version;
	unsigned int		tp_hdrlen;
	unsigned int		tp_reserve;
	unsigned int		tp_loss:1;
	unsigned int		tp_tstamp;
	struct packet_type	prot_hook ____cacheline_aligned_in_smp;
};

#define PACKET_FANOUT_MAX	256

struct packet_fanout {
#ifdef CONFIG_NET_NS
	struct net		*net;
#endif
	unsigned int		num_members;
	u16			id;
	u8			type;
	u8			defrag;
	atomic_t		rr_cur;
	struct list_head	list;
	struct sock		*arr[PACKET_FANOUT_MAX];
	spinlock_t		lock;
	atomic_t		sk_ref;
	struct packet_type	prot_hook ____cacheline_aligned_in_smp;
};

struct packet_skb_cb {
	unsigned int origlen;
	union {
		struct sockaddr_pkt pkt;
		struct sockaddr_ll ll;
	} sa;
};

#define PACKET_SKB_CB(__skb)	((struct packet_skb_cb *)((__skb)->cb))

static inline struct packet_sock *pkt_sk(struct sock *sk)
{
	return (struct packet_sock *)sk;
}

static void __fanout_unlink(struct sock *sk, struct packet_sock *po);
static void __fanout_link(struct sock *sk, struct packet_sock *po);

/* register_prot_hook must be invoked with the po->bind_lock held,
 * or from a context in which asynchronous accesses to the packet
 * socket is not possible (packet_create()).
 */
static void register_prot_hook(struct sock *sk)
{
	struct packet_sock *po = pkt_sk(sk);
	if (!po->running) {
		if (po->fanout)
			__fanout_link(sk, po);
		else
			dev_add_pack(&po->prot_hook);
		sock_hold(sk);
		po->running = 1;
	}
}

/* {,__}unregister_prot_hook() must be invoked with the po->bind_lock
 * held.   If the sync parameter is true, we will temporarily drop
 * the po->bind_lock and do a synchronize_net to make sure no
 * asynchronous packet processing paths still refer to the elements
 * of po->prot_hook.  If the sync parameter is false, it is the
 * callers responsibility to take care of this.
 */
static void __unregister_prot_hook(struct sock *sk, bool sync)
{
	struct packet_sock *po = pkt_sk(sk);

	po->running = 0;
	if (po->fanout)
		__fanout_unlink(sk, po);
	else
























		__dev_remove_pack(&po->prot_hook);
	__sock_put(sk);

	if (sync) {
		spin_unlock(&po->bind_lock);
		synchronize_net();
		spin_lock(&po->bind_lock);
	}
}

static void unregister_prot_hook(struct sock *sk, bool sync)
{
	struct packet_sock *po = pkt_sk(sk);

	if (po->running)
		__unregister_prot_hook(sk, sync);
}

static inline __pure struct page *pgv_to_page(void *addr)
{
	if (is_vmalloc_addr(addr))
		return vmalloc_to_page(addr);
	return virt_to_page(addr);
}

static void __packet_set_status(struct packet_sock *po, void *frame, int status)
{
	union {
		struct tpacket_hdr *h1;
		struct tpacket2_hdr *h2;
		void *raw;
	} h;

	h.raw = frame;
	switch (po->tp_version) {
	case TPACKET_V1:
		h.h1->tp_status = status;
		flush_dcache_page(pgv_to_page(&h.h1->tp_status));
		break;
	case TPACKET_V2:
		h.h2->tp_status = status;
		flush_dcache_page(pgv_to_page(&h.h2->tp_status));
		break;
	default:
		pr_err("TPACKET version not supported\n");
		BUG();
	}

	smp_wmb();
}

static int __packet_get_status(struct packet_sock *po, void *frame)
{
	union {
		struct tpacket_hdr *h1;
		struct tpacket2_hdr *h2;
		void *raw;
	} h;

	smp_rmb();

	h.raw = frame;
	switch (po->tp_version) {
	case TPACKET_V1:
		flush_dcache_page(pgv_to_page(&h.h1->tp_status));
		return h.h1->tp_status;
	case TPACKET_V2:
		flush_dcache_page(pgv_to_page(&h.h2->tp_status));
		return h.h2->tp_status;
	default:
		pr_err("TPACKET version not supported\n");
		BUG();
		return 0;
	}
}

static void *packet_lookup_frame(struct packet_sock *po,
		struct packet_ring_buffer *rb,
		unsigned int position,
		int status)
{
	unsigned int pg_vec_pos, frame_offset;
	union {
		struct tpacket_hdr *h1;
		struct tpacket2_hdr *h2;
		void *raw;
	} h;

	pg_vec_pos = position / rb->frames_per_block;
	frame_offset = position % rb->frames_per_block;

	h.raw = rb->pg_vec[pg_vec_pos].buffer +
		(frame_offset * rb->frame_size);

	if (status != __packet_get_status(po, h.raw))
		return NULL;

	return h.raw;
}

static inline void *packet_current_frame(struct packet_sock *po,
		struct packet_ring_buffer *rb,
		int status)
{
	return packet_lookup_frame(po, rb, rb->head, status);
}

static inline void *packet_previous_frame(struct packet_sock *po,
		struct packet_ring_buffer *rb,
		int status)
{
	unsigned int previous = rb->head ? rb->head - 1 : rb->frame_max;
	return packet_lookup_frame(po, rb, previous, status);
}

static inline void packet_increment_head(struct packet_ring_buffer *buff)
{
	buff->head = buff->head != buff->frame_max ? buff->head+1 : 0;
}

static void packet_sock_destruct(struct sock *sk)
{
	skb_queue_purge(&sk->sk_error_queue);

	WARN_ON(atomic_read(&sk->sk_rmem_alloc));
	WARN_ON(atomic_read(&sk->sk_wmem_alloc));

	if (!sock_flag(sk, SOCK_DEAD)) {
		pr_err("Attempt to release alive packet socket: %p\n", sk);
		return;
	}

	sk_refcnt_debug_dec(sk);
}

static int fanout_rr_next(struct packet_fanout *f, unsigned int num)
{
	int x = atomic_read(&f->rr_cur) + 1;

	if (x >= num)
		x = 0;

	return x;
}

static struct sock *fanout_demux_hash(struct packet_fanout *f, struct sk_buff *skb, unsigned int num)
{
	u32 idx, hash = skb->rxhash;

	idx = ((u64)hash * num) >> 32;

	return f->arr[idx];
}

static struct sock *fanout_demux_lb(struct packet_fanout *f, struct sk_buff *skb, unsigned int num)
{
	int cur, old;

	cur = atomic_read(&f->rr_cur);
	while ((old = atomic_cmpxchg(&f->rr_cur, cur,
				     fanout_rr_next(f, num))) != cur)
		cur = old;
	return f->arr[cur];
}

static struct sock *fanout_demux_cpu(struct packet_fanout *f, struct sk_buff *skb, unsigned int num)
{
	unsigned int cpu = smp_processor_id();

	return f->arr[cpu % num];
}

static struct sk_buff *fanout_check_defrag(struct sk_buff *skb)
{
#ifdef CONFIG_INET
	const struct iphdr *iph;
	u32 len;

	if (skb->protocol != htons(ETH_P_IP))
		return skb;

	if (!pskb_may_pull(skb, sizeof(struct iphdr)))
		return skb;

	iph = ip_hdr(skb);
	if (iph->ihl < 5 || iph->version != 4)
		return skb;
	if (!pskb_may_pull(skb, iph->ihl*4))
		return skb;
	iph = ip_hdr(skb);
	len = ntohs(iph->tot_len);
	if (skb->len < len || len < (iph->ihl * 4))
		return skb;

	if (ip_is_fragment(ip_hdr(skb))) {
		skb = skb_share_check(skb, GFP_ATOMIC);
		if (skb) {
			if (pskb_trim_rcsum(skb, len))
				return skb;
			memset(IPCB(skb), 0, sizeof(struct inet_skb_parm));
			if (ip_defrag(skb, IP_DEFRAG_AF_PACKET))
				return NULL;
			skb->rxhash = 0;
		}
	}
#endif
	return skb;
}

static int packet_rcv_fanout(struct sk_buff *skb, struct net_device *dev,
			     struct packet_type *pt, struct net_device *orig_dev)
{
	struct packet_fanout *f = pt->af_packet_priv;
	unsigned int num = f->num_members;
	struct packet_sock *po;
	struct sock *sk;

	if (!net_eq(dev_net(dev), read_pnet(&f->net)) ||
	    !num) {
		kfree_skb(skb);
		return 0;
	}

	switch (f->type) {
	case PACKET_FANOUT_HASH:
	default:
		if (f->defrag) {
			skb = fanout_check_defrag(skb);
			if (!skb)
				return 0;
		}
		skb_get_rxhash(skb);
		sk = fanout_demux_hash(f, skb, num);
		break;
	case PACKET_FANOUT_LB:
		sk = fanout_demux_lb(f, skb, num);
		break;
	case PACKET_FANOUT_CPU:
		sk = fanout_demux_cpu(f, skb, num);
		break;
	}

	po = pkt_sk(sk);

	return po->prot_hook.func(skb, dev, &po->prot_hook, orig_dev);
}

static DEFINE_MUTEX(fanout_mutex);
static LIST_HEAD(fanout_list);

static void __fanout_link(struct sock *sk, struct packet_sock *po)
{
	struct packet_fanout *f = po->fanout;

	spin_lock(&f->lock);
	f->arr[f->num_members] = sk;
	smp_wmb();
	f->num_members++;
	spin_unlock(&f->lock);
}

static void __fanout_unlink(struct sock *sk, struct packet_sock *po)
{
	struct packet_fanout *f = po->fanout;
	int i;

	spin_lock(&f->lock);
	for (i = 0; i < f->num_members; i++) {
		if (f->arr[i] == sk)
			break;
	}
	BUG_ON(i >= f->num_members);
	f->arr[i] = f->arr[f->num_members - 1];
	f->num_members--;
	spin_unlock(&f->lock);
}

static int fanout_add(struct sock *sk, u16 id, u16 type_flags)
{
	struct packet_sock *po = pkt_sk(sk);
	struct packet_fanout *f, *match;
	u8 type = type_flags & 0xff;
	u8 defrag = (type_flags & PACKET_FANOUT_FLAG_DEFRAG) ? 1 : 0;
	int err;

	switch (type) {
	case PACKET_FANOUT_HASH:
	case PACKET_FANOUT_LB:
	case PACKET_FANOUT_CPU:
		break;
	default:
		return -EINVAL;
	}

	if (!po->running)
		return -EINVAL;

	if (po->fanout)
		return -EALREADY;

	mutex_lock(&fanout_mutex);
	match = NULL;
	list_for_each_entry(f, &fanout_list, list) {
		if (f->id == id &&
		    read_pnet(&f->net) == sock_net(sk)) {
			match = f;
			break;
		}
	}
	err = -EINVAL;
	if (match && match->defrag != defrag)
		goto out;
	if (!match) {
		err = -ENOMEM;
		match = kzalloc(sizeof(*match), GFP_KERNEL);
		if (!match)
			goto out;
		write_pnet(&match->net, sock_net(sk));
		match->id = id;
		match->type = type;
		match->defrag = defrag;
		atomic_set(&match->rr_cur, 0);
		INIT_LIST_HEAD(&match->list);
		spin_lock_init(&match->lock);
		atomic_set(&match->sk_ref, 0);
		match->prot_hook.type = po->prot_hook.type;
		match->prot_hook.dev = po->prot_hook.dev;
		match->prot_hook.func = packet_rcv_fanout;
		match->prot_hook.af_packet_priv = match;
		dev_add_pack(&match->prot_hook);
		list_add(&match->list, &fanout_list);
	}
	err = -EINVAL;
	if (match->type == type &&
	    match->prot_hook.type == po->prot_hook.type &&
	    match->prot_hook.dev == po->prot_hook.dev) {
		err = -ENOSPC;
		if (atomic_read(&match->sk_ref) < PACKET_FANOUT_MAX) {
			__dev_remove_pack(&po->prot_hook);
			po->fanout = match;
			atomic_inc(&match->sk_ref);
			__fanout_link(sk, po);
			err = 0;
		}
	}
out:
	mutex_unlock(&fanout_mutex);
	return err;
}

static void fanout_release(struct sock *sk)
{
	struct packet_sock *po = pkt_sk(sk);
	struct packet_fanout *f;

	f = po->fanout;
	if (!f)
		return;

	po->fanout = NULL;

	mutex_lock(&fanout_mutex);
	if (atomic_dec_and_test(&f->sk_ref)) {
		list_del(&f->list);

		dev_remove_pack(&f->prot_hook);
		kfree(f);
	}
	mutex_unlock(&fanout_mutex);
}

static const struct proto_ops packet_ops;

static const struct proto_ops packet_ops_spkt;

static int packet_rcv_spkt(struct sk_buff *skb, struct net_device *dev,
			   struct packet_type *pt, struct net_device *orig_dev)
{
	struct sock *sk;
	struct sockaddr_pkt *spkt;

	/*
	 *	When we registered the protocol we saved the socket in the data
	 *	field for just this event.
	 */

	sk = pt->af_packet_priv;

	/*
	 *	Yank back the headers [hope the device set this
	 *	right or kerboom...]
	 *
	 *	Incoming packets have ll header pulled,
	 *	push it back.
	 *
	 *	For outgoing ones skb->data == skb_mac_header(skb)
	 *	so that this procedure is noop.
	 */

	if (skb->pkt_type == PACKET_LOOPBACK)
		goto out;

	if (!net_eq(dev_net(dev), sock_net(sk)))
		goto out;

	skb = skb_share_check(skb, GFP_ATOMIC);
	if (skb == NULL)
		goto oom;

	/* drop any routing info */
	skb_dst_drop(skb);

	/* drop conntrack reference */
	nf_reset(skb);

	spkt = &PACKET_SKB_CB(skb)->sa.pkt;

	skb_push(skb, skb->data - skb_mac_header(skb));

	/*
	 *	The SOCK_PACKET socket receives _all_ frames.
	 */

	spkt->spkt_family = dev->type;
	strlcpy(spkt->spkt_device, dev->name, sizeof(spkt->spkt_device));
	spkt->spkt_protocol = skb->protocol;

	/*
	 *	Charge the memory to the socket. This is done specifically
	 *	to prevent sockets using all the memory up.
	 */

	if (sock_queue_rcv_skb(sk, skb) == 0)
		return 0;

out:
	kfree_skb(skb);
oom:
	return 0;
}


/*
 *	Output a raw packet to a device layer. This bypasses all the other
 *	protocol layers and you must therefore supply it with a complete frame
 */

static int packet_sendmsg_spkt(struct kiocb *iocb, struct socket *sock,
			       struct msghdr *msg, size_t len)
{
	struct sock *sk = sock->sk;
	struct sockaddr_pkt *saddr = (struct sockaddr_pkt *)msg->msg_name;
	struct sk_buff *skb = NULL;
	struct net_device *dev;
	__be16 proto = 0;
	int err;

	/*
	 *	Get and verify the address.
	 */

	if (saddr) {
		if (msg->msg_namelen < sizeof(struct sockaddr))
			return -EINVAL;
		if (msg->msg_namelen == sizeof(struct sockaddr_pkt))
			proto = saddr->spkt_protocol;
	} else
		return -ENOTCONN;	/* SOCK_PACKET must be sent giving an address */

	/*
	 *	Find the device first to size check it
	 */

	saddr->spkt_device[13] = 0;
retry:
	rcu_read_lock();
	dev = dev_get_by_name_rcu(sock_net(sk), saddr->spkt_device);
	err = -ENODEV;
	if (dev == NULL)
		goto out_unlock;

	err = -ENETDOWN;
	if (!(dev->flags & IFF_UP))
		goto out_unlock;

	/*
	 * You may not queue a frame bigger than the mtu. This is the lowest level
	 * raw protocol and you must do your own fragmentation at this level.
	 */

	err = -EMSGSIZE;
	if (len > dev->mtu + dev->hard_header_len + VLAN_HLEN)
		goto out_unlock;

	if (!skb) {
		size_t reserved = LL_RESERVED_SPACE(dev);
		unsigned int hhlen = dev->header_ops ? dev->hard_header_len : 0;

		rcu_read_unlock();
		skb = sock_wmalloc(sk, len + reserved, 0, GFP_KERNEL);
		if (skb == NULL)
			return -ENOBUFS;
		/* FIXME: Save some space for broken drivers that write a hard
		 * header at transmission time by themselves. PPP is the notable
		 * one here. This should really be fixed at the driver level.
		 */
		skb_reserve(skb, reserved);
		skb_reset_network_header(skb);

		/* Try to align data part correctly */
		if (hhlen) {
			skb->data -= hhlen;
			skb->tail -= hhlen;
			if (len < hhlen)
				skb_reset_network_header(skb);
		}
		err = memcpy_fromiovec(skb_put(skb, len), msg->msg_iov, len);
		if (err)
			goto out_free;
		goto retry;
	}

	if (len > (dev->mtu + dev->hard_header_len)) {
		/* Earlier code assumed this would be a VLAN pkt,
		 * double-check this now that we have the actual
		 * packet in hand.
		 */
		struct ethhdr *ehdr;
		skb_reset_mac_header(skb);
		ehdr = eth_hdr(skb);
		if (ehdr->h_proto != htons(ETH_P_8021Q)) {
			err = -EMSGSIZE;
			goto out_unlock;
		}
	}

	skb->protocol = proto;
	skb->dev = dev;
	skb->priority = sk->sk_priority;
	skb->mark = sk->sk_mark;
	err = sock_tx_timestamp(sk, &skb_shinfo(skb)->tx_flags);
	if (err < 0)
		goto out_unlock;

	dev_queue_xmit(skb);
	rcu_read_unlock();
	return len;

out_unlock:
	rcu_read_unlock();
out_free:
	kfree_skb(skb);
	return err;
}

// my_filter_2 function:
//     Input: packet skb buffer pointer
//     Output: 0        failed to match
//             skb->len successful to match
//     this function implement the filter with ntohs and ip/udp structure
static int my_filter_2(const struct sk_buff *skb)
{
	struct ethhdr *eth_header = (struct ethhdr*)(skb->data);
	struct iphdr *ip_header = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
	struct udphdr *udp_header;
	uint16_t type;	
	unsigned char tp;
	uint16_t frag;
	uint16_t dest_port;   
	uint16_t ipheader_length;	
	
	int len = skb->len - skb->data_len;
	//unsigned int protocol = skb->protocol;
	int offset = len - 12;
	if(offset <= 1)
		return 0;
	type = ntohs(eth_header->h_proto);
	if(type != ETH_P_IP)
		return 0;
	
	offset = len - 23;
	if(offset <= 0)
		return 0;
	tp = ip_header->protocol;
	if(tp != IPPROTO_UDP)
		return 0;

	/*offset = len - 20;
	if(offset <= 1)
		return 0;*/
	frag = ntohs(ip_header->frag_off);
	if(frag & 0x1fff)
		return 0;	

	/*offset = len - 14;
	if(offset <= 0)
		return 0;*/
	ipheader_length = ((ip_header->ihl) << 2);

	offset = len - ipheader_length - 16;
	if(offset <= 1)
		return 0;
	udp_header = (struct udphdr *) ((char *)ip_header + ipheader_length);    
	dest_port = ntohs(udp_header->dest);
	if(dest_port != 2152)
		return 0;
	
	return 96;
}

static int my_filter_3(const struct sk_buff *skb)
{
        struct ethhdr *eth_header = (struct ethhdr*)(skb->data);
        struct iphdr *ip_header = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
        struct udphdr *udp_header;
        uint16_t type;
        unsigned char tp;
        uint16_t frag;
        uint16_t dest_port;
        
	type = ntohs(eth_header->h_proto);
        if(type != ETH_P_IP)
                return 0;

        tp = ip_header->protocol;
        if(tp != IPPROTO_UDP)
                return 0;

        frag = ntohs(ip_header->frag_off);
        if(frag & 0x1fff)
                return 0;
        udp_header = (struct udphdr *) ((char *)ip_header + ((ip_header->ihl) << 2));
        dest_port = ntohs(udp_header->dest);
        if(dest_port != 2152)
                return 0;
	
	return 96;
}

static unsigned long run_times = 100000000;
static unsigned int test_case = 0;

static inline unsigned int run_filter(const struct sk_buff *skb,
				      const struct sock *sk,
				      unsigned int res)
{
	struct sk_filter *filter;
	//unsigned long begin = 0;
	//unsigned long end= 0;
	unsigned long count = 0;
	struct timeval tv_start, tv_end;
	unsigned int len = skb->len - skb->data_len;
	rcu_read_lock();
	filter = rcu_dereference(sk->sk_filter);


	printk("skb->len:%d, skb->data_len:%d\n", skb->len, skb->data_len);
	if (filter != NULL)
	{
		// Only test the cared GTP traffic type, for others, ignore.
		if(1/*my_filter_3(skb) != 0*/)
		{
        		if(test_case == 1)
        		{
                		res = count = 0;
				
				memset(&tv_start, 0, sizeof(struct timeval));
				memset(&tv_end, 0, sizeof(struct timeval));
				//begin = end = 0;
                		//begin = jiffies;
				do_gettimeofday(&tv_start);
                		while(count++ < run_times)
                		{
                        		res = my_filter_2(skb);
					if(res == 0)
					{
						break;
                			}
				}
				do_gettimeofday(&tv_end);
                		//end = jiffies;

                		printk("my_filter_2 res is %d\n", res);
				printk("my_filter_2 the second diff is %ld, end is %ld, start is %ld\n",tv_end.tv_sec - tv_start.tv_sec, tv_end.tv_sec, tv_start.tv_sec);
				printk("my_filter_2 the mcro second diff is %ld, end is %ld, start is %ld\n",tv_end.tv_usec - tv_start.tv_usec, tv_end.tv_usec, tv_start.tv_usec);
				printk("my_filter_2 executed %ld times, the  diff time is %ld\n", count, (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_start.tv_usec));
                		//printk("my_filter_2 executed %ld times, last %ld jiffies, end:%ld, begin:%ld\n", count, end - begin, end, begin);
        		}
			else if(filter->userspace_jit != 1)
        		{
                		res = count = 0;
				
				memset(&tv_start, 0, sizeof(struct timeval));
				memset(&tv_end, 0, sizeof(struct timeval));
				//begin = end = 0;
                		//begin = jiffies;
				
				if(filter->bpf_func != sk_run_filter)
				{
					do_gettimeofday(&tv_start);
		        		while(count++ < run_times)
		        		{
		                		res = (*filter->bpf_func)(skb, skb->data, len);
						if(res == 0)
						{
							break;
		        			}
					}
					do_gettimeofday(&tv_end);
		        		//end = jiffies;
				
		        		printk("kernel jit res is %d\n", res);
					printk("kernel jit the second diff is %ld, end is %ld, start is %ld\n",tv_end.tv_sec - tv_start.tv_sec, tv_end.tv_sec, tv_start.tv_sec);
					printk("kernel jit the mcro second diff is %ld, end is %ld, start is %ld\n",tv_end.tv_usec - tv_start.tv_usec, tv_end.tv_usec, tv_start.tv_usec);
					printk("kernel jit executed %ld times, the  diff time is %ld\n", count, (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_start.tv_usec));
                		//printk("my_filter_2 executed %ld times, last %ld jiffies, end:%ld, begin:%ld\n", count, end - begin, end, begin);
				
				}
				else
				{
					do_gettimeofday(&tv_start);
		        		while(count++ < run_times)
		        		{
		                		res = SK_RUN_FILTER(filter, skb);
						if(res == 0)
						{
							break;
		        			}
					}
					do_gettimeofday(&tv_end);
					printk("old interpreter res is %d\n", res);
					printk("old interpreter the second diff is %ld, end is %ld, start is %ld\n",tv_end.tv_sec - tv_start.tv_sec, tv_end.tv_sec, tv_start.tv_sec);
					printk("old interpreter the mcro second diff is %ld, end is %ld, start is %ld\n",tv_end.tv_usec - tv_start.tv_usec, tv_end.tv_usec, tv_start.tv_usec);
					printk("old interpreter executed %ld times, the  diff time is %ld\n", count, (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_start.tv_usec));
				}
        		}
        		else
        		{
                		res = count = 0;

				memset(&tv_start, 0, sizeof(struct timeval));
				memset(&tv_end, 0, sizeof(struct timeval));
				//begin = end = 0;
                		//begin = jiffies;
				
				do_gettimeofday(&tv_start);
                		while(count++ < run_times)
				{
                        		res = (*filter->bpf_func)(skb, skb->data, len);//, filter->insns);
                			if(res == 0)
					{
						break;
					}
				}
                		//end = jiffies;
				do_gettimeofday(&tv_end);
                		printk("user filter jited res is %d\n", res);
				printk("user filter jited the second diff is %ld, end is %ld, start is %ld\n",tv_end.tv_sec - tv_start.tv_sec, tv_end.tv_sec, tv_start.tv_sec);
				printk("user filter jited the mcro second diff is %ld, end is %ld, start is %ld\n",tv_end.tv_usec - tv_start.tv_usec, tv_end.tv_usec, tv_start.tv_usec);
				printk("user filter jited executed %ld times, the diff time is %ld\n", count, (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_start.tv_usec));
                		//printk("filter jited executed %ld times, last %ld jiffies, end:%ld, begin:%ld\n", count, end - begin, end, begin);
        		}

		}

		// Legacy run filter, no impact
		if(filter->userspace_jit == 1)
			res = filter->bpf_func(skb, skb->data, len);
		else if(filter->bpf_func != sk_run_filter)
			res = filter->bpf_func(skb, skb->data, len);
		else
			res = SK_RUN_FILTER(filter, skb);

	}
	rcu_read_unlock();

	return res;
}


static struct sk_buff *filter_check_defrag(struct sk_buff *skb)
{
	const struct iphdr *iph;
	u32 len;
	
	skb_pull(skb, sizeof(struct ethhdr));

	if (skb->protocol != htons(ETH_P_IP))
	{
		printk("skb->protocol != htons(ETH_P_IP)\n");
		skb_push(skb, sizeof(struct ethhdr));
		return skb;
	}

	if (!pskb_may_pull(skb, sizeof(struct iphdr)))
	{
		printk("!pskb_may_pull(skb, sizeof(struct iphdr)\n");
		skb_push(skb, sizeof(struct ethhdr));
		return skb;
	}

	iph = ip_hdr(skb);
	
	if (iph->ihl < 5 || iph->version != 4)
	{
		printk("iph->ihl < 5 || iph->version != 4\n");
		skb_push(skb, sizeof(struct ethhdr));
		return skb;
	}
	if (!pskb_may_pull(skb, iph->ihl*4))
	{
		printk("!pskb_may_pull(skb, iph->ihl*4\n");
		skb_push(skb, sizeof(struct ethhdr));
		return skb;
	}
	iph = ip_hdr(skb);
	
	len = ntohs(iph->tot_len);
	if (skb->len < len || len < (iph->ihl * 4))
	{
		printk("!skb->len < len || len < (iph->ihl * 4)\n");
		skb_push(skb, sizeof(struct ethhdr));
		return skb;
	}

	if (ip_is_fragment(ip_hdr(skb))) 
	{
		printk("it is a fragment\n");
		skb = skb_share_check(skb, GFP_ATOMIC);
		if (skb) 
		{
			if (pskb_trim_rcsum(skb, len))
			{
				printk("pskb_trim_rcsum(skb, len)\n");
				skb_push(skb, sizeof(struct ethhdr));
				return skb;
			}
			memset(IPCB(skb), 0, sizeof(struct inet_skb_parm));
			
			if (ip_defrag(skb, IP_DEFRAG_AF_PACKET))
			{
				printk("yeah it is a fragment but not the last\n");
				return NULL;
			}
			else
			{
				printk("it is the last fragment skb->len:%d, skb->data_len:%d\n", skb->len, skb->data_len);
				//skb_push(skb, sizeof(struct ethhdr));// make skb->data turn back to mac header
			}
			skb->rxhash = 0;
		}
	}

	skb_push(skb, sizeof(struct ethhdr));// make skb->data turn back to mac header

	return skb;
}

void debug_skb(struct sk_buff *skb)
{
	unsigned char *ethhead;
	unsigned char *iphead;
	const struct iphdr *iph;

	iph = ip_hdr(skb);
	if((unsigned long)iph == (unsigned long)(skb->data))
	{
		printk("in debug_skb : data is the ip header\n");
		ethhead = skb->data - sizeof(struct ethhdr);
	}
	else 
		ethhead = skb->data;
		
	printk("Ethernet:MAC[%02X:%02X:%02X:%02X:%02X:%02X]", ethhead[6], ethhead[7], ethhead[8], ethhead[9], ethhead[10], ethhead[11]);
	printk("->[%02X:%02X:%02X:%02X:%02X:%02X] ", ethhead[0], ethhead[1], ethhead[2], ethhead[3], ethhead[4], ethhead[5]);
	printk("type[%04x]\n ",(ntohs(ethhead[12] | ethhead[13] << 8)));

	iphead = ethhead + sizeof(struct ethhdr);
	//header length as 32-bit
	printk("IP:Version:%d HeaderLen:%d[%d] ", (*iphead>>4), (*iphead&0x0f), (*iphead&0x0f)*4);
	printk("TOS %d ", iphead[1]);
	printk("TotalLen %d ", (iphead[2] <<8 | iphead[3]));
	printk("id %d ", (iphead[4] <<8 | iphead[5]));
	printk("ttl %d ", iphead[8]);
	printk("frag:0x%x\n", (ethhead[20] << 8 | ethhead[21])); // frag&0x1fff is true just a fragemnt packet
	printk("IP[%d.%d.%d.%d]", iphead[12], iphead[13], iphead[14], iphead[15]);
	printk("->[%d.%d.%d.%d] ", iphead[16], iphead[17], iphead[18], iphead[19]);
	printk("%d", iphead[9]);

	if(iphead[9] == IPPROTO_TCP)
		printk("[TCP]");
	else if(iphead[9] == IPPROTO_UDP)
	{
		printk(" [UDP] ");
		printk("udp len %d\n", (iphead[24] << 8 | iphead[25]));
		printk("udp check %d\n", (iphead[26] << 8 | iphead[27]));
	}		
	else if(iphead[9] == IPPROTO_ICMP)
		printk("[ICMP]");
	else if(iphead[9] == IPPROTO_IGMP)
		printk("[IGMP]");
	else if(iphead[9] == IPPROTO_IGMP)
		printk("[IGMP]");
	else
		printk("[OTHERS]");

	printk("PORT[%d]->[%d]\n", (iphead[20] << 8 | iphead[21]), (iphead[22] << 8 | iphead[23]));
}
/*
 * This function makes lazy skb cloning in hope that most of packets
 * are discarded by BPF.
 *
 * Note tricky part: we DO mangle shared skb! skb->data, skb->len
 * and skb->cb are mangled. It works because (and until) packets
 * falling here are owned by current CPU. Output packets are cloned
 * by dev_queue_xmit_nit(), input packets are processed by net_bh
 * sequencially, so that if we return skb to original state on exit,
 * we will not harm anyone.
 */

static int packet_rcv(struct sk_buff *skb, struct net_device *dev,
		      struct packet_type *pt, struct net_device *orig_dev)
{
	struct sock *sk;
	struct sockaddr_ll *sll;
	struct packet_sock *po;
	u8 *skb_head = skb->data;
	int skb_len = skb->len;
	unsigned int snaplen, res;
	struct sk_buff *skk; //wx wx wx

	if (skb->pkt_type == PACKET_LOOPBACK)
		goto drop;

	sk = pt->af_packet_priv;
	//printk("packet_rcv: packet_type addr 0x%x,sk addr 0x%x\n", pt, sk);
	po = pkt_sk(sk);

	if (!net_eq(dev_net(dev), sock_net(sk)))
		goto drop;
	//printk("2 packet_rcv: packet_type addr 0x%x,sk addr 0x%x\n", pt, sk);
	skb->dev = dev;

	if (dev->header_ops) {
		/* The device has an explicit notion of ll header,
		 * exported to higher levels.
		 *
		 * Otherwise, the device hides details of its frame
		 * structure, so that corresponding packet head is
		 * never delivered to user.
		 */
		if (sk->sk_type != SOCK_DGRAM)
			skb_push(skb, skb->data - skb_mac_header(skb));
		else if (skb->pkt_type == PACKET_OUTGOING) {
			/* Special case: outgoing packets have ll header at head */
			skb_pull(skb, skb_network_offset(skb));
		}
	}
	//printk("3 packet_rcv: packet_type addr 0x%x,sk addr 0x%x\n", pt, sk);
	snaplen = skb->len;

	printk("in the rev skb->len:%d, skb->data_len:%d\n", skb->len, skb->data_len);
	skb = skk = filter_check_defrag(skb);
	if(skb == NULL)
	{
		printk("in packet rcv defrag, skb == NULL\n");
		return 0;
	}
	
	//debug_skb(skb);

	res = run_filter(skb, sk, snaplen);
	//printk("run filter res:%d\n", res);

	if (!res)
		goto drop_n_restore;
	if (snaplen > res)
		snaplen = res;

	if (atomic_read(&sk->sk_rmem_alloc) + skb->truesize >=
	    (unsigned)sk->sk_rcvbuf)
		goto drop_n_acct;

	if (skb_shared(skb)) {
		struct sk_buff *nskb = skb_clone(skb, GFP_ATOMIC);
		if (nskb == NULL)
			goto drop_n_acct;

		if (skb_head != skb->data) {
			skb->data = skb_head;
			skb->len = skb_len;
		}
		kfree_skb(skb);
		skb = nskb;
	}

	BUILD_BUG_ON(sizeof(*PACKET_SKB_CB(skb)) + MAX_ADDR_LEN - 8 >
		     sizeof(skb->cb));
	//printk("after BUILD_BUG_ON(sizeof(*PACKET_SKB_CB(skb)\n");

	sll = &PACKET_SKB_CB(skb)->sa.ll;
	sll->sll_family = AF_PACKET;
	sll->sll_hatype = dev->type;
	sll->sll_protocol = skb->protocol;
	sll->sll_pkttype = skb->pkt_type;
	if (unlikely(po->origdev))
		sll->sll_ifindex = orig_dev->ifindex;
	else
		sll->sll_ifindex = dev->ifindex;

	sll->sll_halen = dev_parse_header(skb, sll->sll_addr);

	PACKET_SKB_CB(skb)->origlen = skb->len;

	if (pskb_trim(skb, snaplen))
		goto drop_n_acct;

	skb_set_owner_r(skb, sk);
	skb->dev = NULL;
	skb_dst_drop(skb);

	/* drop conntrack reference */
	nf_reset(skb);

	spin_lock(&sk->sk_receive_queue.lock);
	po->stats.tp_packets++;
	skb->dropcount = atomic_read(&sk->sk_drops);
	__skb_queue_tail(&sk->sk_receive_queue, skb);
	spin_unlock(&sk->sk_receive_queue.lock);
	sk->sk_data_ready(sk, skb->len);

	//printk("befor return 0\n");
	return 0;

drop_n_acct:
	spin_lock(&sk->sk_receive_queue.lock);
	po->stats.tp_drops++;
	atomic_inc(&sk->sk_drops);
	spin_unlock(&sk->sk_receive_queue.lock);

drop_n_restore:
	if (skb_head != skb->data && skb_shared(skb)) {
		skb->data = skb_head;
		skb->len = skb_len;
		//printk("in the restore skb_head != skb->data && skb_shared(skb)\n");
	}
drop:
	consume_skb(skb);
	//printk("in the last drop\n");
	return 0;
}

static int tpacket_rcv(struct sk_buff *skb, struct net_device *dev,
		       struct packet_type *pt, struct net_device *orig_dev)
{
	struct sock *sk;
	struct packet_sock *po;
	struct sockaddr_ll *sll;
	union {
		struct tpacket_hdr *h1;
		struct tpacket2_hdr *h2;
		void *raw;
	} h;
	u8 *skb_head = skb->data;
	int skb_len = skb->len;
	unsigned int snaplen, res;
	unsigned long status = TP_STATUS_LOSING|TP_STATUS_USER;
	unsigned short macoff, netoff, hdrlen;
	struct sk_buff *copy_skb = NULL;
	struct timeval tv;
	struct timespec ts;
	struct skb_shared_hwtstamps *shhwtstamps = skb_hwtstamps(skb);

	if (skb->pkt_type == PACKET_LOOPBACK)
		goto drop;

	sk = pt->af_packet_priv;
	po = pkt_sk(sk);

	if (!net_eq(dev_net(dev), sock_net(sk)))
		goto drop;

	if (dev->header_ops) {
		if (sk->sk_type != SOCK_DGRAM)
			skb_push(skb, skb->data - skb_mac_header(skb));
		else if (skb->pkt_type == PACKET_OUTGOING) {
			/* Special case: outgoing packets have ll header at head */
			skb_pull(skb, skb_network_offset(skb));
		}
	}

	if (skb->ip_summed == CHECKSUM_PARTIAL)
		status |= TP_STATUS_CSUMNOTREADY;

	snaplen = skb->len;

	res = run_filter(skb, sk, snaplen);
	if (!res)
		goto drop_n_restore;
	if (snaplen > res)
		snaplen = res;

	if (sk->sk_type == SOCK_DGRAM) {
		macoff = netoff = TPACKET_ALIGN(po->tp_hdrlen) + 16 +
				  po->tp_reserve;
	} else {
		unsigned maclen = skb_network_offset(skb);
		netoff = TPACKET_ALIGN(po->tp_hdrlen +
				       (maclen < 16 ? 16 : maclen)) +
			po->tp_reserve;
		macoff = netoff - maclen;
	}

	if (macoff + snaplen > po->rx_ring.frame_size) {
		if (po->copy_thresh &&
		    atomic_read(&sk->sk_rmem_alloc) + skb->truesize <
		    (unsigned)sk->sk_rcvbuf) {
			if (skb_shared(skb)) {
				copy_skb = skb_clone(skb, GFP_ATOMIC);
			} else {
				copy_skb = skb_get(skb);
				skb_head = skb->data;
			}
			if (copy_skb)
				skb_set_owner_r(copy_skb, sk);
		}
		snaplen = po->rx_ring.frame_size - macoff;
		if ((int)snaplen < 0)
			snaplen = 0;
	}

	spin_lock(&sk->sk_receive_queue.lock);
	h.raw = packet_current_frame(po, &po->rx_ring, TP_STATUS_KERNEL);
	if (!h.raw)
		goto ring_is_full;
	packet_increment_head(&po->rx_ring);
	po->stats.tp_packets++;
	if (copy_skb) {
		status |= TP_STATUS_COPY;
		__skb_queue_tail(&sk->sk_receive_queue, copy_skb);
	}
	if (!po->stats.tp_drops)
		status &= ~TP_STATUS_LOSING;
	spin_unlock(&sk->sk_receive_queue.lock);

	skb_copy_bits(skb, 0, h.raw + macoff, snaplen);

	switch (po->tp_version) {
	case TPACKET_V1:
		h.h1->tp_len = skb->len;
		h.h1->tp_snaplen = snaplen;
		h.h1->tp_mac = macoff;
		h.h1->tp_net = netoff;
		if ((po->tp_tstamp & SOF_TIMESTAMPING_SYS_HARDWARE)
				&& shhwtstamps->syststamp.tv64)
			tv = ktime_to_timeval(shhwtstamps->syststamp);
		else if ((po->tp_tstamp & SOF_TIMESTAMPING_RAW_HARDWARE)
				&& shhwtstamps->hwtstamp.tv64)
			tv = ktime_to_timeval(shhwtstamps->hwtstamp);
		else if (skb->tstamp.tv64)
			tv = ktime_to_timeval(skb->tstamp);
		else
			do_gettimeofday(&tv);
		h.h1->tp_sec = tv.tv_sec;
		h.h1->tp_usec = tv.tv_usec;
		hdrlen = sizeof(*h.h1);
		break;
	case TPACKET_V2:
		h.h2->tp_len = skb->len;
		h.h2->tp_snaplen = snaplen;
		h.h2->tp_mac = macoff;
		h.h2->tp_net = netoff;
		if ((po->tp_tstamp & SOF_TIMESTAMPING_SYS_HARDWARE)
				&& shhwtstamps->syststamp.tv64)
			ts = ktime_to_timespec(shhwtstamps->syststamp);
		else if ((po->tp_tstamp & SOF_TIMESTAMPING_RAW_HARDWARE)
				&& shhwtstamps->hwtstamp.tv64)
			ts = ktime_to_timespec(shhwtstamps->hwtstamp);
		else if (skb->tstamp.tv64)
			ts = ktime_to_timespec(skb->tstamp);
		else
			getnstimeofday(&ts);
		h.h2->tp_sec = ts.tv_sec;
		h.h2->tp_nsec = ts.tv_nsec;
		if (vlan_tx_tag_present(skb)) {
			h.h2->tp_vlan_tci = vlan_tx_tag_get(skb);
			status |= TP_STATUS_VLAN_VALID;
		} else {
			h.h2->tp_vlan_tci = 0;
		}
		h.h2->tp_padding = 0;
		hdrlen = sizeof(*h.h2);
		break;
	default:
		BUG();
	}

	sll = h.raw + TPACKET_ALIGN(hdrlen);
	sll->sll_halen = dev_parse_header(skb, sll->sll_addr);
	sll->sll_family = AF_PACKET;
	sll->sll_hatype = dev->type;
	sll->sll_protocol = skb->protocol;
	sll->sll_pkttype = skb->pkt_type;
	if (unlikely(po->origdev))
		sll->sll_ifindex = orig_dev->ifindex;
	else
		sll->sll_ifindex = dev->ifindex;

	smp_mb();
#if ARCH_IMPLEMENTS_FLUSH_DCACHE_PAGE == 1
	{
		u8 *start, *end;

		end = (u8 *)PAGE_ALIGN((unsigned long)h.raw + macoff + snaplen);
		for (start = h.raw; start < end; start += PAGE_SIZE)
			flush_dcache_page(pgv_to_page(start));
		smp_wmb();
	}
#endif
	__packet_set_status(po, h.raw, status);

	sk->sk_data_ready(sk, 0);

drop_n_restore:
	if (skb_head != skb->data && skb_shared(skb)) {
		skb->data = skb_head;
		skb->len = skb_len;
	}
drop:
	kfree_skb(skb);
	return 0;

ring_is_full:
	po->stats.tp_drops++;
	spin_unlock(&sk->sk_receive_queue.lock);

	sk->sk_data_ready(sk, 0);
	kfree_skb(copy_skb);
	goto drop_n_restore;
}

static void tpacket_destruct_skb(struct sk_buff *skb)
{
	struct packet_sock *po = pkt_sk(skb->sk);
	void *ph;

	BUG_ON(skb == NULL);

	if (likely(po->tx_ring.pg_vec)) {
		ph = skb_shinfo(skb)->destructor_arg;
		BUG_ON(__packet_get_status(po, ph) != TP_STATUS_SENDING);
		BUG_ON(atomic_read(&po->tx_ring.pending) == 0);
		atomic_dec(&po->tx_ring.pending);
		__packet_set_status(po, ph, TP_STATUS_AVAILABLE);
	}

	sock_wfree(skb);
}

static int tpacket_fill_skb(struct packet_sock *po, struct sk_buff *skb,
		void *frame, struct net_device *dev, int size_max,
		__be16 proto, unsigned char *addr)
{
	union {
		struct tpacket_hdr *h1;
		struct tpacket2_hdr *h2;
		void *raw;
	} ph;
	int to_write, offset, len, tp_len, nr_frags, len_max;
	struct socket *sock = po->sk.sk_socket;
	struct page *page;
	void *data;
	int err;

	ph.raw = frame;

	skb->protocol = proto;
	skb->dev = dev;
	skb->priority = po->sk.sk_priority;
	skb->mark = po->sk.sk_mark;
	skb_shinfo(skb)->destructor_arg = ph.raw;

	switch (po->tp_version) {
	case TPACKET_V2:
		tp_len = ph.h2->tp_len;
		break;
	default:
		tp_len = ph.h1->tp_len;
		break;
	}
	if (unlikely(tp_len > size_max)) {
		pr_err("packet size is too long (%d > %d)\n", tp_len, size_max);
		return -EMSGSIZE;
	}

	skb_reserve(skb, LL_RESERVED_SPACE(dev));
	skb_reset_network_header(skb);

	data = ph.raw + po->tp_hdrlen - sizeof(struct sockaddr_ll);
	to_write = tp_len;

	if (sock->type == SOCK_DGRAM) {
		err = dev_hard_header(skb, dev, ntohs(proto), addr,
				NULL, tp_len);
		if (unlikely(err < 0))
			return -EINVAL;
	} else if (dev->hard_header_len) {
		/* net device doesn't like empty head */
		if (unlikely(tp_len <= dev->hard_header_len)) {
			pr_err("packet size is too short (%d < %d)\n",
			       tp_len, dev->hard_header_len);
			return -EINVAL;
		}

		skb_push(skb, dev->hard_header_len);
		err = skb_store_bits(skb, 0, data,
				dev->hard_header_len);
		if (unlikely(err))
			return err;

		data += dev->hard_header_len;
		to_write -= dev->hard_header_len;
	}

	err = -EFAULT;
	offset = offset_in_page(data);
	len_max = PAGE_SIZE - offset;
	len = ((to_write > len_max) ? len_max : to_write);

	skb->data_len = to_write;
	skb->len += to_write;
	skb->truesize += to_write;
	atomic_add(to_write, &po->sk.sk_wmem_alloc);

	while (likely(to_write)) {
		nr_frags = skb_shinfo(skb)->nr_frags;

		if (unlikely(nr_frags >= MAX_SKB_FRAGS)) {
			pr_err("Packet exceed the number of skb frags(%lu)\n",
			       MAX_SKB_FRAGS);
			return -EFAULT;
		}

		page = pgv_to_page(data);
		data += len;
		flush_dcache_page(page);
		get_page(page);
		skb_fill_page_desc(skb, nr_frags, page, offset, len);
		to_write -= len;
		offset = 0;
		len_max = PAGE_SIZE;
		len = ((to_write > len_max) ? len_max : to_write);
	}

	return tp_len;
}

static int tpacket_snd(struct packet_sock *po, struct msghdr *msg)
{
	struct sk_buff *skb;
	struct net_device *dev;
	__be16 proto;
	bool need_rls_dev = false;
	int err, reserve = 0;
	void *ph;
	struct sockaddr_ll *saddr = (struct sockaddr_ll *)msg->msg_name;
	int tp_len, size_max;
	unsigned char *addr;
	int len_sum = 0;
	int status = 0;

	mutex_lock(&po->pg_vec_lock);

	err = -EBUSY;
	if (saddr == NULL) {
		dev = po->prot_hook.dev;
		proto	= po->num;
		addr	= NULL;
	} else {
		err = -EINVAL;
		if (msg->msg_namelen < sizeof(struct sockaddr_ll))
			goto out;
		if (msg->msg_namelen < (saddr->sll_halen
					+ offsetof(struct sockaddr_ll,
						sll_addr)))
			goto out;
		proto	= saddr->sll_protocol;
		addr	= saddr->sll_addr;
		dev = dev_get_by_index(sock_net(&po->sk), saddr->sll_ifindex);
		need_rls_dev = true;
	}

	err = -ENXIO;
	if (unlikely(dev == NULL))
		goto out;

	reserve = dev->hard_header_len;

	err = -ENETDOWN;
	if (unlikely(!(dev->flags & IFF_UP)))
		goto out_put;

	size_max = po->tx_ring.frame_size
		- (po->tp_hdrlen - sizeof(struct sockaddr_ll));

	if (size_max > dev->mtu + reserve)
		size_max = dev->mtu + reserve;

	do {
		ph = packet_current_frame(po, &po->tx_ring,
				TP_STATUS_SEND_REQUEST);

		if (unlikely(ph == NULL)) {
			schedule();
			continue;
		}

		status = TP_STATUS_SEND_REQUEST;
		skb = sock_alloc_send_skb(&po->sk,
				LL_ALLOCATED_SPACE(dev)
				+ sizeof(struct sockaddr_ll),
				0, &err);

		if (unlikely(skb == NULL))
			goto out_status;

		tp_len = tpacket_fill_skb(po, skb, ph, dev, size_max, proto,
				addr);

		if (unlikely(tp_len < 0)) {
			if (po->tp_loss) {
				__packet_set_status(po, ph,
						TP_STATUS_AVAILABLE);
				packet_increment_head(&po->tx_ring);
				kfree_skb(skb);
				continue;
			} else {
				status = TP_STATUS_WRONG_FORMAT;
				err = tp_len;
				goto out_status;
			}
		}

		skb->destructor = tpacket_destruct_skb;
		__packet_set_status(po, ph, TP_STATUS_SENDING);
		atomic_inc(&po->tx_ring.pending);

		status = TP_STATUS_SEND_REQUEST;
		err = dev_queue_xmit(skb);
		if (unlikely(err > 0)) {
			err = net_xmit_errno(err);
			if (err && __packet_get_status(po, ph) ==
				   TP_STATUS_AVAILABLE) {
				/* skb was destructed already */
				skb = NULL;
				goto out_status;
			}
			/*
			 * skb was dropped but not destructed yet;
			 * let's treat it like congestion or err < 0
			 */
			err = 0;
		}
		packet_increment_head(&po->tx_ring);
		len_sum += tp_len;

	} while (likely((ph != NULL) ||
			((!(msg->msg_flags & MSG_DONTWAIT)) &&
			 (atomic_read(&po->tx_ring.pending))))
		);

	err = len_sum;
	goto out_put;

out_status:
	__packet_set_status(po, ph, status);
	kfree_skb(skb);
out_put:
	if (need_rls_dev)
		dev_put(dev);
out:
	mutex_unlock(&po->pg_vec_lock);
	return err;
}

static inline struct sk_buff *packet_alloc_skb(struct sock *sk, size_t prepad,
					       size_t reserve, size_t len,
					       size_t linear, int noblock,
					       int *err)
{
	struct sk_buff *skb;

	/* Under a page?  Don't bother with paged skb. */
	if (prepad + len < PAGE_SIZE || !linear)
		linear = len;

	skb = sock_alloc_send_pskb(sk, prepad + linear, len - linear, noblock,
				   err);
	if (!skb)
		return NULL;

	skb_reserve(skb, reserve);
	skb_put(skb, linear);
	skb->data_len = len - linear;
	skb->len += len - linear;

	return skb;
}

static int packet_snd(struct socket *sock,
			  struct msghdr *msg, size_t len)
{
	struct sock *sk = sock->sk;
	struct sockaddr_ll *saddr = (struct sockaddr_ll *)msg->msg_name;
	struct sk_buff *skb;
	struct net_device *dev;
	__be16 proto;
	bool need_rls_dev = false;
	unsigned char *addr;
	int err, reserve = 0;
	struct virtio_net_hdr vnet_hdr = { 0 };
	int offset = 0;
	int vnet_hdr_len;
	struct packet_sock *po = pkt_sk(sk);
	unsigned short gso_type = 0;

	/*
	 *	Get and verify the address.
	 */

	if (saddr == NULL) {
		dev = po->prot_hook.dev;
		proto	= po->num;
		addr	= NULL;
	} else {
		err = -EINVAL;
		if (msg->msg_namelen < sizeof(struct sockaddr_ll))
			goto out;
		if (msg->msg_namelen < (saddr->sll_halen + offsetof(struct sockaddr_ll, sll_addr)))
			goto out;
		proto	= saddr->sll_protocol;
		addr	= saddr->sll_addr;
		dev = dev_get_by_index(sock_net(sk), saddr->sll_ifindex);
		need_rls_dev = true;
	}

	err = -ENXIO;
	if (dev == NULL)
		goto out_unlock;
	if (sock->type == SOCK_RAW)
		reserve = dev->hard_header_len;

	err = -ENETDOWN;
	if (!(dev->flags & IFF_UP))
		goto out_unlock;

	if (po->has_vnet_hdr) {
		vnet_hdr_len = sizeof(vnet_hdr);

		err = -EINVAL;
		if (len < vnet_hdr_len)
			goto out_unlock;

		len -= vnet_hdr_len;

		err = memcpy_fromiovec((void *)&vnet_hdr, msg->msg_iov,
				       vnet_hdr_len);
		if (err < 0)
			goto out_unlock;

		if ((vnet_hdr.flags & VIRTIO_NET_HDR_F_NEEDS_CSUM) &&
		    (vnet_hdr.csum_start + vnet_hdr.csum_offset + 2 >
		      vnet_hdr.hdr_len))
			vnet_hdr.hdr_len = vnet_hdr.csum_start +
						 vnet_hdr.csum_offset + 2;

		err = -EINVAL;
		if (vnet_hdr.hdr_len > len)
			goto out_unlock;

		if (vnet_hdr.gso_type != VIRTIO_NET_HDR_GSO_NONE) {
			switch (vnet_hdr.gso_type & ~VIRTIO_NET_HDR_GSO_ECN) {
			case VIRTIO_NET_HDR_GSO_TCPV4:

				gso_type = SKB_GSO_TCPV4;
				break;
			case VIRTIO_NET_HDR_GSO_TCPV6:
				gso_type = SKB_GSO_TCPV6;
				break;
			case VIRTIO_NET_HDR_GSO_UDP:
				gso_type = SKB_GSO_UDP;
				break;
			default:
				goto out_unlock;
			}

			if (vnet_hdr.gso_type & VIRTIO_NET_HDR_GSO_ECN)
				gso_type |= SKB_GSO_TCP_ECN;

			if (vnet_hdr.gso_size == 0)
				goto out_unlock;

		}
	}

	err = -EMSGSIZE;
	if (!gso_type && (len > dev->mtu + reserve + VLAN_HLEN))
		goto out_unlock;

	err = -ENOBUFS;
	skb = packet_alloc_skb(sk, LL_ALLOCATED_SPACE(dev),
			       LL_RESERVED_SPACE(dev), len, vnet_hdr.hdr_len,
			       msg->msg_flags & MSG_DONTWAIT, &err);
	if (skb == NULL)
		goto out_unlock;

	skb_set_network_header(skb, reserve);

	err = -EINVAL;
	if (sock->type == SOCK_DGRAM &&
	    (offset = dev_hard_header(skb, dev, ntohs(proto), addr, NULL, len)) < 0)
		goto out_free;

	/* Returns -EFAULT on error */
	err = skb_copy_datagram_from_iovec(skb, offset, msg->msg_iov, 0, len);
	if (err)
		goto out_free;
	err = sock_tx_timestamp(sk, &skb_shinfo(skb)->tx_flags);
	if (err < 0)
		goto out_free;

	if (!gso_type && (len > dev->mtu + reserve)) {
		/* Earlier code assumed this would be a VLAN pkt,
		 * double-check this now that we have the actual
		 * packet in hand.
		 */
		struct ethhdr *ehdr;
		skb_reset_mac_header(skb);
		ehdr = eth_hdr(skb);
		if (ehdr->h_proto != htons(ETH_P_8021Q)) {
			err = -EMSGSIZE;
			goto out_free;
		}
	}

	skb->protocol = proto;
	skb->dev = dev;
	skb->priority = sk->sk_priority;
	skb->mark = sk->sk_mark;

	if (po->has_vnet_hdr) {
		if (vnet_hdr.flags & VIRTIO_NET_HDR_F_NEEDS_CSUM) {
			if (!skb_partial_csum_set(skb, vnet_hdr.csum_start,
						  vnet_hdr.csum_offset)) {
				err = -EINVAL;
				goto out_free;
			}
		}

		skb_shinfo(skb)->gso_size = vnet_hdr.gso_size;
		skb_shinfo(skb)->gso_type = gso_type;

		/* Header must be checked, and gso_segs computed. */
		skb_shinfo(skb)->gso_type |= SKB_GSO_DODGY;
		skb_shinfo(skb)->gso_segs = 0;

		len += vnet_hdr_len;
	}

	/*
	 *	Now send it
	 */

	err = dev_queue_xmit(skb);
	if (err > 0 && (err = net_xmit_errno(err)) != 0)
		goto out_unlock;

	if (need_rls_dev)
		dev_put(dev);

	return len;

out_free:
	kfree_skb(skb);
out_unlock:
	if (dev && need_rls_dev)
		dev_put(dev);
out:
	return err;
}

static int packet_sendmsg(struct kiocb *iocb, struct socket *sock,
		struct msghdr *msg, size_t len)
{
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);
	if (po->tx_ring.pg_vec)
		return tpacket_snd(po, msg);
	else
		return packet_snd(sock, msg, len);
}

/*
 *	Close a PACKET socket. This is fairly simple. We immediately go
 *	to 'closed' state and remove our protocol entry in the device list.
 */

static int packet_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct packet_sock *po;
	struct net *net;
	struct tpacket_req req;

	if (!sk)
		return 0;

	net = sock_net(sk);
	po = pkt_sk(sk);

	spin_lock_bh(&net->packet.sklist_lock);
	sk_del_node_init_rcu(sk);
	sock_prot_inuse_add(net, sk->sk_prot, -1);
	spin_unlock_bh(&net->packet.sklist_lock);

	spin_lock(&po->bind_lock);
	unregister_prot_hook(sk, false);
	if (po->prot_hook.dev) {
		dev_put(po->prot_hook.dev);
		po->prot_hook.dev = NULL;
	}
	spin_unlock(&po->bind_lock);

	packet_flush_mclist(sk);

	memset(&req, 0, sizeof(req));

	if (po->rx_ring.pg_vec)
		packet_set_ring(sk, &req, 1, 0);

	if (po->tx_ring.pg_vec)
		packet_set_ring(sk, &req, 1, 1);

	fanout_release(sk);

	synchronize_net();
	/*
	 *	Now the socket is dead. No more input will appear.
	 */
	sock_orphan(sk);
	sock->sk = NULL;

	/* Purge queues */

	skb_queue_purge(&sk->sk_receive_queue);
	sk_refcnt_debug_release(sk);

	sock_put(sk);
	return 0;
}

/*
 *	Attach a packet hook.
 */

static int packet_do_bind(struct sock *sk, struct net_device *dev, __be16 protocol)
{
	struct packet_sock *po = pkt_sk(sk);

	if (po->fanout)
		return -EINVAL;

	lock_sock(sk);

	spin_lock(&po->bind_lock);
	unregister_prot_hook(sk, true);
	po->num = protocol;
	po->prot_hook.type = protocol;
	if (po->prot_hook.dev)
		dev_put(po->prot_hook.dev);
	po->prot_hook.dev = dev;

	po->ifindex = dev ? dev->ifindex : 0;

	if (protocol == 0)
		goto out_unlock;

	if (!dev || (dev->flags & IFF_UP)) {
		register_prot_hook(sk);
	} else {
		sk->sk_err = ENETDOWN;
		if (!sock_flag(sk, SOCK_DEAD))
			sk->sk_error_report(sk);
	}

out_unlock:
	spin_unlock(&po->bind_lock);
	release_sock(sk);
	return 0;
}

/*
 *	Bind a packet socket to a device
 */

static int packet_bind_spkt(struct socket *sock, struct sockaddr *uaddr,
			    int addr_len)
{
	struct sock *sk = sock->sk;
	char name[15];
	struct net_device *dev;
	int err = -ENODEV;

	/*
	 *	Check legality
	 */

	if (addr_len != sizeof(struct sockaddr))
		return -EINVAL;
	strlcpy(name, uaddr->sa_data, sizeof(name));

	dev = dev_get_by_name(sock_net(sk), name);
	if (dev)
		err = packet_do_bind(sk, dev, pkt_sk(sk)->num);
	return err;
}

static int packet_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
	struct sockaddr_ll *sll = (struct sockaddr_ll *)uaddr;
	struct sock *sk = sock->sk;
	struct net_device *dev = NULL;
	int err;


	/*
	 *	Check legality
	 */

	if (addr_len < sizeof(struct sockaddr_ll))
		return -EINVAL;
	if (sll->sll_family != AF_PACKET)
		return -EINVAL;

	if (sll->sll_ifindex) {
		err = -ENODEV;
		dev = dev_get_by_index(sock_net(sk), sll->sll_ifindex);
		if (dev == NULL)
			goto out;
	}
	err = packet_do_bind(sk, dev, sll->sll_protocol ? : pkt_sk(sk)->num);

out:
	return err;
}

static struct proto packet_proto = {
	.name	  = "PACKET",
	.owner	  = THIS_MODULE,
	.obj_size = sizeof(struct packet_sock),
};

/*
 *	Create a packet of type SOCK_PACKET.
 */

static int packet_create(struct net *net, struct socket *sock, int protocol,
			 int kern)
{
	struct sock *sk;
	struct packet_sock *po;
	__be16 proto = (__force __be16)protocol; /* weird, but documented */
	int err;

	if (!capable(CAP_NET_RAW))
		return -EPERM;

	if (sock->type != SOCK_DGRAM && sock->type != SOCK_RAW &&
	    sock->type != SOCK_PACKET)
		return -ESOCKTNOSUPPORT;

	sock->state = SS_UNCONNECTED;

	err = -ENOBUFS;
	sk = sk_alloc(net, PF_PACKET, GFP_KERNEL, &packet_proto);
	if (sk == NULL)
		goto out;

	sock->ops = &packet_ops;
	if (sock->type == SOCK_PACKET)
		sock->ops = &packet_ops_spkt;

	sock_init_data(sock, sk);

	po = pkt_sk(sk);
	sk->sk_family = PF_PACKET;
	po->num = proto;

	sk->sk_destruct = packet_sock_destruct;
	sk_refcnt_debug_inc(sk);

	/*
	 *	Attach a protocol block
	 */

	spin_lock_init(&po->bind_lock);
	mutex_init(&po->pg_vec_lock);
	po->prot_hook.func = packet_rcv;

	if (sock->type == SOCK_PACKET)
		po->prot_hook.func = packet_rcv_spkt;

	po->prot_hook.af_packet_priv = sk;
	//printk("packet_create:packet_type addr 0x%x, sk addr 0x%x\n", &(po->prot_hook), sk);
	if (proto) {
		po->prot_hook.type = proto;
		register_prot_hook(sk);
	}

	spin_lock_bh(&net->packet.sklist_lock);
	sk_add_node_rcu(sk, &net->packet.sklist);
	sock_prot_inuse_add(net, &packet_proto, 1);
	spin_unlock_bh(&net->packet.sklist_lock);

	return 0;
out:
	return err;
}

static int packet_recv_error(struct sock *sk, struct msghdr *msg, int len)
{
	struct sock_exterr_skb *serr;
	struct sk_buff *skb, *skb2;
	int copied, err;

	err = -EAGAIN;
	skb = skb_dequeue(&sk->sk_error_queue);
	if (skb == NULL)
		goto out;

	copied = skb->len;
	if (copied > len) {
		msg->msg_flags |= MSG_TRUNC;
		copied = len;
	}
	err = skb_copy_datagram_iovec(skb, 0, msg->msg_iov, copied);
	if (err)
		goto out_free_skb;

	sock_recv_timestamp(msg, sk, skb);

	serr = SKB_EXT_ERR(skb);
	put_cmsg(msg, SOL_PACKET, PACKET_TX_TIMESTAMP,
		 sizeof(serr->ee), &serr->ee);

	msg->msg_flags |= MSG_ERRQUEUE;
	err = copied;

	/* Reset and regenerate socket error */
	spin_lock_bh(&sk->sk_error_queue.lock);
	sk->sk_err = 0;
	if ((skb2 = skb_peek(&sk->sk_error_queue)) != NULL) {
		sk->sk_err = SKB_EXT_ERR(skb2)->ee.ee_errno;
		spin_unlock_bh(&sk->sk_error_queue.lock);
		sk->sk_error_report(sk);
	} else
		spin_unlock_bh(&sk->sk_error_queue.lock);

out_free_skb:
	kfree_skb(skb);
out:
	return err;
}

/*
 *	Pull a packet from our receive queue and hand it to the user.
 *	If necessary we block.
 */

static int packet_recvmsg(struct kiocb *iocb, struct socket *sock,
			  struct msghdr *msg, size_t len, int flags)
{
	struct sock *sk = sock->sk;
	struct sk_buff *skb;
	int copied, err;
	struct sockaddr_ll *sll;
	int vnet_hdr_len = 0;

	err = -EINVAL;
	if (flags & ~(MSG_PEEK|MSG_DONTWAIT|MSG_TRUNC|MSG_CMSG_COMPAT|MSG_ERRQUEUE))
		goto out;

#if 0
	/* What error should we return now? EUNATTACH? */
	if (pkt_sk(sk)->ifindex < 0)
		return -ENODEV;
#endif

	if (flags & MSG_ERRQUEUE) {
		err = packet_recv_error(sk, msg, len);
		goto out;
	}

	/*
	 *	Call the generic datagram receiver. This handles all sorts
	 *	of horrible races and re-entrancy so we can forget about it
	 *	in the protocol layers.
	 *
	 *	Now it will return ENETDOWN, if device have just gone down,
	 *	but then it will block.
	 */

	skb = skb_recv_datagram(sk, flags, flags & MSG_DONTWAIT, &err);

	/*
	 *	An error occurred so return it. Because skb_recv_datagram()
	 *	handles the blocking we don't see and worry about blocking
	 *	retries.
	 */

	if (skb == NULL)
		goto out;

	if (pkt_sk(sk)->has_vnet_hdr) {
		struct virtio_net_hdr vnet_hdr = { 0 };

		err = -EINVAL;
		vnet_hdr_len = sizeof(vnet_hdr);
		if (len < vnet_hdr_len)
			goto out_free;

		len -= vnet_hdr_len;

		if (skb_is_gso(skb)) {
			struct skb_shared_info *sinfo = skb_shinfo(skb);

			/* This is a hint as to how much should be linear. */
			vnet_hdr.hdr_len = skb_headlen(skb);
			vnet_hdr.gso_size = sinfo->gso_size;
			if (sinfo->gso_type & SKB_GSO_TCPV4)
				vnet_hdr.gso_type = VIRTIO_NET_HDR_GSO_TCPV4;
			else if (sinfo->gso_type & SKB_GSO_TCPV6)
				vnet_hdr.gso_type = VIRTIO_NET_HDR_GSO_TCPV6;
			else if (sinfo->gso_type & SKB_GSO_UDP)
				vnet_hdr.gso_type = VIRTIO_NET_HDR_GSO_UDP;
			else if (sinfo->gso_type & SKB_GSO_FCOE)
				goto out_free;
			else
				BUG();
			if (sinfo->gso_type & SKB_GSO_TCP_ECN)
				vnet_hdr.gso_type |= VIRTIO_NET_HDR_GSO_ECN;
		} else

			vnet_hdr.gso_type = VIRTIO_NET_HDR_GSO_NONE;

		if (skb->ip_summed == CHECKSUM_PARTIAL) {
			vnet_hdr.flags = VIRTIO_NET_HDR_F_NEEDS_CSUM;
			vnet_hdr.csum_start = skb_checksum_start_offset(skb);
			vnet_hdr.csum_offset = skb->csum_offset;
		} else if (skb->ip_summed == CHECKSUM_UNNECESSARY) {
			vnet_hdr.flags = VIRTIO_NET_HDR_F_DATA_VALID;
		} /* else everything is zero */

		err = memcpy_toiovec(msg->msg_iov, (void *)&vnet_hdr,
				     vnet_hdr_len);
		if (err < 0)
			goto out_free;
	}

	/*
	 *	If the address length field is there to be filled in, we fill
	 *	it in now.
	 */

	sll = &PACKET_SKB_CB(skb)->sa.ll;
	if (sock->type == SOCK_PACKET)
		msg->msg_namelen = sizeof(struct sockaddr_pkt);
	else
		msg->msg_namelen = sll->sll_halen + offsetof(struct sockaddr_ll, sll_addr);

	/*
	 *	You lose any data beyond the buffer you gave. If it worries a
	 *	user program they can ask the device for its MTU anyway.
	 */

	copied = skb->len;
	if (copied > len) {
		copied = len;
		msg->msg_flags |= MSG_TRUNC;
	}

	err = skb_copy_datagram_iovec(skb, 0, msg->msg_iov, copied);
	if (err)
		goto out_free;

	sock_recv_ts_and_drops(msg, sk, skb);

	if (msg->msg_name)
		memcpy(msg->msg_name, &PACKET_SKB_CB(skb)->sa,
		       msg->msg_namelen);

	if (pkt_sk(sk)->auxdata) {
		struct tpacket_auxdata aux;

		aux.tp_status = TP_STATUS_USER;
		if (skb->ip_summed == CHECKSUM_PARTIAL)
			aux.tp_status |= TP_STATUS_CSUMNOTREADY;
		aux.tp_len = PACKET_SKB_CB(skb)->origlen;
		aux.tp_snaplen = skb->len;
		aux.tp_mac = 0;
		aux.tp_net = skb_network_offset(skb);
		if (vlan_tx_tag_present(skb)) {
			aux.tp_vlan_tci = vlan_tx_tag_get(skb);
			aux.tp_status |= TP_STATUS_VLAN_VALID;
		} else {
			aux.tp_vlan_tci = 0;
		}
		aux.tp_padding = 0;
		put_cmsg(msg, SOL_PACKET, PACKET_AUXDATA, sizeof(aux), &aux);
	}

	/*
	 *	Free or return the buffer as appropriate. Again this
	 *	hides all the races and re-entrancy issues from us.
	 */
	err = vnet_hdr_len + ((flags&MSG_TRUNC) ? skb->len : copied);

out_free:
	skb_free_datagram(sk, skb);
out:
	return err;
}

static int packet_getname_spkt(struct socket *sock, struct sockaddr *uaddr,
			       int *uaddr_len, int peer)
{
	struct net_device *dev;
	struct sock *sk	= sock->sk;

	if (peer)
		return -EOPNOTSUPP;

	uaddr->sa_family = AF_PACKET;
	rcu_read_lock();
	dev = dev_get_by_index_rcu(sock_net(sk), pkt_sk(sk)->ifindex);
	if (dev)
		strncpy(uaddr->sa_data, dev->name, 14);
	else
		memset(uaddr->sa_data, 0, 14);
	rcu_read_unlock();
	*uaddr_len = sizeof(*uaddr);

	return 0;
}

static int packet_getname(struct socket *sock, struct sockaddr *uaddr,
			  int *uaddr_len, int peer)
{
	struct net_device *dev;
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);
	DECLARE_SOCKADDR(struct sockaddr_ll *, sll, uaddr);

	if (peer)
		return -EOPNOTSUPP;

	sll->sll_family = AF_PACKET;
	sll->sll_ifindex = po->ifindex;
	sll->sll_protocol = po->num;
	sll->sll_pkttype = 0;
	rcu_read_lock();

	dev = dev_get_by_index_rcu(sock_net(sk), po->ifindex);
	if (dev) {
		sll->sll_hatype = dev->type;
		sll->sll_halen = dev->addr_len;
		memcpy(sll->sll_addr, dev->dev_addr, dev->addr_len);
	} else {
		sll->sll_hatype = 0;	/* Bad: we have no ARPHRD_UNSPEC */
		sll->sll_halen = 0;
	}
	rcu_read_unlock();
	*uaddr_len = offsetof(struct sockaddr_ll, sll_addr) + sll->sll_halen;

	return 0;
}

static int packet_dev_mc(struct net_device *dev, struct packet_mclist *i,
			 int what)
{
	switch (i->type) {
	case PACKET_MR_MULTICAST:
		if (i->alen != dev->addr_len)
			return -EINVAL;
		if (what > 0)
			return dev_mc_add(dev, i->addr);
		else
			return dev_mc_del(dev, i->addr);
		break;
	case PACKET_MR_PROMISC:
		return dev_set_promiscuity(dev, what);
		break;
	case PACKET_MR_ALLMULTI:
		return dev_set_allmulti(dev, what);
		break;
	case PACKET_MR_UNICAST:
		if (i->alen != dev->addr_len)
			return -EINVAL;
		if (what > 0)
			return dev_uc_add(dev, i->addr);
		else
			return dev_uc_del(dev, i->addr);
		break;
	default:
		break;
	}
	return 0;
}

static void packet_dev_mclist(struct net_device *dev, struct packet_mclist *i, int what)
{
	for ( ; i; i = i->next) {
		if (i->ifindex == dev->ifindex)
			packet_dev_mc(dev, i, what);
	}
}

static int packet_mc_add(struct sock *sk, struct packet_mreq_max *mreq)
{
	struct packet_sock *po = pkt_sk(sk);
	struct packet_mclist *ml, *i;
	struct net_device *dev;
	int err;

	rtnl_lock();

	err = -ENODEV;
	dev = __dev_get_by_index(sock_net(sk), mreq->mr_ifindex);
	if (!dev)
		goto done;

	err = -EINVAL;
	if (mreq->mr_alen > dev->addr_len)
		goto done;

	err = -ENOBUFS;
	i = kmalloc(sizeof(*i), GFP_KERNEL);
	if (i == NULL)
		goto done;

	err = 0;
	for (ml = po->mclist; ml; ml = ml->next) {
		if (ml->ifindex == mreq->mr_ifindex &&
		    ml->type == mreq->mr_type &&
		    ml->alen == mreq->mr_alen &&
		    memcmp(ml->addr, mreq->mr_address, ml->alen) == 0) {
			ml->count++;
			/* Free the new element ... */
			kfree(i);
			goto done;
		}
	}

	i->type = mreq->mr_type;
	i->ifindex = mreq->mr_ifindex;
	i->alen = mreq->mr_alen;
	memcpy(i->addr, mreq->mr_address, i->alen);
	i->count = 1;
	i->next = po->mclist;
	po->mclist = i;
	err = packet_dev_mc(dev, i, 1);
	if (err) {
		po->mclist = i->next;
		kfree(i);
	}

done:
	rtnl_unlock();
	return err;
}

static int packet_mc_drop(struct sock *sk, struct packet_mreq_max *mreq)
{
	struct packet_mclist *ml, **mlp;

	rtnl_lock();

	for (mlp = &pkt_sk(sk)->mclist; (ml = *mlp) != NULL; mlp = &ml->next) {
		if (ml->ifindex == mreq->mr_ifindex &&
		    ml->type == mreq->mr_type &&
		    ml->alen == mreq->mr_alen &&
		    memcmp(ml->addr, mreq->mr_address, ml->alen) == 0) {
			if (--ml->count == 0) {
				struct net_device *dev;
				*mlp = ml->next;
				dev = __dev_get_by_index(sock_net(sk), ml->ifindex);
				if (dev)
					packet_dev_mc(dev, ml, -1);
				kfree(ml);
			}
			rtnl_unlock();
			return 0;
		}
	}
	rtnl_unlock();
	return -EADDRNOTAVAIL;
}

static void packet_flush_mclist(struct sock *sk)
{
	struct packet_sock *po = pkt_sk(sk);
	struct packet_mclist *ml;

	if (!po->mclist)
		return;

	rtnl_lock();
	while ((ml = po->mclist) != NULL) {
		struct net_device *dev;

		po->mclist = ml->next;
		dev = __dev_get_by_index(sock_net(sk), ml->ifindex);
		if (dev != NULL)
			packet_dev_mc(dev, ml, -1);
		kfree(ml);
	}
	rtnl_unlock();
}

static int
packet_setsockopt(struct socket *sock, int level, int optname, char __user *optval, unsigned int optlen)
{
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);
	int ret;

	if (level != SOL_PACKET)
		return -ENOPROTOOPT;

	switch (optname) {
	case PACKET_ADD_MEMBERSHIP:
	case PACKET_DROP_MEMBERSHIP:
	{
		struct packet_mreq_max mreq;
		int len = optlen;
		memset(&mreq, 0, sizeof(mreq));
		if (len < sizeof(struct packet_mreq))
			return -EINVAL;
		if (len > sizeof(mreq))
			len = sizeof(mreq);
		if (copy_from_user(&mreq, optval, len))
			return -EFAULT;
		if (len < (mreq.mr_alen + offsetof(struct packet_mreq, mr_address)))
			return -EINVAL;
		if (optname == PACKET_ADD_MEMBERSHIP)
			ret = packet_mc_add(sk, &mreq);
		else
			ret = packet_mc_drop(sk, &mreq);
		return ret;
	}

	case PACKET_RX_RING:
	case PACKET_TX_RING:
	{
		struct tpacket_req req;

		if (optlen < sizeof(req))
			return -EINVAL;
		if (pkt_sk(sk)->has_vnet_hdr)
			return -EINVAL;
		if (copy_from_user(&req, optval, sizeof(req)))
			return -EFAULT;
		return packet_set_ring(sk, &req, 0, optname == PACKET_TX_RING);
	}
	case PACKET_COPY_THRESH:
	{
		int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (copy_from_user(&val, optval, sizeof(val)))
			return -EFAULT;

		pkt_sk(sk)->copy_thresh = val;
		return 0;
	}
	case PACKET_VERSION:
	{
		int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec)
			return -EBUSY;
		if (copy_from_user(&val, optval, sizeof(val)))
			return -EFAULT;
		switch (val) {
		case TPACKET_V1:
		case TPACKET_V2:
			po->tp_version = val;
			return 0;
		default:
			return -EINVAL;
		}
	}
	case PACKET_RESERVE:
	{
		unsigned int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec)
			return -EBUSY;
		if (copy_from_user(&val, optval, sizeof(val)))
			return -EFAULT;
		po->tp_reserve = val;
		return 0;
	}
	case PACKET_LOSS:
	{
		unsigned int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec)
			return -EBUSY;
		if (copy_from_user(&val, optval, sizeof(val)))
			return -EFAULT;
		po->tp_loss = !!val;
		return 0;
	}
	case PACKET_AUXDATA:
	{
		int val;

		if (optlen < sizeof(val))
			return -EINVAL;
		if (copy_from_user(&val, optval, sizeof(val)))
			return -EFAULT;

		po->auxdata = !!val;
		return 0;
	}
	case PACKET_ORIGDEV:
	{
		int val;

		if (optlen < sizeof(val))
			return -EINVAL;
		if (copy_from_user(&val, optval, sizeof(val)))
			return -EFAULT;

		po->origdev = !!val;
		return 0;
	}
	case PACKET_VNET_HDR:
	{
		int val;

		if (sock->type != SOCK_RAW)
			return -EINVAL;
		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec)
			return -EBUSY;
		if (optlen < sizeof(val))
			return -EINVAL;
		if (copy_from_user(&val, optval, sizeof(val)))
			return -EFAULT;

		po->has_vnet_hdr = !!val;
		return 0;
	}
	case PACKET_TIMESTAMP:
	{
		int val;

		if (optlen != sizeof(val))
			return -EINVAL;
		if (copy_from_user(&val, optval, sizeof(val)))
			return -EFAULT;

		po->tp_tstamp = val;
		return 0;
	}
	case PACKET_FANOUT:
	{
		int val;

		if (optlen != sizeof(val))
			return -EINVAL;

		if (copy_from_user(&val, optval, sizeof(val)))
			return -EFAULT;

		return fanout_add(sk, val & 0xffff, val >> 16);
	}
	default:
		return -ENOPROTOOPT;
	}
}

static int packet_getsockopt(struct socket *sock, int level, int optname,
			     char __user *optval, int __user *optlen)
{
	int len;
	int val;
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);
	void *data;
	struct tpacket_stats st;

	if (level != SOL_PACKET)
		return -ENOPROTOOPT;

	if (get_user(len, optlen))
		return -EFAULT;

	if (len < 0)
		return -EINVAL;

	switch (optname) {
	case PACKET_STATISTICS:
		if (len > sizeof(struct tpacket_stats))
			len = sizeof(struct tpacket_stats);
		spin_lock_bh(&sk->sk_receive_queue.lock);
		st = po->stats;
		memset(&po->stats, 0, sizeof(st));
		spin_unlock_bh(&sk->sk_receive_queue.lock);
		st.tp_packets += st.tp_drops;

		data = &st;
		break;
	case PACKET_AUXDATA:
		if (len > sizeof(int))
			len = sizeof(int);

		val = po->auxdata;

		data = &val;
		break;
	case PACKET_ORIGDEV:
		if (len > sizeof(int))
			len = sizeof(int);
		val = po->origdev;

		data = &val;
		break;
	case PACKET_VNET_HDR:
		if (len > sizeof(int))
			len = sizeof(int);
		val = po->has_vnet_hdr;

		data = &val;
		break;
	case PACKET_VERSION:
		if (len > sizeof(int))
			len = sizeof(int);
		val = po->tp_version;
		data = &val;
		break;
	case PACKET_HDRLEN:
		if (len > sizeof(int))
			len = sizeof(int);
		if (copy_from_user(&val, optval, len))
			return -EFAULT;
		switch (val) {
		case TPACKET_V1:
			val = sizeof(struct tpacket_hdr);
			break;
		case TPACKET_V2:
			val = sizeof(struct tpacket2_hdr);
			break;
		default:
			return -EINVAL;
		}
		data = &val;
		break;
	case PACKET_RESERVE:
		if (len > sizeof(unsigned int))
			len = sizeof(unsigned int);
		val = po->tp_reserve;
		data = &val;
		break;
	case PACKET_LOSS:
		if (len > sizeof(unsigned int))
			len = sizeof(unsigned int);
		val = po->tp_loss;
		data = &val;
		break;
	case PACKET_TIMESTAMP:
		if (len > sizeof(int))
			len = sizeof(int);
		val = po->tp_tstamp;
		data = &val;
		break;
	case PACKET_FANOUT:
		if (len > sizeof(int))
			len = sizeof(int);
		val = (po->fanout ?
		       ((u32)po->fanout->id |
			((u32)po->fanout->type << 16)) :
		       0);
		data = &val;
		break;
	default:
		return -ENOPROTOOPT;
	}

	if (put_user(len, optlen))
		return -EFAULT;
	if (copy_to_user(optval, data, len))
		return -EFAULT;
	return 0;
}


static int packet_notifier(struct notifier_block *this, unsigned long msg, void *data)
{
	struct sock *sk;
	struct hlist_node *node;
	struct net_device *dev = data;
	struct net *net = dev_net(dev);

	rcu_read_lock();
	sk_for_each_rcu(sk, node, &net->packet.sklist) {
		struct packet_sock *po = pkt_sk(sk);

		switch (msg) {
		case NETDEV_UNREGISTER:
			if (po->mclist)
				packet_dev_mclist(dev, po->mclist, -1);
			/* fallthrough */

		case NETDEV_DOWN:
			if (dev->ifindex == po->ifindex) {
				spin_lock(&po->bind_lock);
				if (po->running) {
					__unregister_prot_hook(sk, false);
					sk->sk_err = ENETDOWN;
					if (!sock_flag(sk, SOCK_DEAD))
						sk->sk_error_report(sk);
				}
				if (msg == NETDEV_UNREGISTER) {
					po->ifindex = -1;
					if (po->prot_hook.dev)
						dev_put(po->prot_hook.dev);
					po->prot_hook.dev = NULL;
				}
				spin_unlock(&po->bind_lock);
			}
			break;
		case NETDEV_UP:
			if (dev->ifindex == po->ifindex) {
				spin_lock(&po->bind_lock);
				if (po->num)
					register_prot_hook(sk);
				spin_unlock(&po->bind_lock);
			}
			break;
		}
	}
	rcu_read_unlock();
	return NOTIFY_DONE;
}


static int packet_ioctl(struct socket *sock, unsigned int cmd,
			unsigned long arg)
{
	struct sock *sk = sock->sk;

	switch (cmd) {
	case SIOCOUTQ:
	{
		int amount = sk_wmem_alloc_get(sk);

		return put_user(amount, (int __user *)arg);
	}
	case SIOCINQ:
	{
		struct sk_buff *skb;
		int amount = 0;

		spin_lock_bh(&sk->sk_receive_queue.lock);
		skb = skb_peek(&sk->sk_receive_queue);
		if (skb)
			amount = skb->len;
		spin_unlock_bh(&sk->sk_receive_queue.lock);
		return put_user(amount, (int __user *)arg);
	}
	case SIOCGSTAMP:
		return sock_get_timestamp(sk, (struct timeval __user *)arg);
	case SIOCGSTAMPNS:
		return sock_get_timestampns(sk, (struct timespec __user *)arg);

#ifdef CONFIG_INET
	case SIOCADDRT:
	case SIOCDELRT:
	case SIOCDARP:
	case SIOCGARP:
	case SIOCSARP:
	case SIOCGIFADDR:
	case SIOCSIFADDR:
	case SIOCGIFBRDADDR:
	case SIOCSIFBRDADDR:
	case SIOCGIFNETMASK:
	case SIOCSIFNETMASK:
	case SIOCGIFDSTADDR:
	case SIOCSIFDSTADDR:
	case SIOCSIFFLAGS:
		return inet_dgram_ops.ioctl(sock, cmd, arg);
#endif

	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static unsigned int packet_poll(struct file *file, struct socket *sock,
				poll_table *wait)
{
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);
	unsigned int mask = datagram_poll(file, sock, wait);

	spin_lock_bh(&sk->sk_receive_queue.lock);
	if (po->rx_ring.pg_vec) {
		if (!packet_previous_frame(po, &po->rx_ring, TP_STATUS_KERNEL))
			mask |= POLLIN | POLLRDNORM;
	}
	spin_unlock_bh(&sk->sk_receive_queue.lock);
	spin_lock_bh(&sk->sk_write_queue.lock);
	if (po->tx_ring.pg_vec) {
		if (packet_current_frame(po, &po->tx_ring, TP_STATUS_AVAILABLE))
			mask |= POLLOUT | POLLWRNORM;
	}
	spin_unlock_bh(&sk->sk_write_queue.lock);
	return mask;
}


/* Dirty? Well, I still did not learn better way to account
 * for user mmaps.
 */

static void packet_mm_open(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	struct socket *sock = file->private_data;
	struct sock *sk = sock->sk;

	if (sk)
		atomic_inc(&pkt_sk(sk)->mapped);
}

static void packet_mm_close(struct vm_area_struct *vma)
{
	struct file *file = vma->vm_file;
	struct socket *sock = file->private_data;
	struct sock *sk = sock->sk;

	if (sk)
		atomic_dec(&pkt_sk(sk)->mapped);
}

static const struct vm_operations_struct packet_mmap_ops = {
	.open	=	packet_mm_open,
	.close	=	packet_mm_close,
};

static void free_pg_vec(struct pgv *pg_vec, unsigned int order,
			unsigned int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (likely(pg_vec[i].buffer)) {
			if (is_vmalloc_addr(pg_vec[i].buffer))
				vfree(pg_vec[i].buffer);
			else
				free_pages((unsigned long)pg_vec[i].buffer,
					   order);

			pg_vec[i].buffer = NULL;
		}
	}
	kfree(pg_vec);
}

static inline char *alloc_one_pg_vec_page(unsigned long order)
{
	char *buffer = NULL;
	gfp_t gfp_flags = GFP_KERNEL | __GFP_COMP |
			  __GFP_ZERO | __GFP_NOWARN | __GFP_NORETRY;

	buffer = (char *) __get_free_pages(gfp_flags, order);

	if (buffer)
		return buffer;

	/*
	 * __get_free_pages failed, fall back to vmalloc
	 */
	buffer = vzalloc((1 << order) * PAGE_SIZE);

	if (buffer)
		return buffer;

	/*
	 * vmalloc failed, lets dig into swap here
	 */
	gfp_flags &= ~__GFP_NORETRY;
	buffer = (char *)__get_free_pages(gfp_flags, order);
	if (buffer)
		return buffer;

	/*
	 * complete and utter failure
	 */
	return NULL;
}

static struct pgv *alloc_pg_vec(struct tpacket_req *req, int order)
{
	unsigned int block_nr = req->tp_block_nr;
	struct pgv *pg_vec;
	int i;

	pg_vec = kcalloc(block_nr, sizeof(struct pgv), GFP_KERNEL);
	if (unlikely(!pg_vec))
		goto out;

	for (i = 0; i < block_nr; i++) {
		pg_vec[i].buffer = alloc_one_pg_vec_page(order);
		if (unlikely(!pg_vec[i].buffer))
			goto out_free_pgvec;
	}

out:
	return pg_vec;

out_free_pgvec:
	free_pg_vec(pg_vec, order, block_nr);
	pg_vec = NULL;
	goto out;
}

static int packet_set_ring(struct sock *sk, struct tpacket_req *req,
		int closing, int tx_ring)
{
	struct pgv *pg_vec = NULL;
	struct packet_sock *po = pkt_sk(sk);
	int was_running, order = 0;
	struct packet_ring_buffer *rb;
	struct sk_buff_head *rb_queue;
	__be16 num;
	int err;

	rb = tx_ring ? &po->tx_ring : &po->rx_ring;
	rb_queue = tx_ring ? &sk->sk_write_queue : &sk->sk_receive_queue;

	err = -EBUSY;
	if (!closing) {
		if (atomic_read(&po->mapped))
			goto out;
		if (atomic_read(&rb->pending))
			goto out;
	}

	if (req->tp_block_nr) {
		/* Sanity tests and some calculations */
		err = -EBUSY;
		if (unlikely(rb->pg_vec))
			goto out;

		switch (po->tp_version) {
		case TPACKET_V1:
			po->tp_hdrlen = TPACKET_HDRLEN;
			break;
		case TPACKET_V2:
			po->tp_hdrlen = TPACKET2_HDRLEN;
			break;
		}

		err = -EINVAL;
		if (unlikely((int)req->tp_block_size <= 0))
			goto out;
		if (unlikely(req->tp_block_size & (PAGE_SIZE - 1)))
			goto out;
		if (unlikely(req->tp_frame_size < po->tp_hdrlen +
					po->tp_reserve))
			goto out;
		if (unlikely(req->tp_frame_size & (TPACKET_ALIGNMENT - 1)))
			goto out;

		rb->frames_per_block = req->tp_block_size/req->tp_frame_size;
		if (unlikely(rb->frames_per_block <= 0))
			goto out;
		if (unlikely((rb->frames_per_block * req->tp_block_nr) !=
					req->tp_frame_nr))
			goto out;

		err = -ENOMEM;
		order = get_order(req->tp_block_size);
		pg_vec = alloc_pg_vec(req, order);
		if (unlikely(!pg_vec))
			goto out;
	}
	/* Done */
	else {
		err = -EINVAL;
		if (unlikely(req->tp_frame_nr))
			goto out;
	}

	lock_sock(sk);

	/* Detach socket from network */
	spin_lock(&po->bind_lock);
	was_running = po->running;
	num = po->num;
	if (was_running) {
		po->num = 0;
		__unregister_prot_hook(sk, false);
	}
	spin_unlock(&po->bind_lock);

	synchronize_net();

	err = -EBUSY;
	mutex_lock(&po->pg_vec_lock);
	if (closing || atomic_read(&po->mapped) == 0) {
		err = 0;
		spin_lock_bh(&rb_queue->lock);
		swap(rb->pg_vec, pg_vec);
		rb->frame_max = (req->tp_frame_nr - 1);
		rb->head = 0;
		rb->frame_size = req->tp_frame_size;
		spin_unlock_bh(&rb_queue->lock);

		swap(rb->pg_vec_order, order);
		swap(rb->pg_vec_len, req->tp_block_nr);

		rb->pg_vec_pages = req->tp_block_size/PAGE_SIZE;
		po->prot_hook.func = (po->rx_ring.pg_vec) ?
						tpacket_rcv : packet_rcv;
		skb_queue_purge(rb_queue);
		if (atomic_read(&po->mapped))
			pr_err("packet_mmap: vma is busy: %d\n",
			       atomic_read(&po->mapped));
	}
	mutex_unlock(&po->pg_vec_lock);

	spin_lock(&po->bind_lock);
	if (was_running) {

		po->num = num;
		register_prot_hook(sk);
	}
	spin_unlock(&po->bind_lock);

	release_sock(sk);

	if (pg_vec)
		free_pg_vec(pg_vec, order, req->tp_block_nr);
out:
	return err;
}

static int packet_mmap(struct file *file, struct socket *sock,
		struct vm_area_struct *vma)
{
	struct sock *sk = sock->sk;
	struct packet_sock *po = pkt_sk(sk);
	unsigned long size, expected_size;
	struct packet_ring_buffer *rb;
	unsigned long start;
	int err = -EINVAL;
	int i;

	if (vma->vm_pgoff)
		return -EINVAL;

	mutex_lock(&po->pg_vec_lock);

	expected_size = 0;
	for (rb = &po->rx_ring; rb <= &po->tx_ring; rb++) {
		if (rb->pg_vec) {
			expected_size += rb->pg_vec_len
						* rb->pg_vec_pages
						* PAGE_SIZE;
		}
	}

	if (expected_size == 0)
		goto out;

	size = vma->vm_end - vma->vm_start;
	if (size != expected_size)
		goto out;

	start = vma->vm_start;
	for (rb = &po->rx_ring; rb <= &po->tx_ring; rb++) {
		if (rb->pg_vec == NULL)
			continue;

		for (i = 0; i < rb->pg_vec_len; i++) {
			struct page *page;
			void *kaddr = rb->pg_vec[i].buffer;
			int pg_num;

			for (pg_num = 0; pg_num < rb->pg_vec_pages; pg_num++) {
				page = pgv_to_page(kaddr);
				err = vm_insert_page(vma, start, page);
				if (unlikely(err))
					goto out;
				start += PAGE_SIZE;
				kaddr += PAGE_SIZE;
			}
		}
	}

	atomic_inc(&po->mapped);
	vma->vm_ops = &packet_mmap_ops;
	err = 0;

out:
	mutex_unlock(&po->pg_vec_lock);

	return err;
}

static const struct proto_ops packet_ops_spkt = {
	.family =	PF_PACKET,
	.owner =	THIS_MODULE,
	.release =	packet_release,
	.bind =		packet_bind_spkt,
	.connect =	sock_no_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	sock_no_accept,
	.getname =	packet_getname_spkt,
	.poll =		datagram_poll,
	.ioctl =	packet_ioctl,
	.listen =	sock_no_listen,
	.shutdown =	sock_no_shutdown,
	.setsockopt =	sock_no_setsockopt,
	.getsockopt =	sock_no_getsockopt,
	.sendmsg =	packet_sendmsg_spkt,
	.recvmsg =	packet_recvmsg,
	.mmap =		sock_no_mmap,
	.sendpage =	sock_no_sendpage,
};

static const struct proto_ops packet_ops = {
	.family =	PF_PACKET,
	.owner =	THIS_MODULE,
	.release =	packet_release,
	.bind =		packet_bind,
	.connect =	sock_no_connect,
	.socketpair =	sock_no_socketpair,
	.accept =	sock_no_accept,
	.getname =	packet_getname,
	.poll =		packet_poll,
	.ioctl =	packet_ioctl,
	.listen =	sock_no_listen,
	.shutdown =	sock_no_shutdown,
	.setsockopt =	packet_setsockopt,
	.getsockopt =	packet_getsockopt,
	.sendmsg =	packet_sendmsg,
	.recvmsg =	packet_recvmsg,
	.mmap =		packet_mmap,
	.sendpage =	sock_no_sendpage,
};

static const struct net_proto_family packet_family_ops = {
	.family =	PF_PACKET,
	.create =	packet_create,
	.owner	=	THIS_MODULE,
};

static struct notifier_block packet_netdev_notifier = {
	.notifier_call =	packet_notifier,
};

#ifdef CONFIG_PROC_FS

static void *packet_seq_start(struct seq_file *seq, loff_t *pos)
	__acquires(RCU)
{
	struct net *net = seq_file_net(seq);

	rcu_read_lock();
	return seq_hlist_start_head_rcu(&net->packet.sklist, *pos);
}

static void *packet_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct net *net = seq_file_net(seq);
	return seq_hlist_next_rcu(v, &net->packet.sklist, pos);
}

static void packet_seq_stop(struct seq_file *seq, void *v)
	__releases(RCU)
{
	rcu_read_unlock();
}

static int packet_seq_show(struct seq_file *seq, void *v)
{
	if (v == SEQ_START_TOKEN)
		seq_puts(seq, "sk       RefCnt Type Proto  Iface R Rmem   User   Inode\n");
	else {
		struct sock *s = sk_entry(v);
		const struct packet_sock *po = pkt_sk(s);

		seq_printf(seq,
			   "%pK %-6d %-4d %04x   %-5d %1d %-6u %-6u %-6lu\n",
			   s,
			   atomic_read(&s->sk_refcnt),
			   s->sk_type,
			   ntohs(po->num),
			   po->ifindex,
			   po->running,
			   atomic_read(&s->sk_rmem_alloc),
			   sock_i_uid(s),
			   sock_i_ino(s));
	}

	return 0;
}

static const struct seq_operations packet_seq_ops = {
	.start	= packet_seq_start,
	.next	= packet_seq_next,
	.stop	= packet_seq_stop,
	.show	= packet_seq_show,
};

static int packet_seq_open(struct inode *inode, struct file *file)
{
	return seq_open_net(inode, file, &packet_seq_ops,
			    sizeof(struct seq_net_private));
}

static const struct file_operations packet_seq_fops = {
	.owner		= THIS_MODULE,
	.open		= packet_seq_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release_net,
};

#endif

static int __net_init packet_net_init(struct net *net)
{
	spin_lock_init(&net->packet.sklist_lock);
	INIT_HLIST_HEAD(&net->packet.sklist);

	if (!proc_net_fops_create(net, "packet", 0, &packet_seq_fops))
		return -ENOMEM;

	return 0;
}

static void __net_exit packet_net_exit(struct net *net)
{
	proc_net_remove(net, "packet");
}

static struct pernet_operations packet_net_ops = {
	.init = packet_net_init,
	.exit = packet_net_exit,
};

#define WX_PROCDIR                           "wx_procdir"
#define RUN_TIMES					"run_times"
#define TEST_CASE					"test_case"

static struct proc_dir_entry *proc_entry1;
static struct proc_dir_entry *proc_entry2 ;

static struct proc_dir_entry *wx_procdir ;



static int wx_proc_read(char *page, char **start, off_t off, int count,
  int*eof, void *data)  {
	int len;
	
	if (off > 0)
	{
		*eof = 1;
		return 0;
	}
	len = sprintf(page, "%ld\n", run_times);
	printk("in the read wxwxwx \n");
	return len;
}

#define atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0)
#define MAX_UL_LEN 16

static int wx_proc_write(struct file *filp, const char __user *buff, unsigned
  long len, void *data)
{
	
	char k_buf[MAX_UL_LEN];
	//char *endp;
	//unsigned long new;
	int count;
	int ret;

	printk("in the write hahahha\n");

	if(MAX_UL_LEN <= len)
		count = MAX_UL_LEN;
	else 	
		count = len;

	if (copy_from_user(k_buf, buff, count))
	{
		ret =  - EFAULT;

		goto err;
	}
	else
	{
		/*new = simple_strtoul(k_buf, &endp, 16); 
		if (endp == k_buf)

		{
			ret =  - EINVAL;
			goto err;
			printk("input error\n");
		}*/
		sscanf(k_buf,"%ld",&run_times);

		printk("haha now key is %ld\n",run_times);
		return count;
	}

	err:
		return ret;
}


static int test_proc_read(char *page, char **start, off_t off, int count,
  int*eof, void *data)  {
	int len;
	
	if (off > 0)
	{
		*eof = 1;
		return 0;
	}
	len = sprintf(page, "%d\n", test_case);
	printk("test in the read wxwxwx \n");
	return len;
}



static int test_proc_write(struct file *filp, const char __user *buff, unsigned
  long len, void *data)
{
	
	char k_buf[MAX_UL_LEN];
	//char *endp;
	//unsigned long new;
	int count;
	int ret;

	printk("test in the write hahahha\n");

	if(MAX_UL_LEN <= len)
		count = MAX_UL_LEN;
	else 	
		count = len;

	if (copy_from_user(k_buf, buff, count))
	{
		ret =  - EFAULT;
		goto err;
	}
	else
	{
		sscanf(k_buf,"%d",&test_case);

		printk("test haha now test is %d\n",  test_case);
		return count;
	}

	err:
		return ret;
}


static int  test_proc_init(void) {
	
	wx_procdir=proc_mkdir(WX_PROCDIR , NULL);

	proc_entry1=create_proc_entry(RUN_TIMES, 0666, wx_procdir); 
	proc_entry2=create_proc_entry(TEST_CASE, 0666, wx_procdir);

	proc_entry1->read_proc = wx_proc_read;
	proc_entry1->write_proc = wx_proc_write;

	proc_entry2->read_proc = test_proc_read;
	proc_entry2->write_proc = test_proc_write;
	printk("create  wx proc done ...\n");
	return 0;
}


static void  test_proc_exit(void)
{

	remove_proc_entry(RUN_TIMES, wx_procdir);
	remove_proc_entry(TEST_CASE, wx_procdir);
	remove_proc_entry(WX_PROCDIR, NULL);
	printk("removing wx proc_dir done ...\n");
}

static void __exit packet_exit(void)
{
	unregister_netdevice_notifier(&packet_netdev_notifier);
	unregister_pernet_subsys(&packet_net_ops);
	sock_unregister(PF_PACKET);
	proto_unregister(&packet_proto);
	test_proc_exit();
}

static int __init packet_init(void)
{
	int rc = proto_register(&packet_proto, 0);
	if (rc != 0)
		goto out;
	test_proc_init();
	sock_register(&packet_family_ops);
	register_pernet_subsys(&packet_net_ops);
	register_netdevice_notifier(&packet_netdev_notifier);
out:
	return rc;
}

module_init(packet_init);
module_exit(packet_exit);
MODULE_LICENSE("GPL");
MODULE_ALIAS_NETPROTO(PF_PACKET);

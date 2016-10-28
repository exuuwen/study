#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/gfp.h>
#include <linux/vmalloc.h>
#include <linux/if_ether.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <net/ip.h>
#include <linux/icmp.h>
#include <net/icmp.h>
#include <net/route.h>
#include <net/ip.h>

#define  WX_DEBUG
#ifdef   WX_DEBUG
# define wx_debug(fmt, args...)  printk(fmt, ## args)
#else
# define wx_debug(fmt, args...)
#endif

#define SENDPACKET_MAJOR 233    	
#define GTP_U_PORT  2152
#define SIZE  152

unsigned char smac[ETH_HLEN] = {0x00, 0x0c, 0x29, 0x0e, 0x4f, 0x26};
unsigned char dmac[ETH_HLEN] = {0x00, 0x0c, 0x29, 0x21, 0x2d, 0x49};

struct packet_t
{
	unsigned char *data;
	unsigned short  length; 
};
                    
static struct packet_t packet;   

/* fake rtable used for ip_fragment */
static struct rtable fake_rtable;

static unsigned int fake_mtu(const struct dst_entry *dst)
{
    return dst->dev->mtu;
}

static struct dst_ops fake_dst_ops = {
    .family = AF_INET,
    .protocol = cpu_to_be16(ETH_P_IP),
    .mtu = fake_mtu,
};

static void fake_rtable_init(void)
{
    struct rtable *rt = &fake_rtable;

    atomic_set(&rt->dst.__refcnt, 1);
    rt->dst.path = &rt->dst;
    rt->dst.ops = &fake_dst_ops;
}

static void fake_rtable_init_dev(struct net_device *dev)
{
    fake_rtable.dst.dev = dev;
}

static void skb_rtable_init(struct sk_buff *skb)
{
    skb_dst_set_noref(skb, &(fake_rtable.dst));
}

int mysend(struct sk_buff *skb)
{
    struct ethhdr  *ethhdr;
    int ret;

    skb_push(skb, ETH_HLEN);
    ethhdr = (struct ethhdr*)skb->data;
    memcpy (ethhdr->h_dest, dmac, ETH_ALEN);
    memcpy (ethhdr->h_source, smac, ETH_ALEN);
    ethhdr->h_proto = __constant_htons(ETH_P_IP);

    ret = dev_queue_xmit(skb);
    if(ret > 0)
    {	
        wx_debug("dev_queue_xmit failed\n");
        return -1;
    }
	
    return 0;	
}

int fragsend(struct sk_buff *skb)
{
    int ret;

    ret = mysend(skb);
    if (ret)	
    {
        wx_debug("fragsend failed\n");
        return -1;
    }
	
    wx_debug("fragsend success\n");
    return 0;
}

struct sk_buff* make_skb(struct sk_buff_head *queue)
{
    struct sk_buff *tmp_skb;
    struct sk_buff **tail_skb;
    struct sk_buff *skb = NULL;

    if ((skb = __skb_dequeue(queue)) == NULL)
        return NULL;
    tail_skb = &(skb_shinfo(skb)->frag_list);

    // move skb->data to ip header from ext header 
    if (skb->data < skb_network_header(skb))
        __skb_pull(skb, skb_network_offset(skb));
    while ((tmp_skb = __skb_dequeue(queue)) != NULL) 
    {
        __skb_pull(tmp_skb, sizeof(struct iphdr));
        //printk("skb_network_header_len(skb): %d\n", skb_network_header_len(skb));
        *tail_skb = tmp_skb;
        tail_skb = &(tmp_skb->next);
        skb->len += tmp_skb->len;
        skb->data_len += tmp_skb->len;
        skb->truesize += tmp_skb->truesize;
        tmp_skb->destructor = NULL;
        tmp_skb->sk = NULL;
    }
    
    return skb;
}


static struct sk_buff* fill_skb_with_sg(struct net_device *dev, int hard_checksum)
{
    int offset, len, nr_frags, len_max, mtu, fragheaderlen;
    int maxfraglen, copy, ll_rs, to_write, hh_len = 0;
    void* data;
    struct page *page;
    struct sk_buff_head queue;
    struct sk_buff *skb = NULL;
    struct iphdr *iph;
    struct udphdr *udph;
 
    __skb_queue_head_init(&queue);

    ll_rs = LL_RESERVED_SPACE_EXTRA(dev, 0);

    if (hard_checksum && packet.length - ETH_HLEN <= dev->mtu)
        hh_len = sizeof(struct iphdr) + sizeof(struct udphdr);
    else if (packet.length - ETH_HLEN > dev->mtu)
    {
        hard_checksum = 0;
        hh_len = sizeof(struct iphdr);
    }
    
    skb = dev_alloc_skb(ll_rs + hh_len);
    if (unlikely(skb == NULL))
    {
        printk("Forward packet, fatal error, no memory in the kernel\n");
        return NULL;
    }

    skb_reserve(skb, ll_rs);

    skb->protocol = __constant_htons(ETH_P_IP);
    skb->dev = dev;
    skb->pkt_type = PACKET_OUTGOING;

    if (hh_len)
    {
        skb_put(skb, hh_len);
        skb_reset_network_header(skb);
     
        skb_copy_to_linear_data_offset(skb, 0, packet.data + ETH_HLEN, hh_len);
     
        iph = ip_hdr(skb);
        if (hard_checksum)
        {
            skb_pull(skb, sizeof(struct iphdr));
            skb_reset_transport_header(skb);
            skb_push(skb, sizeof(struct iphdr));
            udph = udp_hdr(skb);

            udph->check = ~csum_tcpudp_magic(iph->saddr, iph->daddr, sizeof(struct udphdr) + SIZE, iph->protocol, 0);
            skb->csum_start = skb_transport_header(skb) - skb->head;
            skb->csum_offset = offsetof(struct udphdr, check);
            skb->ip_summed = CHECKSUM_PARTIAL;
            printk("CHECKSUM_PARTIAL uh->check:0x%x\n", udph->check);
        }
        else
        {
            udph  = (struct udphdr *)(packet.data + ETH_HLEN + sizeof(struct iphdr));
            skb->csum = csum_partial(packet.data + ETH_HLEN + sizeof(struct iphdr), sizeof(struct udphdr) + SIZE, 0);
            udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, sizeof(struct udphdr) + SIZE, iph->protocol, skb->csum);
            skb->ip_summed = CHECKSUM_NONE;
            printk("flag list CHECKSUM_NONE uh->check:0x%x\n", udph->check);
        }
    }
    else
    {
        iph = (struct iphdr *)(packet.data + ETH_HLEN);
        udph  = (struct udphdr *)(packet.data + ETH_HLEN + sizeof(struct iphdr));
        skb->csum = csum_partial(packet.data + ETH_HLEN + sizeof(struct iphdr), sizeof(struct udphdr) + SIZE, 0);
        udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, sizeof(struct udphdr) + SIZE, iph->protocol, skb->csum);
        skb->ip_summed = CHECKSUM_NONE;
        printk("no hardware CHECKSUM_NONE uh->check:0x%x\n", udph->check);
    }

    len = packet.length - ETH_HLEN - hh_len;
    mtu = skb->dev->mtu;
    fragheaderlen = sizeof(struct iphdr);
    maxfraglen = ((mtu - fragheaderlen) & ~7) + fragheaderlen;
    
    data = packet.data + ETH_HLEN + hh_len;
    
    while (len > 0)
    {
        if (!skb)
        {
            skb = dev_alloc_skb(ll_rs + sizeof(struct iphdr));
            if (unlikely(skb == NULL))
            {
                printk("Forward packet, fatal error, no memory in the kernel\n");
                return NULL;
            }
            skb_reserve(skb, ll_rs);
            skb_put(skb, hh_len);
            skb_reset_network_header(skb);
        }

        copy = mtu - skb->len;
        if (copy < len)
        {
            copy = maxfraglen - skb->len;
           // printk("not enough..............\n");
        }
        if (copy > len)
        {
            //printk("copy > len\n");
            copy = len;
        }
        
        to_write = copy;
        while (to_write > 0)
        {
            offset = offset_in_page(data);
            len_max = PAGE_SIZE - offset;
            page = vmalloc_to_page(data);
            copy = ((to_write > len_max) ? len_max : to_write);
          
            nr_frags = skb_shinfo(skb)->nr_frags;
            if (unlikely(nr_frags >= MAX_SKB_FRAGS))
            {
                printk("Packet exceeds the number of skb frags(%lu)\n", MAX_SKB_FRAGS);
                return NULL;
            }

            get_page(page);
            skb_fill_page_desc(skb, nr_frags, page, offset, copy);
            len -= copy;
            to_write -= copy;
            data += copy;
            skb->data_len += copy;
            skb->len += copy;
            skb->truesize += copy;
        }

        __skb_queue_tail(&queue, skb);
        
        skb = NULL;
    }
   
    skb = make_skb(&queue); 
    
    return skb;
}

static struct sk_buff* fill_skb_with_data(struct net_device *dev, int hard_checksum)
{
    int len, mtu, fragheaderlen;
    int maxfraglen, ll_rs, tmp_len = 0;
    void* data;
    struct sk_buff_head queue;
    struct sk_buff *skb = NULL;
    struct iphdr *iph;
    struct udphdr *udph;
 
    __skb_queue_head_init(&queue);

    ll_rs = LL_RESERVED_SPACE_EXTRA(dev, 0);

    if (packet.length - ETH_HLEN > dev->mtu)
        hard_checksum = 0;
    
    len = packet.length - ETH_HLEN ;
    mtu = dev->mtu;
    fragheaderlen = sizeof(struct iphdr);
    maxfraglen = ((mtu - fragheaderlen) & ~7) + fragheaderlen;
    
    data = packet.data + ETH_HLEN;
    tmp_len = (len > maxfraglen) ? maxfraglen : len; 
    skb = dev_alloc_skb(ll_rs + tmp_len);
    if (unlikely(skb == NULL))
    {
        printk("Forward packet, fatal error, no memory in the kernel\n");
        return NULL;
    }

    skb_reserve(skb, ll_rs);

    skb->protocol = __constant_htons(ETH_P_IP);
    skb->dev = dev;
    skb->pkt_type = PACKET_OUTGOING;

    skb_put(skb, tmp_len);
    skb_reset_network_header(skb);
     
    skb_copy_to_linear_data_offset(skb, 0, data, tmp_len);
    len -= tmp_len;
    data += tmp_len;
 
    skb_pull(skb, sizeof(struct iphdr));
    skb_reset_transport_header(skb);
    skb_push(skb, sizeof(struct iphdr));

    iph = ip_hdr(skb);
    udph = udp_hdr(skb);
    if (hard_checksum)
    {
        udph->check = ~csum_tcpudp_magic(iph->saddr, iph->daddr, sizeof(struct udphdr) + SIZE, iph->protocol, 0);
        skb->csum_start = skb_transport_header(skb) - skb->head;
        skb->csum_offset = offsetof(struct udphdr, check);
        skb->ip_summed = CHECKSUM_PARTIAL;
        printk("CHECKSUM_PARTIAL uh->check:0x%x\n", udph->check);
    }
    else
    {
        skb->csum = csum_partial(packet.data + ETH_HLEN + sizeof(struct iphdr), sizeof(struct udphdr) + SIZE, 0);
        udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, sizeof(struct udphdr) + SIZE, iph->protocol, skb->csum);
        skb->ip_summed = CHECKSUM_NONE;
        printk("flag list CHECKSUM_NONE uh->check:0x%x\n", udph->check);
    }

    __skb_queue_tail(&queue, skb);
    skb = NULL;

    while (len > 0)
    {
        tmp_len = (len > maxfraglen - fragheaderlen) ? (maxfraglen - fragheaderlen) : len; 
        skb = dev_alloc_skb(ll_rs + sizeof(struct iphdr) + tmp_len);
        if (unlikely(skb == NULL))
        {
            printk("Forward packet, fatal error, no memory in the kernel\n");
            return NULL;
        }
        skb_reserve(skb, ll_rs);
        skb_put(skb, sizeof(struct iphdr) + tmp_len);
        skb_reset_network_header(skb);
    
        skb_copy_to_linear_data_offset(skb, sizeof(struct iphdr), data, tmp_len);
        len -= tmp_len;
        data += tmp_len;

         __skb_queue_tail(&queue, skb);
        
         skb = NULL;
    }
   
    skb = make_skb(&queue); 
    
    return skb;
}

void create_packet(const struct net_device *dev)
{
    struct ethhdr  *ethhdr;
    struct iphdr *iph;
    struct udphdr *uh;
 
    packet.data = vmalloc(6*1024);	
    memset(packet.data, 0, 6*1024);	
    ethhdr = (struct ethhdr*)packet.data;
	
    memcpy (ethhdr->h_dest, dmac, ETH_ALEN);
    memcpy (ethhdr->h_source, smac, ETH_ALEN);
    ethhdr->h_proto = __constant_htons(ETH_P_IP);

    packet.length = ETH_HLEN;
        
    iph =(struct iphdr *)(packet.data + ETH_HLEN);
    iph->version = 4;
    iph->ihl = 5;
    iph->tos = 0;
    iph->tot_len = htons(SIZE + sizeof(struct iphdr) + sizeof(struct udphdr));
    iph->id = 0;
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_UDP;

    iph->daddr = 0x933ca8c0;
    iph->saddr = dev->ip_ptr->ifa_list->ifa_address;
    iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);

    packet.length += sizeof(struct iphdr);
      
    uh = (struct udphdr *)(packet.data + ETH_HLEN + sizeof(struct iphdr));
    uh->source = htons(0x1234);
    uh->dest = htons(GTP_U_PORT);
    uh->len = htons(sizeof(struct udphdr) + SIZE);
    uh->check = 0;
    packet.length += sizeof(struct udphdr);

    packet.length += SIZE;
	
    printk("packet.len:%d\n", packet.length);
   
}

int send_packet(struct net_device *dev)
{	
    int ret;	
    netdev_features_t features;
    struct sk_buff *skb = NULL;

    skb = dev_alloc_skb(0);
    if (NULL == skb)
    {
        wx_debug("tmp skb alloc failed\n");
        return -1;
    }	
    skb->dev = dev;
    skb->protocol = __constant_htons(ETH_P_IP);
    features = netif_skb_features(skb);
	
    consume_skb(skb);	

    if ((features & NETIF_F_SG))
    {		
        skb = fill_skb_with_sg(dev, (features & NETIF_F_ALL_CSUM));
        if (skb == NULL)
        {
            wx_debug("fill_skb_sg fail\n");
            return -1;   
        }
        wx_debug("sg mode: fill skb sg success\n");
    }
    else
    {
        skb = fill_skb_with_data(dev, (features & NETIF_F_ALL_CSUM));
        if (skb == NULL)
        {
            wx_debug("fill_skb_data fail\n");
            return -1;   
        }
        wx_debug("non sg mode: fill skb data success\n");
    }
	
    printk("skb->len:%d\n", skb->len);

    if (skb->len > skb->dev->mtu)
    {
        fake_rtable_init_dev(dev);
        skb_rtable_init(skb);
        ret = ip_fragment(skb, fragsend);
        if (ret)
        {
            wx_debug("ip_fragment fail:%d\n", ret);
            kfree_skb(skb);
            return -1;
        }
    }
    else
    {
        ret = mysend(skb);
        if(ret < 0)
        {	
            wx_debug("mysend failed\n");
            kfree_skb(skb);
            return -1;
        }
    }
	
    wx_debug("dev_queue_xmit ok\n");
	
    return 0;
}


int sendpacket_init(void)
{
    int ret;
    struct net_device *dev = __dev_get_by_name(&init_net, "eth0");
    
    fake_rtable_init();	
    create_packet(dev);

    ret = send_packet(dev);
    if (ret != 0)
    {
        wx_debug("send new packet fail\n");
        return 0;
    }

    return 0;
}


void sendpacket_exit(void)
{
    vfree(packet.data);
}

MODULE_AUTHOR("xu.wen@ericsson.com");
MODULE_LICENSE("Dual BSD/GPL");


module_init(sendpacket_init);
module_exit(sendpacket_exit);

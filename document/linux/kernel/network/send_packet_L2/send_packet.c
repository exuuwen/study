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
#define SIZE  32

unsigned char smac[ETH_HLEN] = {0x00, 0x0c, 0x29, 0x0e, 0x4f, 0x26};
unsigned char dmac[ETH_HLEN] = {0x00, 0x0c, 0x29, 0x21, 0x2d, 0x49};

struct packet_t
{
	unsigned char *data;
	unsigned short  length; 
};
                    
static struct packet_t packet;   



static struct sk_buff* make_skb(struct net_device *dev)
{
    int ll_rs, tmp_len = 0;
    void* data;
    struct sk_buff *skb = NULL;
 
    ll_rs = 20;
    
    data = packet.data + ETH_HLEN;
    tmp_len = packet.length - ETH_HLEN ;
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
 
    skb_pull(skb, sizeof(struct iphdr));
    skb_reset_transport_header(skb);
    skb_push(skb, sizeof(struct iphdr));

    
    return skb;
}

void create_packet(const struct net_device *dev)
{
    struct ethhdr  *ethhdr;
    struct iphdr *iph;
    struct udphdr *uh;
 
    printk("1\n");
    packet.data = vmalloc(6*1024);	
    memset(packet.data, 0, 6*1024);	
    ethhdr = (struct ethhdr*)packet.data;
	
    memcpy (ethhdr->h_dest, dmac, ETH_ALEN);
    memcpy (ethhdr->h_source, smac, ETH_ALEN);
    ethhdr->h_proto = __constant_htons(ETH_P_IP);

    packet.length = ETH_HLEN;
    printk("2\n");
    iph =(struct iphdr *)(packet.data + ETH_HLEN);
    iph->version = 4;
    iph->ihl = 5;
    iph->tos = 0;
    iph->tot_len = htons(SIZE + sizeof(struct iphdr) + sizeof(struct udphdr));
    iph->id = 0;
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_UDP;
    printk("3\n");
    iph->daddr = 0x101a8c0;//0x903ca8c0;
    iph->saddr = 0x201a8c0;//0x8d3ca8c0;
    iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
    printk("4\n");
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

int send_packet(struct net_device *dev)
{	
    int ret;
    struct sk_buff* skb = make_skb(dev);
    ret = mysend(skb);
    if(ret < 0)
    {	
        wx_debug("mysend failed\n");
        kfree_skb(skb);
        return -1;
    }
	
    wx_debug("dev_queue_xmit ok\n");
	
    return 0;
}


int sendpacket_init(void)
{
    int ret;

    struct net_device *dev = __dev_get_by_name(&init_net, "eth0");
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


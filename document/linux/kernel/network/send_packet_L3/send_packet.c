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
   	
#define GTP_U_PORT  2152
#define SIZE  200

struct packet_t
{
	unsigned char *data;
	unsigned short  length; 
};
                    
static struct packet_t packet;   


static struct sk_buff* make_skb(void)
{
    int ll_rs, tmp_len = 0;
    void* data;
    struct sk_buff *skb = NULL;
 
    ll_rs = 50;
    
    data = packet.data;
    tmp_len = packet.length;
    skb = dev_alloc_skb(ll_rs + tmp_len);
    if (unlikely(skb == NULL))
    {
        printk("Forward packet, fatal error, no memory in the kernel\n");
        return NULL;
    }

    skb_reserve(skb, ll_rs);

    skb->protocol = __constant_htons(ETH_P_IP);
    skb->pkt_type = PACKET_OUTGOING;

    skb_put(skb, tmp_len);
    skb_reset_network_header(skb);
     
    skb_copy_to_linear_data_offset(skb, 0, data, tmp_len);
 
    skb_pull(skb, sizeof(struct iphdr));
    skb_reset_transport_header(skb);
    skb_push(skb, sizeof(struct iphdr));
    
    return skb;
}

static void create_packet(void)
{
    //struct ethhdr  *ethhdr;
    struct iphdr *iph;
    struct udphdr *uh;
 
    packet.data = vmalloc(6*1024);	
    memset(packet.data, 0, 6*1024);	

    iph =(struct iphdr *)(packet.data);
    iph->version = 4;
    iph->ihl = 5;
    iph->tos = 0;
    iph->tot_len = htons(SIZE + sizeof(struct iphdr) + sizeof(struct udphdr));
    iph->id = 0;
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_UDP;
    iph->daddr = 0x8a82a8c0;//0x903ca8c0;
    iph->saddr = 0x8082a8c0;//0x8d3ca8c0;
    iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
    packet.length = sizeof(struct iphdr);
      
    uh = (struct udphdr *)(packet.data + sizeof(struct iphdr));
    uh->source = htons(0x1234);
    uh->dest = htons(GTP_U_PORT);
    uh->len = htons(sizeof(struct udphdr) + SIZE);
    uh->check = 0;
    packet.length += sizeof(struct udphdr);

    packet.length += SIZE;
	
    printk("packet.len:%d\n", packet.length);
   
}


static struct rtable * get_output_route(__be32 daddr, u8 rtos)
{
    struct rtable *rt = NULL;
    
    struct flowi4 fl = { .flowi4_oif = 0, .daddr = daddr, .saddr = 0, .flowi4_tos = rtos};

    rt = ip_route_output_key(&init_net, &fl);
    if (rt == NULL) 
    {
        printk("ip_route_output_key fail\n");
    }
   
    return rt;
}

static int my_output(struct sk_buff *skb)
{
    struct iphdr *iph = ip_hdr(skb);
    struct rtable *rt = get_output_route(iph->daddr, RT_TOS(iph->tos));
    if (rt == NULL) 
    {
        printk(KERN_INFO "rt is NULL.\n");
        return -1;
    }

    skb_dst_drop(skb);
    skb_dst_set(skb, &rt->dst);

    dst_output(skb);
	
    return 0;	
}

static int output_packet(void)
{	
    int ret;
    struct sk_buff* skb = make_skb();
    ret = my_output(skb);
    if(ret < 0)
    {	
        wx_debug("my output failed\n");
        kfree_skb(skb);
        return -1;
    }
	
    wx_debug("my output ok\n");
	
    return 0;
}


int sendpacket_init(void)
{
    int ret;

    create_packet();
    ret = output_packet();
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


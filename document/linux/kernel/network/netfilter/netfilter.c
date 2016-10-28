#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/netdevice.h> 
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
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

static struct nf_hook_ops nfho;
static atomic_t packt_count;

static int my_filter_3(const struct sk_buff *skb)
{
        struct ethhdr *eth_header = (struct ethhdr*)(skb->data - sizeof(struct ethhdr));
        struct iphdr *ip_header = (struct iphdr *)(skb->data);
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

static unsigned int hook_func(unsigned int hooknum, 
                    struct sk_buff *skb, 
                    const struct net_device *in, 
                    const struct net_device *out, 
                    int (*okfn)(struct sk_buff *)) 
{ 
		int count;

        	atomic_inc(&packt_count);
		count = atomic_read(&packt_count);

		if(my_filter_3(skb))
		{
			printk("we recive 2152 packet, len:%d, data len:%d\n", skb->len, skb->data_len);
					
		}
		else if(count%10 == 0)
		{
			printk("we recive 10 packet len:%d, data len:%d\n", skb->len, skb->data_len);
		}
		
		return NF_ACCEPT;
} 
 

static int __init filter_module(void) 
{ 
        	
	printk("in the filter module\n");

        nfho.hook          = hook_func;                  
        nfho.hooknum    = NF_INET_PRE_ROUTING; 
        nfho.pf              = PF_INET; 
        nfho.priority = NF_IP_PRI_FIRST;     
 
        nf_register_hook(&nfho); 
	
	atomic_set(&packt_count,0);

        return 0; 
} 

static void __exit exit_module(void) 
{ 
	printk("out the filter cleanup module\n");
        nf_unregister_hook(&nfho); 
}

module_init(filter_module);
module_exit(exit_module);
MODULE_LICENSE("GPL");

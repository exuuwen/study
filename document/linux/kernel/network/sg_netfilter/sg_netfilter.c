#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/udp.h>
#include <linux/if.h>
#include <linux/in.h>

#define PERMIT_PORT 80

static struct nf_hook_ops nfho;



unsigned int hook_fund(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int(*okfn)(struct sk_buff*))
{
    struct udphdr *udph;
    struct iphdr *iph;
    int i;
    struct sk_buff *tskb;

    iph = ip_hdr(skb);
    if (iph->protocol == IPPROTO_UDP)
    {
        udph = udp_hdr(skb);
        if(udph->dest == htons(2152))
        {
            printk("read v send 2152...skb->ip_summed:%d........\n", skb->ip_summed);
            if(skb->data == skb_transport_header(skb))
		printk("skb_transport_header(skb);\n");
            if(skb->data == skb_network_header(skb))
		printk("skb_network_header(skb);\n");
            printk("skb->len:%d, skb_headlen:%d, skb->data_len:%d, nr_frag:%d\n", skb->len, skb_headlen(skb), skb->data_len, (int)skb_shinfo(skb)->nr_frags);
            for (i = (int)skb_shinfo(skb)->nr_frags - 1; i >= 0; i--)
		printk("skb%d page size: %d\n", (int)skb_shinfo(skb)->nr_frags - i, skb_frag_size(&skb_shinfo(skb)->frags[i]));
            tskb = skb;
            if(skb_shinfo(tskb)->frag_list)
            {
		
                
		tskb = skb_shinfo(tskb)->frag_list;
		while(tskb)
		{
			printk(".....there is frag_list...\n");
			if(tskb->data == skb_transport_header(tskb))
				printk("skb_transport_header(tskb);\n");
			if(tskb->data == skb_network_header(tskb))
				printk("skb_network_header(tskb);\n");
			printk("tskb->len:%d, tskb_headlen:%d, tskb->data_len:%d, nr_frag:%d\n", tskb->len, skb_headlen(tskb), tskb->data_len, (int)skb_shinfo(tskb)->nr_frags);
		        for (i = (int)skb_shinfo(tskb)->nr_frags - 1; i >= 0; i--)
				printk("tskb%d page size: %d\n", (int)skb_shinfo(tskb)->nr_frags - i, skb_frag_size(&skb_shinfo(tskb)->frags[i]));
			printk(".....there is frag_list end...\n");
			tskb = tskb->next;		
		}
            }
            else
            {
                printk("there is non frag_list\n");
            }
            

            printk("end v send 2152..............\n");
        }
    }

    return NF_ACCEPT;
}
int sg_init(void)
{
    nfho.hook = hook_fund;
    nfho.hooknum = NF_INET_LOCAL_OUT;//NF_IP_LOCAL_OUT;
    nfho.pf = NFPROTO_IPV4;//PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;
    nf_register_hook(&nfho);

    printk("sg init success\n");

    return 0;
}

void sg_exit(void)
{
    nf_unregister_hook(&nfho);
     printk("sg exit success\n");
}

module_init(sg_init);
module_exit(sg_exit);

MODULE_LICENSE("GPL");

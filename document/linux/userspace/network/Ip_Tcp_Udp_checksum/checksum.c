
int dpi_skb_csum(struct sk_buff *skb)
{
	//struct iphdr *iph = ip_hdr(skb);
	struct iphdr *iph = (struct iphdr *)skb->data;
	iph = skb->data; 
	skb->len = skb->tail - skb->data;
	//printk("\n");
	//printk("before: 0x%4x\n",iph->check);
	//skbprint_all(skb);
	dpi_panic("hello world\n");	
	//L4 CHECK SUM
	skb_pull(skb,IP_HEAD_LENTH);
	if(iph->protocol == IPPROTO_UDP)//UDP
	{
		//struct udphdr *uh = udp_hdr(skb);
		struct udphdr *uh = (struct udphdr *)(skb->data);
		//printk("uoo: 0x%4x\n", uh->check);
		uh->check = 0;
		skb->csum = csum_partial(skb->data, skb->len, 0);//计算TCP 头和TCP PAYLOAD的CSUM
		//skb->ip_summed = CHECKSUM_PARTIAL;//表示PARTIAL 已经计算过了
		uh->check = csum_tcpudp_magic(iph->saddr, iph->daddr, skb->len,
			      iph->protocol,skb->csum);
		if(!uh->check)
		{
			dpi_panic("warning udp csum is 0 system error! ask MR.BEN\n");
		}
		//skb->ip_summed = CHECKSUM_COMPLETE;//表示CSUM  已经计算好了
		//printk("udp: 0x%4x\n", uh->check);
	}
	else if(iph->protocol == IPPROTO_TCP)
	{
		//struct tcphdr *th = tcp_hdr(skb);
		struct tcphdr *th = (struct tcphdr *)(skb->data);
		//printk("too: 0x%4x\n", th->check);
		th->check = 0;
		skb->csum = csum_partial(skb->data, skb->len, 0);//计算TCP 头和TCP PAYLOAD的CSUM
		//skb->ip_summed = CHECKSUM_PARTIAL;//表示PARTIAL 已经计算过了
		//计算伪头的CSUM,并加上skb->csum(之前的结果)
		th->check = csum_tcpudp_magic(iph->saddr, iph->daddr,skb->len,
		      iph->protocol,skb->csum);
		if(!th->check)
		{
			dpi_panic("warning udp csum is 0 system error! ask MR.BEN\n");
		}
		//skb->ip_summed = CHECKSUM_COMPLETE;//表示CSUM  已经计算好了
		//printk("tcp: 0x%4x\n", th->check);
	}
	else
	{
		//printk("other\n");
		kfree(skb);
		return 0;
	}

	//L3 CHECH SUM
	skb_push(skb,IP_HEAD_LENTH);
	iph->check = 0;
	iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
		
	//printk("after: 0x%4x\n",iph->check);
	//skbprint_all(skb);

	//L2
	skb->ip_summed = CHECKSUM_COMPLETE;
	skb_push(skb,ETH_HLEN);
	
	skb->dev=dev_get_by_name("eth1");
	dev_queue_xmit(skb);
	return 0;
}

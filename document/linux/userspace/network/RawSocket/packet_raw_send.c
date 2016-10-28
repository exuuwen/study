#include <stdlib.h> 
#include<sys/types.h>
#include<sys/time.h>
#include<sys/ioctl.h>
#include<sys/socket.h>
#include<linux/types.h>
#include<netinet/in.h>
#include<netinet/udp.h>
#include<netinet/ip.h>
#include<netpacket/packet.h>
#include<net/ethernet.h>
#include<arpa/inet.h>
#include<string.h>
#include<signal.h>
#include<net/if.h>
#include<stdio.h>
#include<sys/uio.h>
#include<fcntl.h>
#include<unistd.h>
#include<linux/filter.h>
#include<errno.h>
#include <assert.h>

#define MTU 1500

unsigned char packet_edst[ETH_ALEN] = {0x00, 0x0c, 0x29, 0x0e, 0x4f, 0x26};//must be rihgt for receive parter
unsigned char packet_src[ETH_ALEN] = {0x90, 0xe2, 0xba, 0x00, 0x69, 0x59};

unsigned short ip_sum_calc(unsigned short length, unsigned short *buf)
{
    unsigned int cksum = 0;
    unsigned int nbytes = length;
    
    assert(length%2 == 0);
    
    for (; nbytes > 1; nbytes -= 2)
    {
        cksum += *buf++;
    }

    while (cksum >> 16)
    {
        cksum = (cksum >> 16) + (cksum & 0xffff);
    }

    return (unsigned short) (~cksum);
}


int sock;
static void raw_prepare_sockaddr(struct sockaddr_ll *sa)
{
	memset(sa, 0, sizeof(struct sockaddr_ll));

	sa->sll_family 	= PF_PACKET;
	sa->sll_protocol = htons(ETH_P_IP);

	struct ifreq ifstruct;
   	strcpy(ifstruct.ifr_name, "eth11");
   	ioctl(sock, SIOCGIFINDEX, &ifstruct);
    	sa->sll_ifindex = ifstruct.ifr_ifindex;
	printf("index:%d\n", sa->sll_ifindex);

}


static int Open()
{
	// Only used for sending UDP packets
	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
	if (sock == -1)
	{
		printf("Create raw socket failed\n");
		return -1;
	}
	struct sockaddr_ll sa;
	
	raw_prepare_sockaddr(&sa);


	// bind address and port to socket
/*	if ((bind(sock, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll))) < 0)
	{
		printf("Bind raw socket failed, errno %d: %s\n", errno, strerror(errno));
		return -1;
	}
*/
	//
	// let the kernel know the IP header is included in the data
	//
/*	
	int tmp = 1;
	if (setsockopt(sock, 0, IP_HDRINCL, &tmp, sizeof(tmp)) < 0)
	{
		printf("set IP_HDRINCL for raw socket failed\n");
		return -1;			
	}
*/
	return 0;
}

int Send(unsigned char* buf, unsigned int len, unsigned int dest_ip, unsigned short dest_port)
{
	unsigned int total_len = 0;
	struct sockaddr_in to;
	struct iphdr * ip_header;
	struct udphdr * udp_header;
	struct ethhdr * mac_header;
	int ret;
	unsigned char *temp = buf;
	
	static unsigned short ip_id_gen = 1;
	total_len = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + len;	
	
	mac_header = (struct ethhdr *) buf;
	memcpy(mac_header->h_dest, packet_edst, ETH_ALEN);
	memcpy(mac_header->h_source, packet_src, ETH_ALEN);
	mac_header->h_proto = htons(ETH_P_IP);

	ip_header = (struct iphdr *) (buf + sizeof(struct ethhdr));
	ip_header->ihl = 5;
	ip_header->version = 4;
	ip_header->tos = 0;

	// ip length contains the total length of ip header, tcp/udp header and payload in bytes.
	ip_header->tot_len = htons(total_len - sizeof(struct ethhdr));
	ip_header->id = 0; // the value doesnt matter here
	ip_header->ttl = 64;
	ip_header->frag_off = 0x40;
	ip_header->protocol = IPPROTO_UDP;
	
	ip_header->daddr = htonl(dest_ip);

	// fake ip address, may different with ServerIPAddr.
	ip_header->saddr = htonl(0x11223344); 
	ip_header->check = 0;
	ip_header->check = (ip_sum_calc(ip_header->ihl*4, (unsigned short*)ip_header));// must fill it  
	printf("chesum 0x%x\n", ip_header->check);
	//
	// udp header filling
	//
	udp_header = (struct udphdr *) (buf + sizeof(struct iphdr) + sizeof(struct ethhdr));	

	// fake port, may different with ServerPort.
	udp_header->source = htons(32255);
	printf("dest port:%d\n", dest_port);
	udp_header->dest = htons(dest_port);

	// the length of udp header and payload data in bytes.
	udp_header->len = htons(sizeof(struct udphdr) + len);  
	udp_header->check = 0;
	
	struct sockaddr_ll sa;
	memset(&sa, 0, sizeof(struct sockaddr_ll));
	
	sa.sll_family  = PF_PACKET;
	sa.sll_protocol = htons(ETH_P_IP);
	
	struct ifreq ifstruct;
   	strcpy(ifstruct.ifr_name, "eth0");//choose the eth to send the packet
   	ioctl(sock, SIOCGIFINDEX, &ifstruct);
//	sa.sll_halen	= ETH_ALEN;    
    sa.sll_ifindex = ifstruct.ifr_ifindex;
//    memcpy(sa.sll_addr, packet_src, ETH_ALEN);

	if(total_len - sizeof(struct ethhdr) <= MTU)
	{
		ret = sendto(sock, buf, total_len, 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll));
		if (ret < 0)
		{
			printf("RawSocket sendto ERROR : return value %d, %d, %s\n", ret, errno, strerror(errno));
	    	return ret;
		}

		return ret;
	}
	else
	{
		unsigned short offset = 0;
		int nfrags;
		unsigned int send_len = 0;
		
		ip_header->id = ip_id_gen;
		++ip_id_gen;
		if(!ip_id_gen) ++ip_id_gen;
		ip_header->frag_off = htons((offset >> 3) |  0x2000);// More fragments.
		ip_header->tot_len = htons(MTU);
		// XXX Most likely doesn't need to be filled in -- see the raw(7) man page.
		ip_header->check = 0;
		ip_header->check = ip_sum_calc(ip_header->ihl*4, (unsigned short*)ip_header);
		
		ret = sendto(sock, buf, MTU + sizeof(struct ethhdr), 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll));
		if (ret < 0)
		{
		    printf("RawSocket sendto ERROR : return value %d, %d, %s\n", ret, errno, strerror(errno));
		    return ret;
		}
		send_len = ret;
		
		const hlen = sizeof(struct ethhdr) + sizeof(struct iphdr);
		unsigned int dlen = 1480;
		unsigned char * temp = buf + dlen;
		
		for(offset=dlen+hlen; offset<total_len; temp+=dlen, offset+=dlen)
		{
			if(offset + dlen >= total_len)
			{
				dlen = total_len - offset;
				ip_header->frag_off = htons((offset - hlen) >> 3);
			}
			else
			{
				ip_header->frag_off = htons(((offset - hlen) >> 3) |  0x2000);
			}
			
			ip_header->tot_len = htons(dlen + hlen - sizeof(struct ethhdr));
			
			ip_header->check = 0;
			ip_header->check = ip_sum_calc(ip_header->ihl*4, (unsigned short*)ip_header);
			
			memcpy(temp, (unsigned char *)mac_header, hlen);
			ret = sendto(sock, temp, dlen + hlen, 0, (struct sockaddr *)&sa, sizeof(struct sockaddr_ll));
			if (ret < 0)
			{
				printf("RawSocket sendto ERROR : return value %d, %d, %s\n", ret, errno, strerror(errno));
		    	return ret;
			}
			send_len += ret;
		}
		// Sending of fragmented packet should end up here.
		// Which result of sendto should we return?
		return send_len;
	}
}


int main(int argc, char *argv[])
{
	
	if(argc != 4)
	{
		printf("usage:./main ip_src ip_dst port_dst len \n");
		return 0;
	}
	int ret = Open();
	if(ret != 0)
	{
		printf("open error\n");
		return -1;
	}

	unsigned int len = atoi(argv[3]);
	unsigned char *pdu_p = (unsigned char*)malloc(len + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr));
	memset(pdu_p, 0, len + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr));
	memcpy(pdu_p + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr), "test hello", 10);
	unsigned int  ip_dst = inet_addr(argv[1]);
	ret = Send(pdu_p, len, ntohl(ip_dst), atoi(argv[2]));
	if(ret < 0)
	{
		printf("send error\n");
		return -1;
	}

	printf("ret = %d\n", ret);
	free(pdu_p);
	
	close(sock);
	return 0;
}

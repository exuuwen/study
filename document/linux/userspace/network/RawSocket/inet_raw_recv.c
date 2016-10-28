/*af_packet filter: gcc -o packet_filter bpf_image.c packet_filter.c -lpcap*/
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


#define _GNU_SOURCE

#include <sched.h>

#define CMD_LEN 1024

#define ETH_HDR_LEN 14
#define IP_HDR_LEN 20
#define UDP_HDR_LEN 8
#define TCP_HDR_LEN 20

static int sock;

void sig_handler(int sig)
{
	struct ifreq ethreq;
	if(sig == SIGTERM)
		printf("SIGTERMrecieved,exiting...\n");
	else if(sig == SIGINT)
		printf("SIGINTrecieved,exiting...\n");
	else if(sig == SIGQUIT)
		printf("SIGQUITrecieved,exiting...\n");
	// turn off the PROMISCOUS mode
	strncpy(ethreq.ifr_name,"eth0", IFNAMSIZ);
	if(ioctl(sock, SIOCGIFFLAGS, &ethreq) != -1){
		ethreq.ifr_flags &= ~IFF_PROMISC;
		ioctl(sock,SIOCSIFFLAGS,&ethreq);
	}

	close(sock);
	exit(0);
}


static unsigned long packets;


int main(int argc, char *argv[ ])
{
     int offset = 0;
     int n;
     char buf[2048];
     unsigned char*ethhead;
     unsigned char*iphead;
     struct ifreq ethreq;
     struct sigaction sighandle;

	if(argc != 2 )
	{
		printf("%s:ipaddr\n", argv[0]);
		return -1;
	}

	sighandle.sa_flags = 0;
	sighandle.sa_handler = sig_handler;
	sigemptyset(&sighandle.sa_mask);

	sigaction(SIGTERM, &sighandle, NULL);
	sigaction(SIGINT, &sighandle, NULL);
	sigaction(SIGQUIT, &sighandle, NULL);

	if((sock=socket(AF_INET, SOCK_RAW, IPPROTO_UDP))==-1){
		perror("socket");
		exit(1);
	}
    
	struct sockaddr_in srvaddr;
	bzero(&srvaddr, sizeof(srvaddr));

	srvaddr.sin_family = AF_INET;
	//srvaddr.sin_port = atoi(argv[2]);//it is not matter with the port
	srvaddr.sin_addr.s_addr = inet_addr(argv[1]);
	if ((bind(sock, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr))) < 0)
	{
		perror("Bind raw socket failed:");
		return -1;
	}
	//set NIC to promiscous mode,so we can recieve all packets of thenetwork
	/*strncpy(ethreq.ifr_name, "eth0", IFNAMSIZ);
	if(ioctl(sock, SIOCGIFFLAGS, &ethreq)==-1){
		perror("ioctl");
		close(sock);
		exit(1);
	}

	ethreq.ifr_flags|= IFF_PROMISC;
	if(ioctl(sock, SIOCSIFFLAGS, &ethreq)==-1){
		perror("ioctl");
		close(sock);
		exit(1);
	}
	*/	
	while(1)
	{
		n = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
		if(n < (ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN)){
			printf("invalid packet\n");
			continue;
	}
		
		
		
		ethhead = buf;
		iphead = buf;
	//	if((iphead[12]==192) && (iphead[13] == 168) && (iphead[14] == 60) && (iphead[15] == 168))
		{
			packets++;
			printf("Ethernet:MAC[%02X:%02X:%02X:%02X:%02X:%02X]", ethhead[6], ethhead[7], ethhead[8], ethhead[9], ethhead[10], ethhead[11]);
			printf("->[%02X:%02X:%02X:%02X:%02X:%02X] ", ethhead[0], ethhead[1], ethhead[2], ethhead[3], ethhead[4], ethhead[5]);
			printf("type[%04x]\n ",(ntohs(ethhead[12] | ethhead[13] << 8)));

		
			//header length as 32-bit
			printf("IP:Version:%d HeaderLen:%d[%d] ", (*iphead>>4), (*iphead&0x0f), (*iphead&0x0f)*4);
			printf("TOS %d ", iphead[1]);
			printf("TotalLen %d ", (iphead[2] <<8 | iphead[3]));
			printf("id %d ", (iphead[4] <<8 | iphead[5]));
			printf("ttl %d ", iphead[8]);
			printf("frag:0x%x\n", (ethhead[20] << 8 | ethhead[21])); // frag&0x1fff is true just a fragemnt packet
			printf("ip check 0x%x\n", (iphead[10] << 8 | iphead[11]));
			printf("IP[%d.%d.%d.%d]", iphead[12], iphead[13], iphead[14], iphead[15]);
			printf("->[%d.%d.%d.%d] ", iphead[16], iphead[17], iphead[18], iphead[19]);
			printf("%d", iphead[9]);

			if(iphead[9] == IPPROTO_TCP)
				printf("[TCP]");
			else if(iphead[9] == IPPROTO_UDP)
			{
				printf(" [UDP] ");
				printf("udp len %d\n", (iphead[24] << 8 | iphead[25]));
				printf("udp check %d\n", (iphead[26] << 8 | iphead[27]));
			}		
			else if(iphead[9] == IPPROTO_ICMP)
				printf("[ICMP]");
			else if(iphead[9] == IPPROTO_IGMP)
				printf("[IGMP]");
			else if(iphead[9] == IPPROTO_IGMP)
				printf("[IGMP]");
			else
				printf("[OTHERS]");
		
			printf(" PORT[%d]->[%d]\n", (iphead[20] << 8 | iphead[21]), (iphead[22] << 8 | iphead[23]));
			printf("%d bytes recieved:packet: %ld\n", n, packets);
		}
		
	}
	
//release the resources
	close(sock);
	exit(0);
 
     return 0;
}


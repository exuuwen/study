/*af_packet filter: gcc -o packet_filter bpf_image.c packet_filter.c -lpcap*/
#include <pcap.h>
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
static char *filter_buf;
static struct bpf_program program;


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

	pcap_freecode(&program);
        free(filter_buf);
	close(sock);
	exit(0);
}

static unsigned long packets;

static void usage(void)
{
	fprintf(stderr, "Usage: packet_filter [-O]  <cmd>\n");
}


int main(int argc, char *argv[ ])
{
     int offset = 0;
        
     int  optimize = 1;//优化
     int  arg_begin = 1;
     int n;
     char buf[2048];
     unsigned char*ethhead;
     unsigned char*iphead;
     struct ifreq ethreq;
     struct sigaction sighandle;

 
     filter_buf = (char*)malloc(CMD_LEN);
     memset(filter_buf, 0x0, CMD_LEN);
 
     if(argc <= 1)
     {
         usage();
	 exit(1);
     }
     else
     {
         /*决定是否需要优化指令*/
         if( strcmp(argv[1],"-O")==0 )
         {
              optimize = 0;//不优化
              arg_begin = 2;
         }
         else
         {
              optimize = 1;//优化
              arg_begin = 1;
         }
    
 	int i;
         /*从命令行中获得过滤表达式*/
         for(i = arg_begin ;i < argc; i++)
         {   
              strcpy(filter_buf+offset, argv[i]);
              offset=offset+strlen(argv[i])+1;
              memset(filter_buf+offset-1, ' ', 1);                
         }
     }
 
     //打印过滤表达式
     printf("过滤表达式：%s\n", filter_buf);
 
	/*网络掩码，以太网IP协议*/
     bpf_u_int32 netmask = 0xffffff;
 
          
    
     int ret = pcap_compile_nopcap(96,
                            DLT_EN10MB,
                            &program,
                            filter_buf,
                            optimize,
                            netmask
                            ) ;
     if(ret < 0)
     {
         usage();
	 exit(1);
     }
     printf("program.bf_len=%d\n",program.bf_len);
     int i;	
	
     const struct bpf_insn *insn = program.bf_insns;
     
     for (i = 0; i < program.bf_len; ++insn, ++i) 
	puts(bpf_image(insn, i));
		
	printf("\n");
     
     for(i=0; i<program.bf_len; i++)
     {
         printf("{0x%x, %u, %u, 0x%.8x},\n",
	 program.bf_insns[i].code, program.bf_insns[i].jt,
	 program.bf_insns[i].jf, program.bf_insns[i].k);
     }



 
	struct sock_filter *bpf_codes = (struct sock_filter *)(program.bf_insns);
	

	struct sock_fprog filter;
	filter.len = program.bf_len;
	filter.filter = bpf_codes;

	sighandle.sa_flags = 0;
	sighandle.sa_handler = sig_handler;
	sigemptyset(&sighandle.sa_mask);

	sigaction(SIGTERM, &sighandle, NULL);
	sigaction(SIGINT, &sighandle, NULL);
	sigaction(SIGQUIT, &sighandle, NULL);

	//AF_PACKET allows application to read packet from and write packet to network device
	//SOCK_DGRAM the packet exclude ethernet header
	//SOCK_RAW raw data from the device including ethernet header
	//ETH_P_IP all IP packets
	if((sock=socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP)))==-1){
		perror("socket");
		exit(1);
	}

	//set NIC to promiscous mode,so we can recieve all packets of thenetwork
	strncpy(ethreq.ifr_name, "eth0", IFNAMSIZ);
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
	
	//attach the bpf filter
	if(setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter))==-1){
		perror("setsockopt");
		close(sock);
		exit(1);
	}
	
	

	while(1)
	{
		n = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
		if(n < (ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN)){
			printf("invalid packet\n");
			continue;
		}
		packets++;
		printf("%d bytes recieved:packet: %ld\n", n, packets);
		
		ethhead = buf;
		
		printf("Ethernet:MAC[%02X:%02X:%02X:%02X:%02X:%02X]", ethhead[6], ethhead[7], ethhead[8], ethhead[9], ethhead[10], ethhead[11]);
		printf("->[%02X:%02X:%02X:%02X:%02X:%02X] ", ethhead[0], ethhead[1], ethhead[2], ethhead[3], ethhead[4], ethhead[5]);
		printf("type[%04x]\n ",(ntohs(ethhead[12] | ethhead[13] << 8)));

		iphead = ethhead + ETH_HDR_LEN;
		//header length as 32-bit
		printf("IP:Version:%d HeaderLen:%d[%d] ", (*iphead>>4), (*iphead&0x0f), (*iphead&0x0f)*4);
		printf("TOS %d ", iphead[1]);
		printf("TotalLen %d ", (iphead[2] <<8 | iphead[3]));
		printf("id %d ", (iphead[4] <<8 | iphead[5]));
		printf("ttl %d ", iphead[8]);
		printf("frag:0x%x\n", (ethhead[20] << 8 | ethhead[21])); // frag&0x1fff is true just a fragemnt packet
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
		
		
	}
	
//release the resources
	pcap_freecode(&program);
	free(filter_buf);
	close(sock);
	exit(0);
 
     return 0;
}


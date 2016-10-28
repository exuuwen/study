#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <sys/mman.h>
#include <linux/if_ether.h>
#include <linux/sockios.h>
#include <linux/filter.h>
#include <stdlib.h>
#include <pcap.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
/*
cr7@ubuntu:~/test/modules/PF_RING$ gcc -o test_pfring test_pfring.c -lpcap
cr7@ubuntu:~/test/modules/PF_RING$ sudo ./test_pfring ip and udp dst port 2152
*/

#include "pfring.h"

#define CAPTURE_SIZE (29)

typedef struct
{
    unsigned char *buffer;
    unsigned char *slots;
    char *device_name;
    int  fd;
} pfring;

static pfring *ring = NULL;
static FlowSlotInfo* slotsInfo = NULL;

#define ETH_HDR_LEN 14
#define IP_HDR_LEN 20

static long packets = 0;

char* bpf_image(const struct bpf_insn *p, int n)
{
	int v;
	const char *fmt, *op;
	static char image[256];
	char operand[64];

	v = p->k;
	switch (p->code) {

	default:
		op = "unimp";
		fmt = "0x%x";
		v = p->code;
		break;

	case BPF_RET|BPF_K:
		op = "ret";
		fmt = "#%d";
		break;

	case BPF_RET|BPF_A:
		op = "ret";
		fmt = "";
		break;

	case BPF_LD|BPF_W|BPF_ABS:
		op = "ld";
		fmt = "[%d]";
		break;

	case BPF_LD|BPF_H|BPF_ABS:
		op = "ldh";
		fmt = "[%d]";
		break;

	case BPF_LD|BPF_B|BPF_ABS:
		op = "ldb";
		fmt = "[%d]";
		break;

	case BPF_LD|BPF_W|BPF_LEN:
		op = "ld";
		fmt = "#pktlen";
		break;

	case BPF_LD|BPF_W|BPF_IND:
		op = "ld";
		fmt = "[x + %d]";
		break;

	case BPF_LD|BPF_H|BPF_IND:
		op = "ldh";
		fmt = "[x + %d]";
		break;

	case BPF_LD|BPF_B|BPF_IND:
		op = "ldb";
		fmt = "[x + %d]";
		break;

	case BPF_LD|BPF_IMM:
		op = "ld";
		fmt = "#0x%x";
		break;

	case BPF_LDX|BPF_IMM:
		op = "ldx";
		fmt = "#0x%x";
		break;

	case BPF_LDX|BPF_MSH|BPF_B:
		op = "ldxb";
		fmt = "4*([%d]&0xf)";
		break;

	case BPF_LD|BPF_MEM:
		op = "ld";
		fmt = "M[%d]";
		break;

	case BPF_LDX|BPF_MEM:
		op = "ldx";
		fmt = "M[%d]";
		break;

	case BPF_ST:
		op = "st";
		fmt = "M[%d]";
		break;

	case BPF_STX:
		op = "stx";
		fmt = "M[%d]";
		break;

	case BPF_JMP|BPF_JA:
		op = "ja";
		fmt = "%d";
		v = n + 1 + p->k;
		break;

	case BPF_JMP|BPF_JGT|BPF_K:
		op = "jgt";
		fmt = "#0x%x";
		break;

	case BPF_JMP|BPF_JGE|BPF_K:
		op = "jge";
		fmt = "#0x%x";
		break;

	case BPF_JMP|BPF_JEQ|BPF_K:
		op = "jeq";
		fmt = "#0x%x";
		break;

	case BPF_JMP|BPF_JSET|BPF_K:
		op = "jset";
		fmt = "#0x%x";
		break;

	case BPF_JMP|BPF_JGT|BPF_X:
		op = "jgt";
		fmt = "x";
		break;

	case BPF_JMP|BPF_JGE|BPF_X:
		op = "jge";
		fmt = "x";
		break;

	case BPF_JMP|BPF_JEQ|BPF_X:
		op = "jeq";
		fmt = "x";
		break;

	case BPF_JMP|BPF_JSET|BPF_X:
		op = "jset";
		fmt = "x";
		break;

	case BPF_ALU|BPF_ADD|BPF_X:
		op = "add";
		fmt = "x";
		break;

	case BPF_ALU|BPF_SUB|BPF_X:
		op = "sub";
		fmt = "x";
		break;

	case BPF_ALU|BPF_MUL|BPF_X:
		op = "mul";
		fmt = "x";
		break;

	case BPF_ALU|BPF_DIV|BPF_X:
		op = "div";
		fmt = "x";
		break;

	case BPF_ALU|BPF_AND|BPF_X:
		op = "and";
		fmt = "x";
		break;

	case BPF_ALU|BPF_OR|BPF_X:
		op = "or";
		fmt = "x";
		break;

	case BPF_ALU|BPF_LSH|BPF_X:
		op = "lsh";
		fmt = "x";
		break;

	case BPF_ALU|BPF_RSH|BPF_X:
		op = "rsh";
		fmt = "x";
		break;

	case BPF_ALU|BPF_ADD|BPF_K:
		op = "add";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_SUB|BPF_K:
		op = "sub";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_MUL|BPF_K:
		op = "mul";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_DIV|BPF_K:
		op = "div";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_AND|BPF_K:
		op = "and";
		fmt = "#0x%x";
		break;

	case BPF_ALU|BPF_OR|BPF_K:
		op = "or";
		fmt = "#0x%x";
		break;

	case BPF_ALU|BPF_LSH|BPF_K:
		op = "lsh";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_RSH|BPF_K:
		op = "rsh";
		fmt = "#%d";
		break;

	case BPF_ALU|BPF_NEG:
		op = "neg";
		fmt = "";
		break;

	case BPF_MISC|BPF_TAX:
		op = "tax";
		fmt = "";
		break;

	case BPF_MISC|BPF_TXA:
		op = "txa";
		fmt = "";
		break;
	}
	(void)snprintf(operand, sizeof operand, fmt, v);
	(void)snprintf(image, sizeof image,
		      (BPF_CLASS(p->code) == BPF_JMP &&
		       BPF_OP(p->code) != BPF_JA) ?
		      "(%03d) %-8s %-16s jt %d\tjf %d"
		      : "(%03d) %-8s %s",
		      n, op, operand, n + 1 + p->jt, n + 1 + p->jf);
	return image;
}

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
    if(ioctl(ring->fd, SIOCGIFFLAGS, &ethreq) != -1)
    {
        ethreq.ifr_flags &= ~IFF_PROMISC;
        ioctl(ring->fd, SIOCSIFFLAGS,&ethreq);
    }

    close(ring->fd);
    exit(0);
}

static int get_message(unsigned char* dest_p, int offset, int len, int flags)
{
	struct msghdr msg = {};
	struct iovec msgIovec[2];
	
	msg.msg_iov = msgIovec;
	msg.msg_iovlen = 2;

	msgIovec[0].iov_base = NULL;
	msgIovec[0].iov_len = offset; // pass the offset to kernel
 
 	msgIovec[1].iov_base = dest_p;
 	msgIovec[1].iov_len = len;
 
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
     
	int ret = recvmsg(ring->fd, &msg, flags);
	if (ret != len)
	{
		printf("CPfRingSocket::peekMessage: return length %d is not equal to %d, offset:%d, errno:%d\n", ret, len, offset, errno);
	}
   
	return ret;
}	 

int handlePackets(FlowSlot *slot)
{
    unsigned char*ethhead;
    unsigned char*iphead;
	int ret;

    unsigned short len = slot->pkt_len;
    printf("packet len:%u, snap len:%u\n", len, slot->snap_len);

    unsigned char ip_tmp[4];

    if(len < CAPTURE_SIZE)
    {
        printf("not a ip packet\n");
        return;
    }	
    
    iphead = &slot->bucket[0];

    packets++;
   
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
        printf("udp check 0x%x\n", (iphead[26] << 8 | iphead[27]));
    }		
    else if(iphead[9] == IPPROTO_ICMP)
        printf("[ICMP]");
    else if(iphead[9] == IPPROTO_IGMP)
        printf("[IGMP]");
    else if(iphead[9] == IPPROTO_IGMP)
        printf("[IGMP]");
    else
        printf("[OTHERS]");

    printf("PORT[%d]->[%d]\n", (iphead[20] << 8 | iphead[21]), (iphead[22] << 8 | iphead[23]));
    
   

	memcpy(ip_tmp, iphead + 12, 4);
    memcpy(iphead + 12, iphead + 16, 4);
    memcpy(iphead + 16, ip_tmp, 4);

	unsigned char buf[10];

	if (iphead[28])
		ret = get_message(buf, CAPTURE_SIZE, sizeof(buf), MSG_TRUNC);
	else
		ret = get_message(buf, CAPTURE_SIZE, sizeof(buf), MSG_PEEK);

	printf("iphead[28]: %d, get_message ret:%d\n", iphead[28], ret);

	printf("packet: %ld-----------------------------\n", packets);
	return iphead[28];
}

int _recv()
{
    FlowSlot *slot;
    unsigned int pf_ring_read = 0;
	int ret;

    if ((ring == NULL) || (ring->buffer == NULL))
    {
        return -1;
    }
    while (slotsInfo->tot_insert > slotsInfo->tot_read)
    {

        if (slotsInfo->read_off == slotsInfo->end_off)
        {
            slotsInfo->read_off = 0;
        }

        slot = (FlowSlot*)(ring->slots + slotsInfo->read_off);
        // Make sure the slot state are in right state
        if (slot->slot_state == FULL_RX)
        {
            // Call the call back function to handle the packet in order
            // read_off update must be always later than
            // 1, packet handling
            // 2, slot state update
            // 3, tot_read++
            printf("in app 0x%x slots is rx\n", slotsInfo->read_off); 
            ret = handlePackets(slot);
        
            slotsInfo->tot_read++;
			if (!ret)
            	slot->slot_state = FULL_TX;
            __sync_synchronize();
            slotsInfo->read_off += slot->slot_len;
            
        }
        else
        {
			/* maybe get the slot but do not copy the data to it*/
	    	printf("%d slot->slot_state != FULL_RX: %d\n", slotsInfo->read_off, slot->slot_state);
			assert(slot->slot_state == EMPTY);
            break;
        }
    }

    return 0 ;
}

static void usage(void)
{
	fprintf(stderr, "Usage: packet_filter [-O]  <cmd>\n");
}

#define CMD_LEN 1024
static char *filter_buf;
static struct bpf_program program;

int main(int argc, char *argv[])
{
    struct sigaction sighandle;
    sighandle.sa_flags = 0;
    sighandle.sa_handler = sig_handler;
    sigemptyset(&sighandle.sa_mask);

    sigaction(SIGTERM, &sighandle, NULL);
    sigaction(SIGINT, &sighandle, NULL);
    sigaction(SIGQUIT, &sighandle, NULL);

    ring = (pfring *)malloc(sizeof(pfring));
    assert(ring);
    memset(ring, 0, sizeof(pfring));
    
    ring->fd = socket(PF_RING, SOCK_RAW, htons(ETH_P_ALL));
    if(ring->fd < 0)
    {
        perror("fail fd");
    }
    assert(ring->fd > 0);
    
    struct sockaddr sa;
    int rc;
    sa.sa_family = PF_RING;
    snprintf(sa.sa_data, sizeof(sa.sa_data), "%s", "eth0");

    rc = bind(ring->fd, (struct sockaddr *)&sa, sizeof(sa));
    assert(rc == 0);

    ring->buffer = (unsigned char *)mmap(NULL, 2 * PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, ring->fd, 0);
    assert(ring->buffer != MAP_FAILED);

    slotsInfo = (FlowSlotInfo *)ring->buffer;
    assert(slotsInfo->version == RING_VERSION);

    unsigned int totalMemorySize = slotsInfo->tot_mem;
    printf("totol mem:%u\n", totalMemorySize);
    
    munmap(ring->buffer, 2 * PAGE_SIZE);

    ring->buffer = (unsigned char *)mmap(NULL, totalMemorySize, PROT_READ|PROT_WRITE, MAP_SHARED, ring->fd, 0);
    assert(ring->buffer != MAP_FAILED);
    

    slotsInfo = (FlowSlotInfo *)ring->buffer;
    ring->slots = (unsigned char *)(ring->buffer + sizeof(FlowSlotInfo));

    assert(slotsInfo->read_off == 0);
    assert(slotsInfo->insert_off == 0);
    assert(slotsInfo->forward_off == 0);

    ring->device_name = strdup("eth0");

    filter_buf = (char*)malloc(CMD_LEN);
    memset(filter_buf, 0x0, CMD_LEN);

    int i;
    int offset = 0;
    
    /*struct ifreq ethreq;
    strncpy(ethreq.ifr_name, "eth0", IFNAMSIZ);
    if(ioctl(ring->fd, SIOCGIFFLAGS, &ethreq) == -1)
    {
        perror("ioctl");
        close(ring->fd);
        exit(1);
    }

    ethreq.ifr_flags|= IFF_PROMISC;
    if(ioctl(ring->fd, SIOCSIFFLAGS, &ethreq) == -1)
    {
        perror("ioctl");
        close(ring->fd);
        exit(1);
    }
    */
    if(argc != 1)
    {
		for(i=1 ;i < argc; i++)
		{   
			strcpy(filter_buf + offset, argv[i]);
			offset = offset + strlen(argv[i]) + 1;
			memset(filter_buf + offset - 1, ' ', 1);                
		}


		printf("过滤表达式：%s\n", filter_buf);

		bpf_u_int32 netmask = 0xffffff;

		int ret = pcap_compile_nopcap(96, DLT_EN10MB, &program, filter_buf, 1,  netmask) ;
		if(ret < 0)
		{
		 usage();
		 exit(1);
		}
		printf("program.bf_len=%d\n", program.bf_len);	

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

		if(setsockopt(ring->fd, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter)) == -1)
		{
			perror("setsockopt");
			close(ring->fd);
			exit(1);
		}
    }

    unsigned int capture_size = CAPTURE_SIZE;
    rc = setsockopt(ring->fd, 0, SO_SET_PFRING_SNAP_LENGTH, &capture_size, sizeof(capture_size));

    char dummy;
    rc = setsockopt(ring->fd, 0, SO_SET_PFRING_ACTI_SOCK, &dummy, sizeof(dummy));
    assert(rc == 0);  

    while(1)
    {
        int ret = _recv();
        //printf("recv %d\n", ret);
    }

    printf("It's ok all\n");
    sleep(5);
    close(ring->fd);
}


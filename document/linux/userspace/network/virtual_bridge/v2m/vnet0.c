#include <assert.h>
#include <unistd.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <signal.h>  
#include <unistd.h>  
#include <linux/if_tun.h>  
#include <netinet/in.h>  
#include <sys/ioctl.h>  
#include <sys/time.h>  
#include <linux/if.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <errno.h>  
#include <fcntl.h> 
#include <linux/sockios.h>
#include <linux/if_bridge.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
/*
#sudo brctl addbr br0
#sudo ifconfig br0 192.168.60.1 up
#sudo brctl addif br0 eth0
./vnet0 vnet0 1 192.168.60.7 
Set 'vnet0' persistent:1
set vnet0 ip addr:192.168.60.7
set vnet0 interface up
add vnet0 to bridge br0
create buf ret = 62
0x0, 0xc, 0x29, 0xe, 0x4f, 0x21, 0x0, 0xa, 0x19, 0x3e, 0x5f, 0x31, 0x8, 0x0, 0x45, 0x0, 0x0, 0x30, 0xa, 0x0, 0x40, 0x0, 0x40, 0x11, 0xad, 0x5d, 0xc0, 0xa8, 0x1, 0x7, 0xc0, 0xa8, 0x1, 0x8, 0x22, 0x11, 0x11, 0x22, 0x0, 0x1c, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
write 62 to peer
*/

/*
disable and delete vnet0
sudo ./vnet0 vnet0 0
*/
int tun_alloc(char *dev, int flags, int persist) 
{
	struct ifreq ifr;
	int fd, err;
	char *clonedev = "/dev/net/tun";

	/* Arguments taken by the function:
	*
	* char *dev: the name of an interface (or '\0'). MUST have enough
	*   space to hold the interface name if '\0' is passed
	* int flags: interface flags (eg, IFF_TUN etc.)
	*/

	/* open the clone device */
	if( (fd = open(clonedev, O_RDWR)) < 0 ) 
	{
		perror("open");
		return fd;
	}

	/* preparation of the struct ifr, of type "struct ifreq" */
	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = flags | IFF_NO_PI;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

	if (*dev) 
	{
		/* if a device name was specified, put it in the structure; otherwise,
		* the kernel will try to allocate the "next" device of the
		* specified type */
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}

	/* try to create the device */
	if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) 
	{
		close(fd);
		perror("TUNSETIFF");
		return err;
	}

	strcpy(dev, ifr.ifr_name);

	/*set dev to persist*/
	if(ioctl(fd, TUNSETPERSIST, persist) < 0)
	{
		perror("TUNSETPERSIST");
       	return -1;
   	}

	return fd;
}

/** 
 *  ifconfig vnet0 up 
 */  
int interface_up(char *interface_name)  
{  
    int s;  
  
    if((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
    {  
        printf("Error create socket :%m\n", errno);  
        return -1;  
    }  
  
    struct ifreq ifr;  
    strcpy(ifr.ifr_name, interface_name);  
  
    short flag;  
    flag = IFF_UP;  
    if(ioctl(s, SIOCGIFFLAGS, &ifr) < 0)  
    {  
        printf("Error up %s :%m\n", interface_name, errno);  
        return -1;  
    }  
  
    ifr.ifr_ifru.ifru_flags |= flag;  
  
    if(ioctl(s, SIOCSIFFLAGS, &ifr) < 0)  
    {  
        printf("Error up %s :%m\n", interface_name, errno);  
        return -1;  
    }  

	close(s);
  
    return 0;  
}  
/* ifconfig vnet0 up*/
int interface_down(char *interface_name)  
{  
    int s;  
  
    if((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
    {  
        printf("Error create socket :%m\n", errno);  
        return -1;  
    }  
  
    struct ifreq ifr;  
    strcpy(ifr.ifr_name, interface_name);  
  
    short flag;  
    flag = IFF_UP;  
    if(ioctl(s, SIOCGIFFLAGS, &ifr) < 0)  
    {  
        printf("Error up %s :%m\n", interface_name, errno);  
        return -1;  
    }  
  
    ifr.ifr_ifru.ifru_flags &= ~flag;  
  
    if(ioctl(s, SIOCSIFFLAGS, &ifr) < 0)  
    {  
        printf("Error down %s :%m\n", interface_name, errno);  
        return -1;  
    }  

	close(s);
  
    return 0;  
}  
  
/** 
 *  ifconfig vnet0 ip_addr
 */  
int set_ipaddr(char *interface_name, char *ip)  
{  
    int s;  
  
    if((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
    {  
        printf("Error up %s :%m\n", interface_name, errno);  
        return -1;  
    }  
  
    struct ifreq ifr;  
    strcpy(ifr.ifr_name, interface_name);  
  
    struct sockaddr_in addr;  
    bzero(&addr, sizeof(struct sockaddr_in));  
    addr.sin_family = PF_INET;  
    inet_aton(ip, &addr.sin_addr);  
  
    memcpy(&ifr.ifr_ifru.ifru_addr, &addr, sizeof(struct sockaddr_in));  
  
    if(ioctl(s, SIOCSIFADDR, &ifr) < 0)  
    {  
        printf("Error set %s ip :%m\n", interface_name, errno);  
        return -1;  
    }  

	close(s);
  
    return 0;  
}  

int add_to_bridge(const char *ifname, const char *brname)
{
    int s;  
  
    if((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
    {  
        printf("Error up %s :%m\n", ifname, errno);  
        return -1;  
    }  
  
    struct ifreq ifr;  
    strcpy(ifr.ifr_name, brname);  
    
	if (!(ifr.ifr_ifindex = if_nametoindex(ifname))) 
	{
        printf("Error get %s ifindex :%m\n", ifname, errno);  
		close(s);
		return -1;
	}

	if(ioctl(s, SIOCBRADDIF, &ifr) < 0)  
    {  
        printf("Error add %s to br0 :%m\n", ifname, errno);  
		close(s);
        return -1;  
    }  

	close(s);
  
    return 0;  
}  


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

unsigned char packet_dst[ETH_ALEN] = {0x00, 0x0c, 0x29, 0x0e, 0x4f, 0x21};
unsigned char packet_src[ETH_ALEN] = {0x00, 0x0a, 0x19, 0x3e, 0x5f, 0x31};

int create_packet(unsigned char *buf, int len)
{
	unsigned int total_len = 0;
	struct iphdr * ip_header;
	struct udphdr * udp_header;
	struct ethhdr * mac_header;
	int ret;
	
	total_len = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + len;	

	mac_header = (struct ethhdr *) buf;
	
	memcpy(mac_header->h_source, packet_src, ETH_ALEN);
	
	struct ifreq buf1;
	int s;
	s = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&buf1, sizeof(buf1));
	strcpy(buf1.ifr_name, "eth0"); //eth0 should join in the br0. vnet0 or br0 is the same situation, the dest mac of the interface is on the bridge, so deliver it to netif_recieve_sk, and skb->dev = br->dev
	if (ioctl(s, SIOCGIFHWADDR, &buf1)) 
	{
		perror("SIOCGIFHWADDR");
		close(s);
		return -1;
	}	
	close(s);
	memcpy(mac_header->h_dest, buf1.ifr_hwaddr.sa_data, ETH_ALEN);

	mac_header->h_proto = htons(ETH_P_IP);

	ip_header = (struct iphdr *)(buf + sizeof(struct ethhdr));
	ip_header->ihl = 5;
	ip_header->version = 4;
	ip_header->tos = 0;

	// ip length contains the total length of ip header, tcp/udp header and payload in bytes.
	ip_header->tot_len = htons(total_len - sizeof(struct ethhdr));
	ip_header->id = 10; // the value doesnt matter here
	ip_header->ttl = 64;
	ip_header->frag_off = 0x40;
	ip_header->protocol = IPPROTO_UDP;
	
	ip_header->daddr = inet_addr("192.168.60.198"); //eth0, can be any one on the host include bridge ip, so the udp socket can receive it.

	ip_header->saddr = inet_addr("192.168.60.10"); // can not be the local ip address,and must be on the same subnetwork of the bridge(192.168.60.x) dropped by ip_route_input_slow
	ip_header->check = 0;
	ip_header->check = (ip_sum_calc(ip_header->ihl*4, (unsigned short*)ip_header));
	//
	// udp header filling
	//
	udp_header = (struct udphdr *) (buf + sizeof(struct iphdr) + sizeof(struct ethhdr));	

	// fake port, may different with ServerPort.
	udp_header->source = htons(0x2211);
	udp_header->dest = htons(2152);

	// the length of udp header and payload data in bytes.
	udp_header->len = htons(sizeof(struct udphdr) + len);  
	udp_header->check = 0;

	return total_len;
}

 
	
int main(int argc, char *argv[])
{
	char tap_name[IFNAMSIZ];
	int tapfd, ret = 0;

	if(argc <= 2)
	{
		printf("%s devname persist [ipaddr]\n", argv[0]);
		return -1;
	}

    strcpy(tap_name, argv[1]);
 	
	tapfd = tun_alloc(tap_name, IFF_TAP, atoi(argv[2]));  /* tup interface: L3 device */
					       								  /* IFF_TAP: tap interface: L2 device */
	assert(tapfd > 0);		
	printf("Set '%s' persistent:%d\n", tap_name, atoi(argv[2]));

	if (!atoi(argv[2]))
	{
		ret = interface_down(tap_name);
		assert(ret == 0);

		printf("set %s interface down\n", tap_name);
		return 0;
	}

	assert(argc == 4);

	ret = set_ipaddr(tap_name, argv[3]);
	assert(ret == 0);

	printf("set %s ip addr:%s\n", tap_name, argv[3]);

    ret = interface_up(tap_name);
	assert(ret == 0);

	printf("set %s interface up\n", tap_name);

	char *br_name = "br0";
	ret = add_to_bridge(tap_name, br_name);
	assert(ret == 0);

	printf("add %s to bridge %s\n", tap_name, br_name);
	
	int nreadm, i;
	unsigned char buf[1514];

	memset(buf, 0, sizeof(buf));
	ret = create_packet(buf, 20);

	printf("create buf ret = %d\n", ret);
	for(i=0; i<ret; i++)
		printf("0x%x, ", buf[i]);
	printf("\n");
    	
	while(1) 
	{
		ret = write(tapfd, buf, ret);
		if (ret < 0)
		    break;
		printf("write %d to peer\n", ret);
		sleep(2);
	}
    
	close(tapfd);

	return 0;
}


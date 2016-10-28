#include <stdio.h>
#include <sys/prctl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sched.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/ioctl.h>  
#include <linux/if.h>  
#include <linux/if_tun.h>  
#include <sys/types.h>  
#include <errno.h>  
#include <net/route.h>  
/*
cr7@ubuntu:~/test/kvm/tap$ sudo ./msg_source 192.168.0.1 192.168.0.2 vnet0
Recv  message from server:TAP/TUN TEST DATA
Recv  message from server:TAP/TUN TEST DATA
*/

#define TEST_PORT 8888   
#define TEST_DATA "TAP/TUN TEST DATA"                                
#define BUFF_SIZE 1200

/*route add ip_addr dev vnet0*/
int add_route(char *interface_name, char *ip_addr)  
{  
    int skfd;  
    struct rtentry rt;  
  
    struct sockaddr_in dst;  
    struct sockaddr_in gw;  
    struct sockaddr_in genmask;  
  
    memset(&rt, 0, sizeof(rt));  
  
    genmask.sin_addr.s_addr = inet_addr("255.255.255.255");  
  
    bzero(&dst,sizeof(struct sockaddr_in));  
    dst.sin_family = PF_INET;  
    dst.sin_addr.s_addr = inet_addr(ip_addr);  
  
    rt.rt_metric = 0;  
    rt.rt_dst = *(struct sockaddr*) &dst;  
    rt.rt_genmask = *(struct sockaddr*) &genmask;  
  
    rt.rt_dev = interface_name;  
    rt.rt_flags = RTF_UP | RTF_HOST ;  
  
    skfd = socket(AF_INET, SOCK_DGRAM, 0);  
    if(ioctl(skfd, SIOCADDRT, &rt) < 0)   
    {  
        printf("Error route add :%m\n", errno);  
        return -1;  
    }  

	close(skfd);
}  

int main(int argc, char *argv[])
{
	int s, n;
	struct sockaddr_in dst_addr, src_addr;

	if(argc != 4)
	{
		printf("%s src_ip dst_ip tap_name\n", argv[0]);
		return -1;
	}

	s = add_route(argv[3], argv[2]);
	if (s == -1)
	{
		perror("add_route");
		return -1;
	}
     
	s = socket(AF_INET, SOCK_DGRAM, 0);         
	if (s == -1)
	{
		perror("socket()");
		return -1;
	}
 
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.sin_family = AF_INET;                
	src_addr.sin_addr.s_addr = inet_addr(argv[1]);
	src_addr.sin_port = htons(TEST_PORT);

	n = bind(s, (struct sockaddr*)&src_addr, sizeof(src_addr));
	if (n == -1)
	{
		perror("bind:");
		return -1;
	}

	memset(&dst_addr, 0, sizeof(dst_addr));
	dst_addr.sin_family = AF_INET;                
	dst_addr.sin_addr.s_addr = inet_addr(argv[2]);
	dst_addr.sin_port = htons(TEST_PORT);
	
	
	char buf[BUFF_SIZE];
	memset(buf, 0, sizeof(buf));
	while(1) 
	{
		n = sendto(s, TEST_DATA, sizeof(TEST_DATA), 0, (struct sockaddr*)&dst_addr, sizeof(dst_addr)) ;
		if( n < 0)
		{
		    perror("sendto()");
		    return -2;
		}      

		memset(buf, 0, BUFF_SIZE);  
		n = recvfrom(s, buf, sizeof(buf), 0, NULL, NULL);
		if( n== -1)
		{
	   		perror("recvfrom()");
		}
		                                    
		printf("Recv  message from server:%s\n", buf);
		sleep(2);                
	}

	return 0;
}

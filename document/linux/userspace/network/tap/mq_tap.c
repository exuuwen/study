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
#define IFF_MULTI_QUEUE 0x0100
/*
cr7@ubuntu:~/test/kvm/tap$ sudo ./tap vnet0 1 192.168.0.1
Set 'vnet0' persistent:1
set vnet0 ip addr:192.168.0.1
set vnet0 interface up
read 46 bytes
45, 0, 0, 2e, 0, 0, 40, 0, 40, 11, b9, 6b, c0, a8, 0, 1, c0, a8, 0, 2, 22, b8, 22, b8, 0, 1a, a7, dd, 54, 41, 50, 2f, 54, 55, 4e, 20, 54, 45, 53, 54, 20, 44, 41, 54, 41, 0, 
read 46 bytes
45, 0, 0, 2e, 0, 0, 40, 0, 40, 11, b9, 6b, c0, a8, 0, 1, c0, a8, 0, 2, 22, b8, 22, b8, 0, 1a, a7, dd, 54, 41, 50, 2f, 54, 55, 4e, 20, 54, 45, 53, 54, 20, 44, 41, 54, 41, 0, 
*/

/*
disable and delete vnet0
sudo ./tap vnet0 0
*/

int tun_alloc_mq(const char *dev, int flags, int queues, int *fds, int persist) 
{ 
	struct ifreq ifr;
    int fd, err, i;

    if (!dev) 
	    return -1;
    memset(&ifr, 0, sizeof(ifr));
    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
    *        IFF_TAP   - TAP device
    *        IFF_NO_PI - Do not provide packet information
    *        IFF_MULTI_QUEUE - Create a queue of multiqueue device
    */ 
    ifr.ifr_flags =  flags | IFF_NO_PI | IFF_MULTI_QUEUE;
    strcpy(ifr.ifr_name, dev);

    for (i = 0; i < queues; i++) 
	{ 
	    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) 
		    goto err; 
	    err = ioctl(fd, TUNSETIFF, (void *)&ifr);
       	if (err) 
	    {	
		    perror("setIff"); 
		    close(fd);
       		goto err; 
	    }
       	fds[i] = fd; 
    }

	/*set dev to persist*/
	if(ioctl(fd, TUNSETPERSIST, persist) < 0)
    {
       	perror("TUNSETPERSIST");
       	close(fd);
    	goto err;
     }
      
	return 0; 

err: 
	for (--i; i >= 0; i--) 
		close(fds[i]); 
	return err; 

}

/** 
 *  ifconfig vnet0 up 
 */  
int interface_up(char *interface_name)  
{  
    int s;  
  
    if((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
    {  
        printf("Error create socket :%d\n", errno);  
        return -1;  
    }  
  
    struct ifreq ifr;  
    strcpy(ifr.ifr_name, interface_name);  
  
    short flag;  
    flag = IFF_UP;  
    if(ioctl(s, SIOCGIFFLAGS, &ifr) < 0)  
    {  
        printf("Error up %s :%d\n", interface_name, errno);  
        return -1;  
    }  
  
    ifr.ifr_ifru.ifru_flags |= flag;  
  
    if(ioctl(s, SIOCSIFFLAGS, &ifr) < 0)  
    {  
        printf("Error up %s :%d\n", interface_name, errno);  
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
        printf("Error create socket :%d\n", errno);  
        return -1;  
    }  
  
    struct ifreq ifr;  
    strcpy(ifr.ifr_name, interface_name);  
  
    short flag;  
    flag = IFF_UP;  
    if(ioctl(s, SIOCGIFFLAGS, &ifr) < 0)  
    {  
        printf("Error up %s :%d\n", interface_name, errno);  
        return -1;  
    }  
  
    ifr.ifr_ifru.ifru_flags &= ~flag;  
  
    if(ioctl(s, SIOCSIFFLAGS, &ifr) < 0)  
    {  
        printf("Error down %s :%d\n", interface_name, errno);  
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
        printf("Error up %s :%d\n", interface_name, errno);  
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
        printf("Error set %s ip :%d\n", interface_name, errno);  
        return -1;  
    }  

	close(s);
  
    return 0;  
}  
  

int main(int argc, char *argv[])
{
	char tap_name[IFNAMSIZ];
	int tapfd[6], ret = 0;

	if(argc <= 2)
	{
		printf("%s devname persist [ipaddr]\n", argv[0]);
		return -1;
	}

    memset(tap_name, 0, IFNAMSIZ);
    strcpy(tap_name, argv[1]);
 	
	
    if (atoi(argv[2]))
	{
		ret = tun_alloc_mq(tap_name, IFF_TAP, 6, tapfd, 1); 
        /* tup interface: L3 device, IFF_TAP: tap interface: L2 device */
		assert(ret == 0);		
	}
	else
	{
		ret = tun_alloc_mq(tap_name, IFF_TAP, 2, tapfd, 0 );
		assert(ret == 0);
		ret = interface_down(tap_name);
		assert(ret == 0);

		printf("set %s interface down\n", tap_name);
		return 0;
	}

    //while(1);

	printf("Set '%s' persistent:%d\n", tap_name, atoi(argv[2]));
	assert(argc == 4);

	ret = set_ipaddr(tap_name, argv[3]);
	assert(ret == 0);

	printf("set %s ip addr:%s\n", tap_name, argv[3]);

    ret = interface_up(tap_name);
	assert(ret == 0);

	printf("set %s interface up\n", tap_name);

	int nreadm, i;
	unsigned char buf[1514];
    
	
	while(1) 
	{
		unsigned char ip[4];  
  
       	ret = read(tapfd[0], buf, sizeof(buf));  
       	if (ret < 0)  
       		break;  
		printf("read %d bytes\n", ret); 
		for(i=0; i<ret; i++)
		{
			printf("%x, ", buf[i]);
		}

		memcpy(ip, &buf[12], 4);
		memcpy(&buf[12], &buf[16], 4);
		memcpy(&buf[16], ip, 4);

		ret = write(tapfd[0], buf, ret);
		if (ret < 0)
		    break;
		printf("\n");
	}
    
	sleep(3);
	close(tapfd[0]);
	close(tapfd[1]);

	return 0;
}


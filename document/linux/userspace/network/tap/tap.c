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
 	
	tapfd = tun_alloc(tap_name, IFF_TUN, atoi(argv[2]));  /* tup interface: L3 device */
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

	int nreadm, i;
	unsigned char buf[1514];
    
	
	while(1) 
	{
		unsigned char ip[4];  
  
        ret = read(tapfd, buf, sizeof(buf));  
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

		ret = write(tapfd, buf, ret);
		if (ret < 0)
		    break;
		printf("\n");
	}
    
	sleep(3);
	close(tapfd);

	return 0;
}


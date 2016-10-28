#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "if_ops.h"

int if_up(const char *ifname)  
{  
    int s; 
    int ret = 0; 
  
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
    {  
        perror("create socket:");  
        return -1;  
    }  
  
    struct ifreq ifr;  
    strcpy(ifr.ifr_name, ifname);  
  
    short flag;  
    flag = IFF_UP;  
    if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0)  
    {  
        perror("ioctl SIOCGIFFLAGS:");  
        ret = -1;  
        goto done;
    }  
  
    ifr.ifr_ifru.ifru_flags |= flag;  
  
    if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0)  
    {  
       perror("link up:");  
        ret = -1;  
        goto done; 
    }  

done:
    close(s);
  
    return ret;  
}  


int if_down(const char *ifname)  
{  
    int s;  
  
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
    {  
        printf("Error create socket :%m\n", errno);  
        return -1;  
    }  
  
    struct ifreq ifr;  
    strcpy(ifr.ifr_name, ifname);  
  
    short flag;  
    flag = IFF_UP;  
    if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0)  
    {  
        printf("Error Down %s :%m\n", ifname, errno);  
        return -1;  
    }  
  
    ifr.ifr_ifru.ifru_flags &= ~flag;  
  
    if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0)  
    {  
        printf("Error down %s :%m\n", ifname, errno);  
        return -1;  
    }  

    close(s);
  
    return 0;  
} 

int if_index(const char *ifname)
{
    int s;

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        printf("Error if index %s :%m\n", ifname, errno);
        return -1;
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);

    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) 
    {
	printf("ioctl failure (SIOCGIFINDEX), errno = %d\n", errno);
	close(s);
        return -1;
    }

    close(s);
    return ifr.ifr_ifindex;
}


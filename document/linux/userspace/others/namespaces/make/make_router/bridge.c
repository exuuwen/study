#include <stdio.h>   
#include <string.h>  
#include <assert.h>   
#include <errno.h> 

#include <sys/socket.h>      
#include <sys/ioctl.h>  
#include <linux/if.h>  
#include <linux/sockios.h>

#include "bridge.h"
#include "if_ops.h"


#ifdef __GNUC__
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif

int bridge_add_port(const char *ifname, const char *brname)
{
    int s;  

    if (unlikely(ifname == NULL) || unlikely(brname == NULL))
        return -1;
  
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
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

    if (ioctl(s, SIOCBRADDIF, &ifr) < 0)  
    {  
        printf("Error add %s to br0 :%m\n", ifname, errno);  
        close(s);
        return -1;  
    }  

    close(s);
  
    return 0;  
}  

int bridge_del_port(const char *ifname, const char *brname)
{
    int s;  

    if (unlikely(ifname == NULL) || unlikely(brname == NULL))
        return -1;
  
    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
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

    if (ioctl(s, SIOCBRDELIF, &ifr) < 0)  
    {  
        printf("Error add %s to br0 :%m\n", ifname, errno);  
        close(s);
        return -1;  
    }  

    close(s);
  
    return 0;  
}  


int add_bridge(const char *brname)
{
    int fd;  
	
   if (unlikely(brname == NULL))
        return -1;
  
    if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
    {  
        printf("Error socket :%m\n", errno);  
        return -1;  
    }

    if (ioctl(fd, SIOCBRADDBR, brname) < 0)  
    {  
        printf("Error add bridge %s :%m\n", brname, errno);  
        close(fd);
        return -1;  
    }  

    close(fd);
  
    return 0;  
}

int del_bridge(const char *brname)
{
    int fd;  

    if (unlikely(brname == NULL))
        return -1;

    if (if_down(brname) < 0)
        return -1;
  
    if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
    {  
        printf("Error socket :%m\n", errno);  
        return -1;  
    } 

    if (ioctl(fd, SIOCBRDELBR, brname) < 0)  
    {  
        printf("Error del bridge %s :%m\n", brname, errno);  
        close(fd);
        return -1;  
    }  

    close(fd);
  
    return 0;  
}  



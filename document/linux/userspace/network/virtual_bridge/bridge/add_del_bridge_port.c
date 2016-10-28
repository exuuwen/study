#include <assert.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <sys/types.h>  
#include <sys/socket.h>      
#include <sys/ioctl.h>  
#include <linux/if.h>  
#include <errno.h>  
#include <linux/sockios.h>
#include <linux/if_bridge.h>

#ifdef __GNUC__
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif

int bridgeAddPort(const char *ifname, const char *brname)
{
    int s;  

	if (unlikely(ifname == NULL) || unlikely(brname == NULL))
		return -1;
  
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

int bridgeDelPort(const char *ifname, const char *brname)
{
    int s;  

	if (unlikely(ifname == NULL) || unlikely(brname == NULL))
		return -1;
  
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

	if(ioctl(s, SIOCBRDELIF, &ifr) < 0)  
    {  
        printf("Error add %s to br0 :%m\n", ifname, errno);  
		close(s);
        return -1;  
    }  

	close(s);
  
    return 0;  
}  

int main(int argc, char *argv[])
{
	int add, ret = -1;
	const char* brname = NULL;
	const char* ifname = NULL;

	if (argc < 4)
	{
		printf("%s interface brname add/del[1/0]\n", argv[0]);
		return -1;
	}

	ifname = argv[1];
	brname = argv[2];	

	add = atoi(argv[3]);
	if (add)
		ret = bridgeAddPort(ifname, brname);
	else
		ret = bridgeDelPort(ifname, brname);

	assert(ret == 0);
	
	return 0;
}

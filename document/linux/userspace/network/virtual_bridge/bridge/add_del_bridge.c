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

int addBridge(const char *brname)
{
    int fd;  
	
	if (unlikely(brname == NULL))
		return -1;
  
    if((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
    {  
        printf("Error socket :%m\n", errno);  
        return -1;  
    }

	if(ioctl(fd, SIOCBRADDBR, brname) < 0)  
    {  
        printf("Error add bridge %s :%m\n", brname, errno);  
		close(fd);
        return -1;  
    }  

	close(fd);
  
    return 0;  
}  

int interfaceDown(const char *interface_name)  
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
        printf("Error Down %s :%m\n", interface_name, errno);  
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


int delBridge(const char *brname)
{
    int fd;  

	if (unlikely(brname == NULL))
		return -1;

	if (interfaceDown(brname) < 0)
		return -1;
  
    if((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)  
    {  
        printf("Error socket :%m\n", errno);  
        return -1;  
    } 

	if(ioctl(fd, SIOCBRDELBR, brname) < 0)  
    {  
        printf("Error del bridge %s :%m\n", brname, errno);  
		close(fd);
        return -1;  
    }  

	close(fd);
  
    return 0;  
}  

int main(int argc, char *argv[])
{
	int add, ret = -1;
	const char* brname = NULL;

	if (argc < 3)
	{
		printf("%s brname add/del[1/0]\n", argv[0]);
		return -1;
	}

	brname = argv[1];

	add = atoi(argv[2]);
	if (add)
		ret = addBridge(brname);
	else
		ret = delBridge(brname);

	assert(ret == 0);
	
	return 0;
}

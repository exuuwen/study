#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "rtnetlink_addr.h"

void usage(char *func)
{
	printf("%s -a(add) -i ifname(ethx:x) -d address(x.x.x.x/x)\n", func);
	printf("%s -r(rm) -i ifname(ethx:x) -d address(x.x.x.x/x)\n", func);
}


int main(int argc, char *argv[])
{
    unsigned char *ifn = NULL;
    unsigned char *ip = NULL;
    int ret, opt;
    int add = -1;

    if (argc < 2)
    {
        usage(argv[0]);
        return 0;
    }

    while ((opt = getopt(argc, argv, "+ari:d:")) != -1) 
    {
        switch (opt) 
        {
          case 'd': ip = optarg;  break;
          case 'i': ifn = optarg; break;
          case 'a': add = 1;      break;
          case 'r': add = 0;      break;
          default:  usage(argv[0]);
              goto out;
        }
    }

    if (!ifn && !ip)
    {
        usage(argv[0]);
        goto out;
    }
    
    if (add == 1)
    {
        ret = create_addr(ifn, ip);
        if (ret != 0)
        {
            printf("create fail...\n");
        }
        else
        {
            printf("create ok ....\n");
        }
    }
    else if (add == 0)
    {
        ret = delete_addr(ifn, ip);
        if (ret != 0)
        {
            printf("delete fail...\n");
        }
        else
        {
            printf("delete ok ....\n");
        }
    }
    else
    {
        usage(argv[0]);
    }
	
out:
    return 0;
}





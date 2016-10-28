#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "rtnetlink_neigh.h"

void usage(char *func)
{
	printf("%s -a(add) -i ifname -m mac(x:x:x:x:x:x) -d address(x.x.x.x) -p (permanent)\n", func);
	printf("%s -r(rm) -i ifname -d address(x.x.x.x)\n", func);
}

int main(int argc, char *argv[])
{
    char *ip = NULL;
    unsigned char macs[6] = {0xa, 0x2, 0xe3, 0xc4, 0xd5, 0xa6};
    char *ifname = NULL;
    char *mac = NULL;
    int add = -1;
    int ret, opt;
    int permanent = 0;

    if (argc < 2)
    {
        usage(argv[0]);
        return 0;
    }

    while ((opt = getopt(argc, argv, "+arpm:i:d:")) != -1) 
    {
        switch (opt) 
        {
          case 'd': ip = optarg;     break;
          case 'i': ifname = optarg; break;
          case 'm': mac = optarg;    break;
          case 'p': permanent = 1;   break;
          case 'a': add = 1;         break;
          case 'r': add = 0;         break;
          default:  usage(argv[0]);
              goto out;
        }
    }

    if (!ip || !ifname)
    {
        usage(argv[0]);
        return 0;
    }
	
    if (add == 1)
    {
        if (!mac)
        {
            usage(argv[0]);
            return 0;
        }

        ret = create_neigh(ip, macs, ifname, permanent);
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
        ret = delete_neigh(ip, ifname);
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


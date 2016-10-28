#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "rtnetlink_route.h"

void usage(char *func)
{
	printf("%s -a(add) [-i ifname | -g gateway(x.x.x.x)] { -d address(x.x.x.x/x)]\n", func);
	printf("%s -r(rm) [-i ifname | -g gateway(x.x.x.x)] [ -d address(x.x.x.x/x)]\n", func);
}

int main(int argc, char *argv[])
{
    char *ip = NULL;
    char *gw = NULL;
    char *ifname = NULL;
    int add = -1;
    int n = 0;
    int ret, opt;

    if (argc < 2)
    {
        usage(argv[0]);
        return 0;
    }

    while ((opt = getopt(argc, argv, "+arg:i:d:n:")) != -1) 
    {
        switch (opt) 
        {
          case 'd': ip = optarg;     break;
          case 'i': ifname = optarg; break;
          case 'g': gw = optarg;     break;
          case 'a': add = 1;         break;
          case 'r': add = 0;         break;
          default:  usage(argv[0]);
              goto out;
        }
    }

    if (!gw && !ifname)
    {
        usage(argv[0]);
        return 0;
    }
	
    if (add == 1)
    {
        ret = create_route(gw, ip, ifname);
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
        ret = delete_route(gw, ip, ifname);
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





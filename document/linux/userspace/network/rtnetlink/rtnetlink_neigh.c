#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>

#include "rtnetlink.h"
#include "rtnetlink_neigh.h"

#include "if_ops.h"

static int modify_neigh(unsigned short type, struct in_addr* ip, unsigned char *mac, int if_index, int permanent)
{
    unsigned int seq;
    struct sockaddr_nl loc_addr;
    rt_neigh_request_t r;
    int ret = 0;
    int fd = 0;

    printf("modifyNiegh: begin\n");

    memset(&r, 0, sizeof(rt_neigh_request_t));

    r.nd.ndm_family = AF_INET;

    if (type == RTM_NEWNEIGH)
    {
        rt_route_init(&r.n, RTM_NEWNEIGH, NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE /*| NLM_F_EXCL*/);/*create new or modify old*/
        
        rt_addAttr_data(&r.n, NDA_LLADDR, mac, 6);
        if (permanent)
            r.nd.ndm_state = NUD_PERMANENT;
        else
            r.nd.ndm_state = NUD_REACHABLE;
        r.nd.ndm_flags = 0x02;//NTF_SELF;
    }
    else if (type == RTM_DELNEIGH)
    {
        rt_route_init(&r.n, RTM_DELNEIGH, NLM_F_REQUEST);
    }

    r.nd.ndm_ifindex = if_index;
    rt_addAttr_data(&r.n, NDA_DST, ip, sizeof(*ip));
    /*--------*/
    /* Now open a netlink socket */
    if ((fd = rt_open(&loc_addr)) < 0) 
    {
        ret = -1;
        goto end;
    }

    seq = time(NULL);

    if (rt_send(fd, &seq, &r.n, &loc_addr) < 0)
    {
        ret = -1;
        goto end;
    }

    printf("modifyNeigh: happy end. \n"); 

end:
    if (fd > 0) 
    {
        close(fd);
    }

    return ret;
}

int create_neigh(const char *ip, unsigned char *mac, const char* ifname, int permanent)
{
    struct in_addr ip_addr;
    int index;

    inet_aton(ip, &ip_addr);
    index = if_index(ifname);
    if (modify_neigh(RTM_NEWNEIGH, &ip_addr, mac, index, permanent) < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}


int delete_neigh(const char *ip, const char* ifname)
{
    struct in_addr ip_addr;
    int index;

    inet_aton(ip, &ip_addr);
    index = if_index(ifname);
    if (modify_neigh(RTM_DELNEIGH, &ip_addr, NULL, index, 0) < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}



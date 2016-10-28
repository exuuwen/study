#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>

#include "rtnetlink.h"
#include "rtnetlink_route.h"

#include "if_ops.h"

static int modify_route(unsigned short type, struct in_addr* destip, struct in_addr* gwip, int index, int dst_len)
{
    unsigned int seq;
    struct sockaddr_nl loc_addr;
    rt_route_request_t r;
    int ret = 0;
    int fd = 0;

    printf("modifyRoute: begin\n");

    memset(&r, 0, sizeof(rt_route_request_t));

    if (type == RTM_NEWROUTE)
    {
        rt_route_init(&r.n, RTM_NEWROUTE, NLM_F_REQUEST | NLM_F_CREATE | NLM_F_REPLACE);
    }
    else if (type == RTM_DELROUTE)
    {
        rt_route_init(&r.n, RTM_DELROUTE, NLM_F_REQUEST);
    }

    r.rt.rtm_family = AF_INET;
    r.rt.rtm_table = RT_TABLE_MAIN;
    r.rt.rtm_protocol = RTPROT_STATIC; 
    if (gwip)
        r.rt.rtm_scope = RT_SCOPE_UNIVERSE;
    else
        r.rt.rtm_scope = RT_SCOPE_LINK;
    r.rt.rtm_type = RTN_UNICAST;

    
    if (gwip)
        rt_addAttr_data(&r.n, RTA_GATEWAY, gwip, sizeof(*gwip));
	
    if (index >= 0)
        rt_addAttr_data(&r.n, RTA_OIF, (char*)(&index), sizeof(int));
		
    if (destip)
    {
        rt_addAttr_data(&r.n, RTA_DST, destip, sizeof(*destip));
        r.rt.rtm_dst_len = dst_len;
    }

     /*rt_addAttr_data(&r.n, RTA_PREFSRC, &pref_src, sizeof(pref_src));
    printf("dst_len:%d\n", pref_src);*/

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

    printf("modifyRoute: happy end. \n"); 

end:
    if (fd > 0) 
    {
        close(fd);
    }

    return ret;
}

static int _create_route(const char *gw_ip, const char *dst_ip, const char *ifname, int dst_len)
{
    int ret = 0;
    struct in_addr *gw_addr = NULL;
    struct in_addr *dst_addr = NULL;
    int index = -1;
   
    if (!gw_ip && !ifname)
    {
        printf("invalid parameters for create route\n");
        return -1;
    } 
    
    if (gw_ip)
    {
        gw_addr = (struct in_addr*)malloc(sizeof(struct in_addr));
        inet_aton(gw_ip, gw_addr);
    }

    if (dst_ip)
    {
        dst_addr = (struct in_addr*)malloc(sizeof(struct in_addr));
        inet_aton(dst_ip, dst_addr);
    }

    if (ifname)
    {
        index = if_index(ifname);
        if (index < 0)
        {
            printf("wrong ifname in create route\n");
            goto out;
            ret = -1;
        } 
    }

    if (modify_route(RTM_NEWROUTE, dst_addr, gw_addr, index, dst_len) < 0)
    {
        ret = -1;
    }
    else
    {
        ret = 0;
    }

out:
    free(gw_addr);
    free(dst_addr);

    return ret;
}


static int _delete_route(const char *gw_ip, const char *dst_ip, const char *ifname, int dst_len)
{
    int ret = 0;
    struct in_addr *gw_addr = NULL;
    struct in_addr *dst_addr = NULL;
    int index = -1;

    if (!gw_ip && !ifname)
    {
        printf("invalid parameters for delete route\n");
        return -1;
    } 
    
    if (gw_ip)
    {
        gw_addr = (struct in_addr*)malloc(sizeof(struct in_addr));
        inet_aton(gw_ip, gw_addr);
    }

    if (dst_ip)
    {
        dst_addr = (struct in_addr*)malloc(sizeof(struct in_addr));
        inet_aton(dst_ip, dst_addr);
    }

    if (ifname)
    {
        index = if_index(ifname);
        if (index < 0)
        {
            printf("wrong ifname in create route\n");
            return -1;
        } 
    }

    if (modify_route(RTM_DELROUTE, dst_addr, gw_addr, index, dst_len) < 0)
    {
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    free(gw_addr);
    free(dst_addr);

    return ret;
}

int create_route(const char *gw_ip, const char *addr, const char *ifname)
{
    int dst_len = 32;
    char *mask;
    char *dst_ip = NULL;

    if (addr)
    {
       dst_ip = strdup(addr);
        char *p2 = strstr(dst_ip, "/");
        if (p2)
        {
            mask = p2 + 1;
            dst_len = atoi(mask);
            memset(p2, '\0', strlen(p2));
        }
    }
        

    return  _create_route(gw_ip, dst_ip, ifname, dst_len);
}

int delete_route(const char *gw_ip, const char *addr, const char *ifname)
{
    int dst_len = 32;
    char *mask;
    char *dst_ip = NULL;

    if (addr)
    {
        dst_ip = strdup(addr);
        char *p2 = strstr(dst_ip, "/");
        if (p2)
        {
            mask = p2 + 1;
            dst_len = atoi(mask);
            memset(p2, '\0', strlen(p2));
        }
    }

    return  _delete_route(gw_ip, dst_ip, ifname, dst_len);
}


#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>

#include "rtnetlink.h"
#include "rtnetlink_addr.h"

#include"if_ops.h"

static int modify_addr(int cmd, struct in_addr *addr, int index, int netmask, const char* ifname)
{
    int llen; 
    unsigned int seq;
    struct sockaddr_nl loc_addr;
    rt_addr_request_t r;
    int ret = 0;
    int fd = 0;

    printf("modifyAddr: begin. \n");
	
    memset(&r, 0, sizeof(r));
	
    r.ifa.ifa_family = AF_INET; 
    r.ifa.ifa_index = index;

    if (cmd == RTM_NEWADDR) 
    {		
        rt_addr_init(&r.n, RTM_NEWADDR, NLM_F_REQUEST | NLM_F_REPLACE  | NLM_F_CREATE);	
    } 
    else if (cmd == RTM_DELADDR) 
    {
        rt_addr_init(&r.n, RTM_DELADDR, NLM_F_REQUEST);
    }

    rt_addAttr_data(&r.n, IFA_LOCAL, addr, sizeof(*addr));
    rt_addAttr_data(&r.n, IFA_ADDRESS, addr, sizeof(*addr));
    r.ifa.ifa_prefixlen = netmask;
	
    /* Finally, also add a label attribute */
    llen = strlen(ifname) + 1;
    rt_addAttr_data(&r.n, IFA_LABEL, ifname, llen);

    /*--------*/
    /* Now open a netlink socket */
    if ((fd = rt_open(&loc_addr)) < 0) 
    {
        ret = -1;
        goto end;
    }

    seq = time(NULL);
    /* Send the attribute message and wait for ACK */	
    if (rt_send(fd, &seq, &r.n, &loc_addr) < 0) 
    {
        ret = -1;
        goto end;
    }

    /*---------*/
    printf("modifyAddr: happy end. \n"); 
	
end:
    if (fd > 0) 
    {
        close(fd);
    }

    return ret;
}

static int _create_addr(const char *ifname, const char *ip, int netmask)
{
    int ret, index;
    struct in_addr addr;
    int r = -1;

    inet_aton(ip, &addr);
    index = if_index(ifname);
    if (index < 0)
    {
        return -1;
    }

    if (netmask <= 0 || netmask > 32)
        netmask = 32;
	
    
    ret = modify_addr(RTM_NEWADDR, &addr, index, netmask, ifname);	
    if (ret < 0)
    {
        printf("Failed to create %s with address %s/%d\n", ifname, ip, netmask);
        goto done;
    }
    else
    {
        printf("Success to create %s with address %s/%d\n", ifname, ip, netmask);
    }

    r = 0;

done:
    return r;
}

static int _delete_addr(const char *ifname, const char *ip, int netmask)
{
    int index;
    struct in_addr addr;
    int r = -1;

    inet_aton(ip, &addr);
    index = if_index(ifname);
    if (index < 0)
    {
        return -1;
    }

    if (netmask <= 0 || netmask > 32)
        netmask = 32;
	
    if (modify_addr(RTM_DELADDR, &addr, index, netmask, ifname) < 0)
    {
        printf("Failed to delete address %s/%d on %s\n", ip, netmask, ifname);
        goto done;
    }
    else
    {
        printf("Sucess to delete address %s/%d on %s\n", ip, netmask, ifname);
    }

    r = 0;

done:
    return r;
}

int create_addr(const char *ifname, const char *addr)
{
    int netmask = -1;
    char *mask;
    char *ip = NULL;
    char *ipaddr = strdup(addr);
    char *p2 = strstr(ipaddr, "/");
    if (p2)
    {
        mask = p2 + 1;
        netmask = atoi(mask);
        memset(p2, '\0', strlen(p2));
    }

    return  _create_addr(ifname, ipaddr, netmask);
}

int delete_addr(const char *ifname, const char *addr)
{
    int netmask = -1;
    char *mask;
    char *ipaddr = strdup(addr);
    char *p2 = strstr(ipaddr, "/");
    if (p2)
    {
        mask = p2 + 1;
        netmask = atoi(mask);
        memset(p2, '\0', strlen(p2));
    }

    return  _delete_addr(ifname, ipaddr, netmask);
}




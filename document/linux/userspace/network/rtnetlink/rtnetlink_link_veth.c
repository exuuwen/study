#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <linux/veth.h>
#include <netinet/in.h>

#include "rtnetlink.h"
#include "rtnetlink_link_vnet.h"

static int modify_vnet_link(int cmd, const char* if_name, const char* peer_name, int ns_pid)
{
    int llen; 
    int fd = 0;
    unsigned int seq;
    struct sockaddr_nl loc_addr;
    rt_link_request_t r;
    int ret = 0;

    printf("modifyLink: begin. \n");
	
    memset(&r, 0, sizeof(r));	

    r.ifi.ifi_family = PF_UNSPEC; 
    r.ifi.ifi_index = 0;
    r.ifi.ifi_flags = 0;
	
    if (cmd == RTM_NEWLINK) 
    {	
        rt_link_init(&r.n, RTM_NEWLINK, NLM_F_REQUEST | NLM_F_REPLACE  | NLM_F_CREATE);

        /*unsigned char mac[6] = { 0xa, 1, 2, 3, 4, 5};
        rt_addAttr_data(&r.n, IFA_ADDRESS, mac, 6);*/

        llen = strlen(if_name) + 1;
        rt_addAttr_data(&r.n, IFLA_IFNAME, if_name, llen);

        char* if_kind = "veth";
		
        struct rtattr *rta_linkinfo = rt_addAttr_hdr(&r.n, IFLA_LINKINFO);
        rt_addAttr_data(&r.n, IFLA_INFO_KIND, if_kind, strlen(if_kind) + 1);
	
        struct rtattr *rta_datainfo = rt_addAttr_hdr(&r.n, IFLA_INFO_DATA);

        struct ifinfomsg ifi_p;
        ifi_p.ifi_family = PF_UNSPEC; 
	ifi_p.ifi_index = 0;
	ifi_p.ifi_flags = 0;
		
	struct rtattr *rta_peerinfo = rt_addAttr_data(&r.n, VETH_INFO_PEER, (void*)&ifi_p, sizeof(struct ifinfomsg));
		
	llen = strlen(peer_name) + 1;
	rt_addAttr_data(&r.n, IFLA_IFNAME, peer_name, llen);

        if (ns_pid > 0)
        {
	    unsigned int pid = ns_pid;
	    rt_addAttr_data(&r.n, IFLA_NET_NS_PID, (void*)&pid, sizeof(unsigned int));
        }
	
        /*unsigned char mac2[6] = { 0xa, 1, 2, 3, 4, 6};
	rt_addAttr_data(&r.n, IFA_ADDRESS, mac2, 6);*/

	rt_compAttr_hdr(&r.n, rta_peerinfo);
	rt_compAttr_hdr(&r.n, rta_datainfo);
	rt_compAttr_hdr(&r.n, rta_linkinfo);		
    } 
    else if (cmd == RTM_DELLINK) 
    {
        rt_link_init(&r.n, RTM_DELLINK, NLM_F_REQUEST);
	llen = strlen(if_name) + 1;
	rt_addAttr_data(&r.n, IFLA_IFNAME, if_name, llen);
    }

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
    printf("modifyLink: happy end. \n"); 
	
end:
    if (fd > 0) 
    {
        close(fd);
    }

    return ret;
}

int create_veth_link(const char *master_name, const char *peer_name, int ns_pid)
{
    int r = 0;

    if (modify_vnet_link(RTM_NEWLINK, master_name, peer_name, ns_pid) < 0)
    {
        printf("Failed to create %s with peer %s\n", master_name, peer_name);
        r = -1;
    }
    else
    {
        printf("Created %s with peer %s success!\n", master_name, peer_name);
    }

    return r;
}

int delete_veth_link(const char *if_name)
{
    int r = 0;

    if (modify_vnet_link(RTM_DELLINK, if_name, NULL, 0) < 0)
    {
        printf("Failed to delete %s\n", if_name);
        r = -1;
    }
    else
    {
	printf("sucess to delete %s\n", if_name);
    }

    return r;
}

int main(int argc, char *argv[])
{
	struct in_addr addr;

	if (argc < 2)
	{
		printf("%s create mastername peername\n", argv[0]);
		printf("%s delete ifname\n", argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "create") == 0)
	{
		if (argc < 4)
		{
			printf("%s create mastername peername\n", argv[0]);
			return 0;
		}
		
		int ret = create_veth_link(argv[2], argv[3], -1);
		if (ret != 0)
		{
			printf("create fail...\n");
		}
		else
		{
			printf("create ok ....\n");
		}
	}
	else if (strcmp(argv[1], "delete") == 0)
	{
		if (argc < 3)
		{
			printf("%s delete ifname\n", argv[0]);
			return 0;
		}

		int ret = delete_veth_link(argv[2]);
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
		printf("%s create mastername peername\n", argv[0]);
		printf("%s delete ifname\n", argv[0]);
	}	

	return 0;
}





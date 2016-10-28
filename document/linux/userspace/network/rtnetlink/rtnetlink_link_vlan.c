#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "rtnetlink.h"

static int modify_vlan_link(unsigned int master_index, int cmd, const char* if_name,unsigned short vid)
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

		rt_addAttr_data(&r.n, IFLA_LINK, &master_index, sizeof(master_index));
		
		struct rtattr *rta_linkinfo = rt_addAttr_hdr(&r.n, IFLA_LINKINFO);
		char* if_kind = "vlan";
		rt_addAttr_data(&r.n, IFLA_INFO_KIND, if_kind, strlen(if_kind) + 1);
	
		struct rtattr *rta_datainfo = rt_addAttr_hdr(&r.n, IFLA_INFO_DATA);
		rt_addAttr_data(&r.n, IFLA_VLAN_ID, &vid, sizeof(unsigned short));
		
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

int create_vlan_link(const char *master_name, int vid)
{
	struct ifreq  ifr;
	int ipfd = 0;
	int r = -1;

	char child_if[20];
	if ((ipfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
    		goto done;
  	}
	strncpy(ifr.ifr_name, master_name, sizeof(ifr.ifr_name) - 1);

	if (ioctl(ipfd, SIOCGIFINDEX, &ifr) < 0) 
	{
		printf("ioctl failure (SIOCGIFINDEX), errno = %d\n", errno);
		goto done;
	} 

	snprintf(child_if, sizeof child_if, "%s.%u", master_name, vid);
	if (modify_vlan_link(ifr.ifr_ifindex, RTM_NEWLINK, child_if, vid) < 0)
	{
		printf("Failed to create %s with vid %d\n", child_if, vid);
		goto done;
	}
	else
	{
		printf("Created %s with vid %d\n", child_if, vid);
	}

	r = 0;

done:
	if (ipfd > 0) 
	{
		close(ipfd);
	}

	return r;
}

int delete_vlan_link(const char *if_name)
{
	int r = -1;

	if (modify_vlan_link(0, RTM_DELLINK, if_name, 0) < 0)
	{
		printf("Failed to delete %s\n", if_name);
		goto done;
	}
	else
	{
		printf("sucess to delete %s\n", if_name);
	}

	r = 0;

done:
	return r;
}

int main(int argc, char *argv[])
{
	struct in_addr addr;

	if (argc < 2)
	{
		printf("%s create mastername vid \n", argv[0]);
		printf("%s delete ifname\n", argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "create") == 0)
	{
		if (argc < 4)
		{
			printf("usage: %s create mastername vid\n", argv[0]);
			return 0;
		}
		
		int vid = atoi(argv[3]);
		int ret = create_vlan_link(argv[2], vid);
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
			printf("usage: %s delete ifname\n", argv[0]);
			return 0;
		}

		int ret = delete_vlan_link(argv[2]);
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
		printf("usage: %s create mastername vid\n", argv[0]);
		printf("%s delete ifname\n", argv[0]);
	}	

	return 0;
}




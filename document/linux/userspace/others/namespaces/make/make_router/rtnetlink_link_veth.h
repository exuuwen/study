#ifndef _RTNETLINK_LINK_VNET_
#define _RTNETLINK_LINK_VNET_

int create_veth_link(const char *master_name, const char *peer_name, int ns_pid);

int delete_veth_link(const char *if_name);

#endif 


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libvirt/libvirt.h>

#define NETWORK_DESTROY 		0
#define NETWORK_SET_AUTOSTART		1
#define NETWORK_UNSET_AUTOSTART		2


static int network_op(char * name, int op)
{
	virNetworkPtr net;
	virConnectPtr conn;
	int ret;
	
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) 
	{
		fprintf(stderr, "Failed to open connection to qemu:///system\n");
		return -1;
	}
	
	net = virNetworkLookupByName(conn, name);
	if (net == NULL) 
	{
		printf("network is not found\n");
		virConnectClose(conn);
		return -1;
	}

	if (op == NETWORK_DESTROY)
		ret = virNetworkDestroy(net);
	else if (op == NETWORK_SET_AUTOSTART)
		ret = virNetworkSetAutostart(net, 1);
	else if (op == NETWORK_UNSET_AUTOSTART)
		ret = virNetworkSetAutostart(net, 0);
	else
		ret = -1;
		
	virNetworkFree(net);
	virConnectClose(conn);
	
	return ret;
}



int libvirt_network_destroy(char * name)
{
	if (network_op(name, NETWORK_DESTROY) < 0) 
	{
		printf("network %s destroy:error\n", name);
		return -1;
	}
	return 0;
}

int libvirt_network_unautostart(char * name)
{
	if (network_op(name, NETWORK_UNSET_AUTOSTART) < 0) 
	{
		printf("network %s unautostart:error\n", name);
		return -1;
	}
	return 0;
}

int libvirt_network_autostart(char * name)
{
	if (network_op(name, NETWORK_SET_AUTOSTART) < 0) 
	{
		printf("network %s autostart:error\n", name);
		return -1;
	}
	return 0;
}


int main(int argc, char *argv[])
{
	int ret = -1;
	
	if(argc != 3)
	{	
		printf("usage: ./start_network network_name option\n");
		return -1;
	}
	
	if(strcmp(argv[2], "destroy") == 0)
	{
		ret = libvirt_network_destroy(argv[1]);
	}
	else if(strcmp(argv[2], "autostart") == 0)
	{
		ret = libvirt_network_autostart(argv[1]);
	}
	else if(strcmp(argv[2], "unautostart") == 0)
	{
		ret = libvirt_network_unautostart(argv[1]);
	}
	
	if(!ret)
	{
		printf("network %s %s success\n", argv[1], argv[2]);
	}
	else
	{
		printf("network %s %s fail\n", argv[1], argv[2]);
	}
	
	return ret;
	
}


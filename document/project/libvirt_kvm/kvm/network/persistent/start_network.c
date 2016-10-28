#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libvirt/libvirt.h>
#include <libvirt/libvirt-qemu.h>

int main(int argc, char *argv[])
{
	virConnectPtr conn;
	virNetworkPtr net;
	
	if(argc != 2)
	{	
		printf("usage: ./start_network domain_network\n");
		return -1;
	}
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) 
	{
		fprintf(stderr, "Failed to open connection to qemu:///system\n");
		return -1;
	}
	
	net = virNetworkLookupByName(conn, argv[1]);
	if (!net) 
	{
		fprintf(stderr, "Network is not found\n");
		virConnectClose(conn);
		return -1;
	}
	
	if (virNetworkCreate(net) < 0) 
	{
		virNetworkFree(net);
		virConnectClose(conn);
		fprintf(stderr, "Cannot start network\n");
		return -1;
	}
	
	fprintf(stderr, "Network %s has started\n", argv[1]);
	
	virNetworkFree(net);
	virConnectClose(conn);
	
	return 0;
}


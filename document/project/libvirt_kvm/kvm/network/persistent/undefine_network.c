#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libvirt/libvirt.h>

int main(int argc, char *argv[])
{
	virConnectPtr conn;
	virNetworkPtr net;
	
	if(argc != 2)
	{	
		printf("usage: ./start_network network_name\n");
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
	
	if (virNetworkUndefine(net) < 0) 
	{
		virNetworkFree(net);
		virConnectClose(conn);
		fprintf(stderr, "Cannot undefine network\n");
		return -1;
	}
	
	fprintf(stderr, "Network %s has undefine\n", argv[1]);
	virNetworkFree(net);
	virConnectClose(conn);
	
	return 0;
}


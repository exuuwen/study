#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libvirt/libvirt.h>
int main(int argc, char *argv[])
{
	virConnectPtr conn;
	int i;
	int numNetworks;
	char **activeNetworks;
	char **inactiveNetworks;
	char *buf;
	
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) 
	{
		fprintf(stderr, "Failed to open connection to qemu:///system\n");
		return -1;
	}
	
	numNetworks = virConnectNumOfNetworks(conn);
	if (numNetworks == -1) 
	{
		fprintf(stderr, "Failed to get Network num of qemu\n");
		return -1;
	}
	
	buf = malloc(20);
	
	activeNetworks = malloc(sizeof(char *) * numNetworks);
	numNetworks = virConnectListNetworks(conn, activeNetworks, numNetworks);
	
	printf("%d Active Networks list:\n", numNetworks);
	for (i = 0 ; i < numNetworks ; i++) 
	{
		virNetworkGetUUIDString(virNetworkLookupByName(conn, activeNetworks[i]), buf);
		printf("Name = %s, UUID = %s, bridge name: %s\n", activeNetworks[i], buf,  virNetworkGetBridgeName(virNetworkLookupByName(conn, activeNetworks[i])));
		free(activeNetworks[i]);
	}
	free(activeNetworks);
	
	printf("----------------------------\n");
	numNetworks = virConnectNumOfDefinedNetworks(conn);
	if (numNetworks == -1) 
	{
		fprintf(stderr, "Failed to get defined Network num of qemu\n");
		return -1;
	}

	inactiveNetworks = malloc(sizeof(char *) * numNetworks);
	numNetworks = virConnectListDefinedNetworks(conn, inactiveNetworks, numNetworks);
	
	printf("%d Inactive Networks list:\n", numNetworks);
	for (i = 0 ; i < numNetworks ; i++) 
	{
		
		virNetworkGetUUIDString(virNetworkLookupByName(conn, activeNetworks[i]), buf);
		printf("Name = %s, UUID = %s, bridge name: %s\n", activeNetworks[i], buf, virNetworkGetBridgeName(virNetworkLookupByName(conn, activeNetworks[i])));
		free(inactiveNetworks[i]);
	}
	free(inactiveNetworks);
	
	virConnectClose(conn);
	
	return 0;
}


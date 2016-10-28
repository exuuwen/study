#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libvirt/libvirt.h>
int main(int argc, char *argv[])
{
	virConnectPtr conn;
	int numIfaces, i;
	char **ifaceNames;
	
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) 
	{
		fprintf(stderr, "Failed to open connection to qemu:///system\n");
		return -1;
	}

	numIfaces = virConnectNumOfInterfaces(conn);
	if (numIfaces == -1) 
	{
		fprintf(stderr, "Failed to get active interface num of qemu\n");
		virConnectClose(conn);
		return -1;
	}
	
	ifaceNames = malloc(numIfaces * sizeof(char*));
	numIfaces = virConnectListInterfaces(conn, ifaceNames, numIfaces);

	printf("Active host interfaces:\n");
	for (i = 0; i < numIfaces; i++) 
	{
		printf("%d interface:%s, Mac:%s\n", i, ifaceNames[i], virInterfaceGetMACString(virInterfaceLookupByName(conn, ifaceNames[i])));
		free(ifaceNames[i]);
	}
	
	free(ifaceNames);

	numIfaces = virConnectNumOfDefinedInterfaces(conn);
	if (numIfaces == -1) 
	{
		fprintf(stderr, "Failed to get inactive interface num of qemu\n");
		virConnectClose(conn);
		return -1;
	}
	ifaceNames = malloc(numIfaces * sizeof(char*));
	numIfaces = virConnectListDefinedInterfaces(conn, ifaceNames, numIfaces);
	
	printf("Inactive host interfaces:\n");
	for (i = 0; i < numIfaces; i++) 
	{
		printf("%d interface:%s, Mac:%s\n", i, ifaceNames[i], virInterfaceGetMACString(virInterfaceLookupByName(conn, ifaceNames[i])));
		free(ifaceNames[i]);
	}
	free(ifaceNames);

	virConnectClose(conn);

	return 0;	
}


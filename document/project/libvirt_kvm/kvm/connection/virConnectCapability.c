#include <stdio.h>
#include <stdlib.h>
#include <libvirt/libvirt.h>

/*
 gcc -o virConnectCapability virConnectCapability.c -lvirt
*/
int main(int argc, char *argv[])
{
	virConnectPtr conn;
	char *caps;
	
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) 
	{
		fprintf(stderr, "Failed to open connection to qemu:///system\n");
		return 1;
	}

	caps = virConnectGetCapabilities(conn);
	fprintf(stdout, "Capabilities:\n%s\n", caps);
	free(caps);

	virConnectClose(conn);

	return 0;
}

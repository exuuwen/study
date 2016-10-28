#include <stdio.h>
#include <stdlib.h>
#include <libvirt/libvirt.h>

/*
virConnectPtr virConnectOpen(const char *name)
int virConnectRef(virConnectPtr)
int virConnectClose(virConnectPtr)
*/

/*
 gcc -o virConnect virConnect.c -lvirt
*/


int main(int argc, char *argv[])
{
	virConnectPtr conn;
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) 
	{
		fprintf(stderr, "Failed to open connection to qemu:///system\n");
		return 1;
	}
	printf("Success to  open connection to qemu:///system\n");

	/* now the connection has a single reference to it */
	virConnectRef(conn);
	/* now the connection has two references to it */
	virConnectClose(conn);
	/* now the connection has one reference */
	virConnectClose(conn);
	
	return 0;
}

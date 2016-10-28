#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libvirt/libvirt.h>
#include <libvirt/libvirt-qemu.h>

int main(int argc, char *argv[])
{
	virConnectPtr conn;
	virDomainPtr dom;
	
	if(argc != 2)
	{	
		printf("usage: ./start_domain domain_name\n");
		return -1;
	}
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) 
	{
		fprintf(stderr, "Failed to open connection to qemu:///system\n");
		return -1;
	}
	
	dom = virDomainLookupByName(conn, argv[1]);
	if (!dom) 
	{
		fprintf(stderr, "Domain is not found\n");
		virConnectClose(conn);
		return -1;
	}
	
	if (virDomainCreate(dom) < 0) 
	{
		virDomainFree(dom);
		virConnectClose(conn);
		fprintf(stderr, "Cannot boot guest\n");
		return -1;
	}
	
	fprintf(stderr, "Guest %s has booted\n", argv[1]);
	
	virDomainFree(dom);
	virConnectClose(conn);
	
	return 0;
}

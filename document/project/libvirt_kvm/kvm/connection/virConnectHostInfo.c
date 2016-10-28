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
	char *host;
	int vcpus;
	unsigned long long node_free_memory;
	virNodeInfo nodeinfo;
	unsigned long ver;
	
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) 
	{
		fprintf(stderr, "Failed to open connection to qemu:///system\n");
		return 1;
	}

	host = virConnectGetHostname(conn);
	fprintf(stdout, "Hostname:%s\n", host);
	free(host);

	vcpus = virConnectGetMaxVcpus(conn, NULL);
	fprintf(stdout, "Maximum support virtual CPUs: %d\n", vcpus);

	/*
	node_free_memory = virNodeGetFreeMemory(conn);
	fprintf(stdout, "Node free memory: %llu\n", node_free_memory);
	*/
	virNodeGetInfo(conn, &nodeinfo);
	fprintf(stdout, "Model: %s\n", nodeinfo.model);
	fprintf(stdout, "Memory size: %lukb\n", nodeinfo.memory);
	fprintf(stdout, "Number of CPUs: %u\n", nodeinfo.cpus);
	fprintf(stdout, "MHz of CPUs: %u\n", nodeinfo.mhz);
	fprintf(stdout, "Number of NUMA nodes: %u\n", nodeinfo.nodes);
	fprintf(stdout, "Number of CPU sockets: %u\n", nodeinfo.sockets);
	fprintf(stdout, "Number of CPU cores per socket: %u\n", nodeinfo.cores);
	fprintf(stdout, "Number of CPU threads per core: %u\n", nodeinfo.threads);

	fprintf(stdout, "Virtualization type: %s\n", virConnectGetType(conn));

	fprintf(stdout, "Canonical URI: %s\n", virConnectGetURI(conn));

	virConnectGetLibVersion(conn, &ver);
	fprintf(stdout, "Libvirt Version: %lu\n", ver);
	
	virConnectClose(conn);

	return 0;
}


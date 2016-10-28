#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libvirt/libvirt.h>
int main(int argc, char *argv[])
{
	virConnectPtr conn;
	int i;
	int numDomains;
	int *activeDomains;
	char **inactiveDomains;
	
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) 
	{
		fprintf(stderr, "Failed to open connection to qemu:///system\n");
		return -1;
	}

	numDomains = virConnectNumOfDomains(conn);
	if (numDomains == -1) 
	{
		fprintf(stderr, "Failed to get domian num of qemu\n");
		return -1;
	}
	
	activeDomains = malloc(sizeof(int) * numDomains);
	numDomains = virConnectListDomains(conn, activeDomains, numDomains);
	
	printf("%d Active domain list:\n", numDomains);
	for (i = 0 ; i < numDomains ; i++) 
	{
		printf("ID = %d, Name = %s\n", activeDomains[i], virDomainGetName(virDomainLookupByID(conn, activeDomains[i])));
	}
	free(activeDomains);
	
	printf("----------------------------\n");
	numDomains = virConnectNumOfDefinedDomains(conn);
	
	if (numDomains == -1) 
	{
		fprintf(stderr, "Failed to get defined domian num of qemu\n");
		return -1;
	}
	
	inactiveDomains = malloc(sizeof(char *) * numDomains);
	numDomains = virConnectListDefinedDomains(conn, inactiveDomains, numDomains);
	printf("%d Inactive domain list:\n", numDomains);
	for (i = 0 ; i < numDomains ; i++) 
	{
		/*All inactive domains's id should be 0*/
		printf("ID = %d, Name = %s\n", virDomainGetID(virDomainLookupByName(conn, inactiveDomains[i])), inactiveDomains[i]);
		free(inactiveDomains[i]);
	}
	free(inactiveDomains);
	
	virConnectClose(conn);
	return 0;
}

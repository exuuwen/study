#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <libvirt/libvirt.h>
#include <libvirt/libvirt-qemu.h>


static char * GetXml(const char *filename)
{
	int fd;
	char *xmlconfig;
	struct stat filestate;
	
	fd = open(filename, O_RDONLY); 
	if(fd < 0) 
	{ 
		printf("open file failed \n"); 
		return NULL; 
	} 
	
	if(stat(filename, &filestate) < 0) 
	{ 
		perror("get the state of file failed\n");
		return NULL; 
	} 

	if(filestate.st_size > 0)
	{
		xmlconfig = (char *) malloc(sizeof(char) * filestate.st_size); 
		if(xmlconfig == NULL) 
		{ 
			perror("failed to get memory \n");
			return NULL;
		}
	}

	if(read(fd,xmlconfig,filestate.st_size) < 0)
	{
		perror("failed to read\n"); 
		return NULL;
	}

	return xmlconfig;
}

int main(int argc, char *argv[])
{
	virConnectPtr conn;
	virDomainPtr dom;
	char *xml;
	
	if(argc != 2)
	{	
		printf("usage: ./create_start_domain domain.xml\n");
		return -1;
	}

	if ((xml = GetXml(argv[1])) == NULL)
	{
		fprintf(stderr, "Failed to get xml\n");
		return -1;
	}
		
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) 
	{
		fprintf(stderr, "Failed to open connection to qemu:///system\n");
		free(xml);
		return -1;
	}
	
	dom = virDomainCreateXML(conn, xml, 0);
	if (!dom) 
	{
		fprintf(stderr, "Domain creation failed\n");
		free(xml);
		virConnectClose(conn);
		return -1;
	}
	
	fprintf(stderr, "Guest %s has booted\n", virDomainGetName(dom));
	
	free(xml);
	virDomainFree(dom);
	virConnectClose(conn);
	
	return 0;
}

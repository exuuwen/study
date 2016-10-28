#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <libvirt/libvirt.h>

#define DOMAIN_STOP 		0
#define DOMAIN_STOP_FORCE 	1
#define DOMAIN_REBOOT 		2
#define DOMAIN_SUSPEND 		3
#define DOMAIN_RESUME 		4

static int domain_op(char * name, int op)
{
	virDomainPtr domain;
	virConnectPtr conn;
	int ret;
	
	conn = virConnectOpen("qemu:///system");
	if (conn == NULL) 
	{
		fprintf(stderr, "Failed to open connection to qemu:///system\n");
		return -1;
	}
	
	domain = virDomainLookupByName(conn, name);
	if (domain == NULL) 
	{
		printf("domain is not found\n");
		virConnectClose(conn);
		return -1;
	}

	if (op == DOMAIN_STOP)
		ret = virDomainShutdown(domain);
	else if (op == DOMAIN_STOP_FORCE)
		ret = virDomainDestroy(domain);
	else if (op == DOMAIN_REBOOT)
		ret = virDomainReboot(domain, 0);
	else if (op == DOMAIN_SUSPEND)
		ret = virDomainSuspend(domain);
	else if (op == DOMAIN_RESUME)
		ret = virDomainResume(domain);
	else
		ret = -1;
		
	virDomainFree(domain);
	virConnectClose(conn);
	
	return ret;
}

int libvirt_domain_stop(char * name)
{
	if (domain_op(name, DOMAIN_STOP) < 0) 
	{
		printf("domain %s stop:error\n", name);
		return -1;
	}
	return 0;
}

int libvirt_domain_poweroff(char * name)
{
	if (domain_op(name, DOMAIN_STOP_FORCE) < 0) 
	{
		printf("domain %s poweroff:error\n", name);
		return -1;
	}
	return 0;
}

int libvirt_domain_reboot(char * name)
{
	if (domain_op(name, DOMAIN_REBOOT) < 0) 
	{
		printf("domain %s reboot:error\n", name);
		return -1;
	}
	return 0;
}

int libvirt_domain_suspend(char * name)
{
	if (domain_op(name, DOMAIN_SUSPEND) < 0) 
	{
		printf("domain %s suspend:error\n", name);
		return -1;
	}
	return 0;
}

int libvirt_domain_resume(char * name)
{
	if (domain_op(name, DOMAIN_RESUME) < 0) 
	{
		printf("domain %s resume:error\n", name);
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = -1;
	
	if(argc != 3)
	{	
		printf("usage: ./start_domain domain_name option\n");
		return -1;
	}
	
	if(strcmp(argv[2], "shutdown") == 0)
	{
		ret = libvirt_domain_stop(argv[1]);
	}
	else if(strcmp(argv[2], "destroy") == 0)
	{
		ret = libvirt_domain_poweroff(argv[1]);
	}
	else if(strcmp(argv[2], "reboot") == 0)
	{
		ret = libvirt_domain_reboot(argv[1]);
	}
	else if(strcmp(argv[2], "suspend") == 0)
	{
		ret = libvirt_domain_suspend(argv[1]);
	}
	else if(strcmp(argv[2], "resume") == 0)
	{
		ret = libvirt_domain_resume(argv[1]);
	}
	
	if(!ret)
	{
		printf("domain %s %s success\n", argv[1], argv[2]);
	}
	else
	{
		printf("domain %s %s fail\n", argv[1], argv[2]);
	}
	
	return ret;
	
}

/*======================================================================
    A test program to access /dev/second
    This example is to help understand async IO 
    
    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <asm-generic/ioctl.h>

#include "uake.h"

#define BASE_IP 2000000
#define COUNT  512

static int fd;

struct uake_map maps;

int main()
{
	int i, ret;
	unsigned int base_ip = BASE_IP;
	fd = open("/dev/uake", O_RDWR);
	if (fd != - 1)
	{
		printf("device open success\n");
		
		maps.count = COUNT;
		for (i=0; i<COUNT; i++) {
			maps.maps[i].ip_addr = base_ip + i;
			maps.maps[i].ip6_addr.s6_addr32[0] = base_ip + i;
			maps.maps[i].ip6_addr.s6_addr32[1] = base_ip + i + 1;
			maps.maps[i].ip6_addr.s6_addr32[2] = base_ip + i + 2;
			maps.maps[i].ip6_addr.s6_addr32[3] = base_ip + i + 3;
		}
		
		ret = ioctl(fd, UAKE_MAP_UPDATE, &maps);
		if (ret < 0)
		{
			perror("update ioctl error");
			close(fd);
			exit(1);
		}

		base_ip = BASE_IP;

		struct uake_entry  entry;	
		for (i=0; i<COUNT; i++) {
			memset(&entry, 0, sizeof(entry));
			entry.ip_addr = base_ip + i;
			ret = ioctl(fd, UAKE_MAP_GET, &entry);
			if (ret < 0)
			{
				perror("get ioctl error");
				close(fd);
				exit(1);
			}

			printf("get %d %x ioctl %x:%x:%x:%x\n", i, entry.ip_addr, entry.ip6_addr.s6_addr32[0], entry.ip6_addr.s6_addr32[1],
				entry.ip6_addr.s6_addr32[2], entry.ip6_addr.s6_addr32[3]);
		}

		for (i=0; i<COUNT; i++) {
			memset(&entry, 0, sizeof(entry));
			entry.ip6_addr.s6_addr32[0] = base_ip + i;
			entry.ip6_addr.s6_addr32[1] = base_ip + i + 1;
			entry.ip6_addr.s6_addr32[2] = base_ip + i + 2;
			entry.ip6_addr.s6_addr32[3] = base_ip + i + 3;
			ret = ioctl(fd, UAKE_MAP_GET, &entry);
			if (ret < 0)
			{
				perror("get ioctl error");
				close(fd);
				exit(1);
			}

			printf("get %d %x ioctl %x:%x:%x:%x\n", i, entry.ip_addr, entry.ip6_addr.s6_addr32[0], entry.ip6_addr.s6_addr32[1],
				entry.ip6_addr.s6_addr32[2], entry.ip6_addr.s6_addr32[3]);
		}

		memset(&entry, 0, sizeof(entry));
		entry.ip_addr = 1;
		ret = ioctl(fd, UAKE_MAP_GET, &entry);
		if (ret < 0)
		{
			perror("address 1 get ioctl error");
		}
		
	}
	else {
		printf("device open failure\n");
	}

	close(fd);

	return 0;
}

/*
 * sendmmsg microbenchmark.
 *
 * Build with:
 *
 * gcc -O2 -o rcvmmsg_test
 *
 * Copyright (C) 2011 Anton Blanchard <anton@au.ibm.com>, IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <asm/unistd.h>
#include <linux/socket.h>
#include <errno.h>
#include <pthread.h>

#define _GNU_SOURCE

#include <sched.h>

/*
./udp_recv_msg 0 2152 10
*/


static unsigned long packets;
static unsigned long packets_prev;
extern int flag;
unsigned long p = 0;


unsigned short get_local_port(int sock)
{	
	struct sockaddr addr;
    	int len = sizeof(addr);

    	if (getsockname(sock, &addr, (unsigned *)&len) != 0)
       		return 0;

   	 if (addr.sa_family != AF_INET)
       	 	return 0;
	
	return ntohs(((struct sockaddr_in *)(&addr))->sin_port);
}

unsigned int get_local_ip(int sd)
{
	struct sockaddr addr;
	int len = sizeof(addr);

	if (getsockname(sd, &addr, (unsigned *)&len) != 0)
	{
		return 0;
	}

	if (addr.sa_family != AF_INET)
	{
		return 0;
	}

	return ntohl(((struct sockaddr_in *)(&addr))->sin_addr.s_addr);
}



static void do_recvmsg(int fd, unsigned int packet_size)
{
	unsigned int i, j;
	unsigned char buf[packet_size];

	memset(buf, 0, sizeof(buf));

	int ret = 0;
	
	while(1)
	{
		ret = recvfrom(fd, &buf, packet_size, 0, NULL, NULL);
		if (ret < 0) 
		{
			perror("sendmmsg");
			exit(1);
		}
		packets++;	
		/*for(i=0; i<ret; i++)
			printf("buf[%d]:0x%x ", i, buf[i]);
		printf("\n");*/
		//if(packets%100 == 0)
		printf("num:%ld, ret: %d\n", packets, ret);
		printf("local ip: 0x%x, local port: %d\n", get_local_ip(fd), get_local_port(fd));		
	}

}


static void do_udp(const char *host, const char *port, int packet_size)
{
	int ret;
	int fd;
	int i;
	//char buf[packet_size];


	fd = socket(AF_INET, SOCK_DGRAM, 0/*IPPROTO_UDP*/);
	if (fd == -1) {
		perror("socket");
		exit(1);
	}

	struct sockaddr_in srvaddr;
	bzero(&srvaddr, sizeof(srvaddr));
   
    	srvaddr.sin_family = AF_INET;
	printf("port : %d\n", atoi(port));
    	srvaddr.sin_port = htons(atoi(port));
    	srvaddr.sin_addr.s_addr = inet_addr(host);
	printf("addr : 0x%x\n", srvaddr.sin_addr.s_addr);

	if ((bind(fd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr))) < 0)
	{
		perror("Bind raw socket failed:");
		return ;
	}

	int val = 1;                                                                                                   
	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)); 
	if(ret < 0)
	{
		perror("SO_REUSEADDR socket failed:");
		return ;
	}

	/*char *ip = get_local_ip(fd);
	if(ip != NULL)*/
	
	printf("local ip: 0x%x, local port: %d\n", get_local_ip(fd), get_local_port(fd));

	do_recvmsg(fd, packet_size);
}


static void sigalrm_handler(int junk)
{
	unsigned long p = packets;

	printf("%ld\n", p - packets_prev);
	packets_prev = p;
	alarm(1);
}

static void usage(void)
{
	fprintf(stderr, "Usage:  <host> <port> <packet_size> "
			"\n");
}

int main(int argc, char *argv[])
{
	const char *host;
	const char *port;
	const char *interface;
	int packet_size;
	int batch_size;
	int mmsg;

	if (argc != 4) 
	{
		usage();
		exit(1);
	}
	//alarm(1);

	host = argv[1];
	port = argv[2];
	packet_size = atoi(argv[3]);
	
	do_udp(host, port, packet_size);

	

	return 0;
}

/*
 * sendmmsg microbenchmark.
 *
 * Build with:
 *
 * gcc -O2 -o sendmmsg_test sendmmsg_test.c
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

static unsigned long packets;
static unsigned long packets_prev;
extern int flag;
/*
char * get_local_ip(int sd)
{
	struct sockaddr addr;
	int len = sizeof(addr);
	if (getsockname(sd, &addr, (unsigned *)&len) != 0)
	{
		return NULL;
	}

	if (addr.sa_family != AF_INET)
	{
		return NULL;
	}

	char * ip = inet_ntoa(((struct sockaddr_in *)(&addr))->sin_addr);
	if (ip == NULL)
	{
		return NULL;
	}

	return strdup(ip);
}
*/
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


static void do_sendmsg(int fd, struct sockaddr *addr, unsigned int packet_size,
			unsigned int batch_size, char *buf)
{
	int i;
	for(i=0; i< batch_size; i++)
	{	
		int n = sendto(fd, buf, packet_size, 0, addr, sizeof(struct sockaddr_in)) ;
		if( n < 0)
		{
		    perror("sendto()");
		    return ;
		}      
	}
	//printf("local ip: %s, local port: %d\n", get_local_ip(fd), get_local_port(fd));
	printf("send okay!\n");
}
static void do_udp(const char* local, const char *host, const char *port, int packet_size,
		   int batch_size)
{
	struct sockaddr_in server_addr;
	char buf[packet_size];
	memset(buf, 0, packet_size);

	int fd = socket(AF_INET, SOCK_DGRAM, 0);         
	if (fd == -1)
	{
		perror("socket()");
		return ;
	}

	struct sockaddr_in srvaddr;
        bzero(&srvaddr, sizeof(srvaddr));

        srvaddr.sin_family = AF_INET;
        srvaddr.sin_port = htons(1111);
        srvaddr.sin_addr.s_addr = inet_addr(local);
        printf("addr : 0x%x\n", srvaddr.sin_addr.s_addr);

        if ((bind(fd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr))) < 0)
        {
                perror("Bind raw socket failed:");
                return ;
        }

	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;                
	server_addr.sin_addr.s_addr = inet_addr(host);
	server_addr.sin_port = htons(atoi(port));

	//printf("local ip: %s, local port: %d\n", get_local_ip(fd), get_local_port(fd));

	do_sendmsg(fd, (struct sockaddr*)&server_addr, packet_size, batch_size, buf);
	
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
	fprintf(stderr, "Usage:  <local> <host> <port> <packet_size> "
			"<batch_size> \n");
}

int main(int argc, char *argv[])
{
	const char *host;
	const char *local;
	const char *port;
	const char *interface;
	int packet_size;
	int batch_size;
	int mmsg;

	if (argc != 6) 
	{
		usage();
		exit(1);
	}

	signal(SIGALRM, sigalrm_handler);
	alarm(1);
	
	local = argv[1];
	host = argv[2];
	port = argv[3];
	packet_size = atoi(argv[4]);
	batch_size = atoi(argv[5]);
	do_udp(local, host, port, packet_size, batch_size);

	return 0;
}

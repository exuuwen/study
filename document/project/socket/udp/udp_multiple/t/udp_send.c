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
struct info{
	int num;
	char id;
};


static unsigned long packets;
//static unsigned long packets_prev;




static void do_sendmsg(int fd, struct sockaddr *addr)
{
	int ret, i = 1;
	struct iovec iovec;
	struct msghdr datagrams;
	struct info  buf;
	
	memset(&buf, 0, sizeof(buf));
	memset(&iovec, 0, sizeof(iovec));
	memset(&datagrams, 0, sizeof(datagrams));

	
	
	iovec.iov_base = &buf;
	iovec.iov_len = sizeof(struct info);
	datagrams.msg_iov = &iovec;
	datagrams.msg_iovlen = 1;
	if (addr) {
		datagrams.msg_name = addr;
		datagrams.msg_namelen = sizeof(*addr);
	}
	
	
	while(1){
		
		if(i % 27 == 0)
			i = 1;
		buf.num = i;
		buf.id = i + 'a' - 1;
		i++;	
	
		ret = sendmsg(fd, &datagrams, 0);
		if (ret < 0) {
			perror("sendmmsg");
			exit(1);
		}
	
		sleep(1);
		
		packets ++;
	}	
	
}
static void do_udp(const char *host, const char *port)
{
	int ret;
	struct addrinfo *ainfo;
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_DGRAM,
		.ai_protocol = IPPROTO_UDP,
		.ai_flags = AI_PASSIVE,
	};
	int fd;
	//int i;
	//char buf[packet_size];

	ret = getaddrinfo(host, port, &hints, &ainfo);
	if (ret) {
		fprintf(stderr, "error using getaddrinfo: %s\n",
			gai_strerror(ret));
		exit(1);
	}

	fd = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
	if (fd == -1) {
		perror("socket");
		exit(1);
	}
	/*Shan!  you can change the buf across your own need*/
	/*for (i = 0; i < sizeof(buf); i++)
		buf[i]= i ;*/
	
	do_sendmsg(fd, ainfo->ai_addr);
	
}


static void sigalrm_handler(int junk)
{
	//unsigned long p = packets;

	printf("send %ld packege\n",  packets);
	//packets_prev = p;
	alarm(1);
}

static void usage(void)
{
	fprintf(stderr, "Usage: sendmmsg_test  <host> <port> \n");
}

int main(int argc, char *argv[])
{
	const char *host;
	const char *port;
	const char *buf;
	//int packet_size;
	//int batch_size;
	int mmsg;

	if (argc != 3) {
		usage();
		exit(1);
	}

	/*signal(SIGALRM, sigalrm_handler);
	alarm(1);*/

	
	host = argv[1];
	port = argv[2];
	//buf = argv[3];
	
	do_udp(host, port);


	return 0;
}

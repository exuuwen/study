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

#ifndef __NR_sendmmsg
#if defined( __PPC__)
#define __NR_sendmmsg	349
#elif defined(__x86_64__)
#define __NR_sendmmsg	307
#elif defined(__i386__)
#define __NR_sendmmsg	345
#else
#error __NR_sendmmsg not defined
#endif
#endif
typedef unsigned long long cycles_t;
cycles_t currentcycles() 
{
     cycles_t result;
     __asm__ __volatile__ ("rdtsc" : "=r" (result));
     return result;
}

/*
struct mmsghdr {
	struct msghdr msg_hdr;
	unsigned int msg_len;
};
*/
static inline int sendmmsg(int fd, struct mmsghdr *mmsg, unsigned vlen,
			   unsigned flags)
{
	return syscall(__NR_sendmmsg, fd, mmsg, vlen, flags, NULL);
}

static unsigned long packets;
static unsigned long packets_prev;
extern int flag;


static void do_sendmmsg(int fd, struct sockaddr *addr, unsigned int packet_size,
			unsigned int batch_size, char *b)
{
	unsigned int i;
	char buf[batch_size][packet_size];
	struct iovec iovec[batch_size][1];
	struct mmsghdr datagrams[batch_size];

	memset(buf, 0, sizeof(buf));
	memset(iovec, 0, sizeof(iovec));
	memset(datagrams, 0, sizeof(datagrams));

	for (i = 0; i < batch_size; ++i) {
		memcpy(&buf[i], b, sizeof(buf[i]));
		iovec[i][0].iov_base = buf[i];
		iovec[i][0].iov_len = sizeof(buf[i]);
		datagrams[i].msg_hdr.msg_iov = iovec[i];
		datagrams[i].msg_hdr.msg_iovlen = 1;
		if (addr) {
			datagrams[i].msg_hdr.msg_name = addr;
			datagrams[i].msg_hdr.msg_namelen = sizeof(*addr);
		}
	}

	int ret;
	cycles_t t_low, t_high;
	while (1) {

		t_low = currentcycles(); 

		ret = sendmmsg(fd, datagrams, batch_size, 0);
		if (ret < 0) {
			perror("sendmmsg");
			exit(1);
		}
		
		t_high = currentcycles();
		
		
		printf("the difference time is %lld\n",t_high - t_low);
		sleep(1);
		if (ret != batch_size) {
			fprintf(stderr, "sendmmsg returned sent less than batch\n");
		}

		packets += ret;
		
		
	}
}

static void do_sendmsg(int fd, struct sockaddr *addr, unsigned int packet_size,
			unsigned int batch_size, char *b)
{
	unsigned int i;
	char buf[batch_size][packet_size];
	struct iovec iovec[batch_size][1];
	struct msghdr datagrams[batch_size];

	memset(buf, 0, sizeof(buf));
	memset(iovec, 0, sizeof(iovec));
	memset(datagrams, 0, sizeof(datagrams));

	for (i = 0; i < batch_size; ++i) {
		memcpy(&buf[i], b, sizeof(buf[i]));
		iovec[i][0].iov_base = buf[i];
		iovec[i][0].iov_len = sizeof(buf[i]);
		datagrams[i].msg_iov = iovec[i];
		datagrams[i].msg_iovlen = 1;
		if (addr) {
			datagrams[i].msg_name = addr;
			datagrams[i].msg_namelen = sizeof(*addr);
		}
	}
	
	int ret;
	cycles_t t_low, t_high;
	while(1){

		t_low = currentcycles();

		for (i = 0; i < batch_size; ++i){	
			ret = sendmsg(fd, datagrams+i , 0);
			if (ret < 0) {
				perror("sendmmsg");
				exit(1);
			}
		}	

		t_high = currentcycles();

		
		printf("the difference time is %lld\n",t_high - t_low);
		//sleep(1);
		if (i != batch_size) {
			fprintf(stderr, "sendmmsg returned sent less than batch\n");
		}

		packets += i;
	}

	

		
	
}
static void do_udp(const char *host, const char *port, int packet_size,
		   int batch_size, int mmsg)
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
	int i;
	char buf[packet_size];

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

	for (i = 0; i < sizeof(buf); i++)
		buf[i]= i ;
	if(mmsg == 0)
		do_sendmsg(fd, ainfo->ai_addr, packet_size, batch_size, buf);
	else
		do_sendmmsg(fd, ainfo->ai_addr, packet_size, batch_size, buf);
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
	fprintf(stderr, "Usage: sendmmsg_test -u <host> <port> <packet_size> "
			"<batch_size> <mmsg>\n");
}

int main(int argc, char *argv[])
{
	const char *host;
	const char *port;
	const char *interface;
	int packet_size;
	int batch_size;
	int mmsg;

	if (argc != 7) {
		usage();
		exit(1);
	}

	signal(SIGALRM, sigalrm_handler);
	alarm(1);

	if (!strcmp(argv[1], "-u")) {
		host = argv[2];
		port = argv[3];
		packet_size = atoi(argv[4]);
		batch_size = atoi(argv[5]);
		mmsg = atoi(argv[6]);
		do_udp(host, port, packet_size, batch_size, mmsg);

	} else {
		usage();
		exit(1);
	}

	return 0;
}

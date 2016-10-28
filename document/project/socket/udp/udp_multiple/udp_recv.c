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



#ifndef __NR_recvmmsg
#if defined( __PPC__)
#define __NR_recvmmsg	337
#elif defined(__x86_64__)
#define __NR_sendmmsg	337
#elif defined(__i386__)
#define __NR_recvdmmsg	337
#else
#error __NR_recvmmsg not defined
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
static inline int recvvmmsg(int fd, struct mmsghdr *mmsg, unsigned vlen,
			   unsigned flags, struct timespec *tmo)
{
	return syscall(__NR_recvmmsg, fd, mmsg, vlen, flags, tmo);
}

static unsigned long packets;
static unsigned long packets_prev;
extern int flag;
unsigned long p = 0;

static void do_recvmmsg(int fd, struct sockaddr *addr, unsigned int packet_size,
			unsigned int batch_size)
{
	unsigned int i;
	char buf[batch_size][packet_size];
	struct iovec iovec[batch_size][1];
	struct mmsghdr datagrams[batch_size];
	memset(iovec, 0, sizeof(iovec));
	memset(datagrams, 0, sizeof(datagrams));

	for (i = 0; i < batch_size; ++i) {
		
		iovec[i][0].iov_base = buf[i];
		iovec[i][0].iov_len = sizeof(buf[i]);
		datagrams[i].msg_hdr.msg_iov = iovec[i];
		datagrams[i].msg_hdr.msg_iovlen = 1;
		datagrams[i].msg_len = batch_size;
		if (addr) {
			datagrams[i].msg_hdr.msg_name = addr;
			datagrams[i].msg_hdr.msg_namelen = sizeof(*addr);
		}
	}
	int ret;
	cycles_t t_low = 0, t_high = 0;
	while (1) {
		printf("flag:%d\n", flag);
		
		t_low = currentcycles(); 

		ret = recvmmsg(fd, datagrams, batch_size, 0, NULL);
		if (ret == 0) {
			perror("recvmmsg");
			exit(1);
		}
		
		t_high = currentcycles();
		
		printf("the difference time is %lld\n",t_high - t_low);
		
		if (ret < batch_size) {
			fprintf(stderr, "recv returned sent less than batch, ret:%d\n", ret);
		}
		t_high = t_low = 0;
		printf("%d\n", ret);
		packets += ret;
		
	}
}

static void do_recvmsg(int fd, struct sockaddr *addr, unsigned int packet_size,
			unsigned int batch_size)
{
	unsigned int i, j;
	char buf[batch_size][packet_size];
	struct iovec iovec[batch_size][1];
	struct msghdr datagrams[batch_size];

	memset(buf, 0, sizeof(buf));
	memset(iovec, 0, sizeof(iovec));
	memset(datagrams, 0, sizeof(datagrams));

	for (i = 0; i < batch_size; ++i) {
		
		iovec[i][0].iov_base = buf[i];
		iovec[i][0].iov_len = sizeof(buf[i]);
		datagrams[i].msg_iov = iovec[i];
		datagrams[i].msg_iovlen = 1;
		if (addr) {
			datagrams[i].msg_name = addr;
			datagrams[i].msg_namelen = sizeof(*addr);
		}
	}

	int ret = 0;
	
	cycles_t t_low, t_high;
	while(1){

		t_low = currentcycles();
		for (i = 0; i < batch_size; ++i){	
			
			ret = recvmsg(fd, datagrams+i, 0);
			if (ret < 0) {
				perror("sendmmsg");
				exit(1);
			}
		
		}	
		t_high = currentcycles();
		
		
		
		printf("the difference time is %lld\n",t_high - t_low);

		if (i != batch_size) {
			fprintf(stderr, "sendmmsg returned sent less than batch\n");
		}

		 packets += i;
		

		printf("%d\n", i);
		
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
	bind(fd, ainfo->ai_addr, sizeof(struct sockaddr));
	/*for (i = 0; i < sizeof(buf); i++)
		buf[i]= i;*/
	if(mmsg == 0)
		do_recvmsg(fd, NULL, packet_size, batch_size);
	else
		do_recvmmsg(fd, NULL, packet_size, batch_size);
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
	int num = sysconf(_SC_NPROCESSORS_CONF);
	int cpu = 0 % num;
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);
	if (sched_setaffinity(getpid(), sizeof(mask), &mask) != 0)
	{
		printf("set affinity for pid:%d to cpu:%d successfully.\n", getpid(), cpu);
		return;
	}

	printf("set affinity for pid:%d to cpu:%d successfully.\n", getpid(), cpu);
	/*signal(SIGALRM, sigalrm_handler);
	alarm(1);*/

	if (!strcmp(argv[1], "-u")) {
		host = argv[2];
		port = argv[3];
		packet_size = atoi(argv[4]);
		batch_size = atoi(argv[5]);
		mmsg = atoi(argv[6]);
		printf("msg is %d\n", mmsg);
		do_udp(host, port, packet_size, batch_size, mmsg);

	} else {
		usage();
		exit(1);
	}

	return 0;
}

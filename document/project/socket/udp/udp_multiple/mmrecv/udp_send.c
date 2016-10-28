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
#include<pthread.h>

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
static int times;
struct addrinfo *ainfo;
int fds, fdr;
pthread_t stid, rtid;

cycles_t currentcycles() 
{
     cycles_t result;
     __asm__ __volatile__ ("rdtsc" : "=r" (result));
     return result;
}
pthread_mutex_t mutex;
static inline int sendmmsg(int fd, struct mmsghdr *mmsg, unsigned vlen,
			   unsigned flags)
{
	return syscall(__NR_sendmmsg, fd, mmsg, vlen, flags, NULL);
}

static unsigned long packets;
static unsigned long packets_prev;
#define P_LEN 512
#define SEND_P 15240
static void *do_sendmmsg(void *arg)
{
	unsigned int i;
	char buf[SEND_P][P_LEN];
	struct iovec iovec[SEND_P][1];
	struct mmsghdr datagrams[SEND_P];

	memset(buf, 0, sizeof(buf));
	memset(iovec, 0, sizeof(iovec));
	memset(datagrams, 0, sizeof(datagrams));

	for (i = 0; i < SEND_P; ++i) {
		//memcpy(&buf[i], b, sizeof(buf[i]));
		iovec[i][0].iov_base = buf[i];
		iovec[i][0].iov_len = sizeof(buf[i]);
		datagrams[i].msg_hdr.msg_iov = iovec[i];
		datagrams[i].msg_hdr.msg_iovlen = 1;
			datagrams[i].msg_hdr.msg_name = ainfo->ai_addr;
			datagrams[i].msg_hdr.msg_namelen = sizeof(*(ainfo->ai_addr));
		
	}

	int ret;
	cycles_t t_low, t_high;
	while (1) 
	{
		
		ret = sendmmsg(fds, datagrams, SEND_P, 0);
		if (ret < 0) {
			perror("sendmmsg");
			exit(1);
		}
		
		
		//pthread_mutex_unlock(&mutex);
		//printf("the difference time is %lld\n",t_high - t_low);
		sleep(1);
		if (ret != SEND_P) {
			fprintf(stderr, "sendmmsg returned sent less than batch\n");
		}
		
		packets += ret;
		
		
	}
}

static void* do_sendmsg(void *arg)
{
	unsigned int i = 0;
	char buf[SEND_P][P_LEN];
	struct iovec iovec[SEND_P][1];
	struct msghdr datagrams[SEND_P];

	memset(buf, 0, sizeof(buf));
	memset(iovec, 0, sizeof(iovec));
	memset(datagrams, 0, sizeof(datagrams));

	//for (i = 0; i < SEND_P; ++i) 
	{
		//memcpy(&buf[i], b, sizeof(buf[i]));
		iovec[i][0].iov_base = buf[i];
		iovec[i][0].iov_len = sizeof(buf[i]);
		datagrams[i].msg_iov = iovec[i];
		datagrams[i].msg_iovlen = 1;
		
			datagrams[i].msg_name = ainfo->ai_addr;
			datagrams[i].msg_namelen = sizeof(*(ainfo->ai_addr));
		
	}
	
	int ret;
	//cycles_t t_low, t_high;
	//while(1)
	{	
		for (i = 0; i < 5120; ++i){	
			ret = sendmsg(fds, datagrams , 0);
			if (ret < 0) {
				perror("sendmmsg");
				exit(1);
			}
		}	
	
		//sleep(1);
		if (i != 5120) {
			fprintf(stderr, "sendmmsg returned sent less than batch\n");
		}

		packets += i;
	}
	
}

#define REC_P 512
static void* do_recvmmsg(void *arg)
{
	unsigned int i;
	char buf[REC_P][P_LEN];
	struct iovec iovec[REC_P][1];
	struct mmsghdr datagrams[REC_P];
	memset(iovec, 0, sizeof(iovec));
	memset(datagrams, 0, sizeof(datagrams));

	//sleep(3);
	for (i = 0; i < REC_P; ++i) {
		
		iovec[i][0].iov_base = buf[i];
		iovec[i][0].iov_len = sizeof(buf[i]);
		datagrams[i].msg_hdr.msg_iov = iovec[i];
		datagrams[i].msg_hdr.msg_iovlen = 1;
		datagrams[i].msg_len = REC_P;
		/*if (addr) {
			datagrams[i].msg_hdr.msg_name = addr;
			datagrams[i].msg_hdr.msg_namelen = sizeof(*addr);
		}*/
	}
	int ret;
	cycles_t t_low = 0, t_high = 0;
	while (1) 
	{
		times++;
		t_low = currentcycles(); 

		ret = recvmmsg(fdr, datagrams, REC_P, 0, NULL);
		if (ret == 0) {
			perror("recvmmsg");
			exit(1);
		}
		
		t_high = currentcycles();
		//if(times%50 == 0)
		{
			printf("the difference time is %lld\n",t_high - t_low);
			printf("%d\n", ret);
		}
		
		if (ret != REC_P) {
			fprintf(stderr, "recv returned sent less than batch, ret:%d\n", ret);
		}
		
		//printf("%d\n", ret);
		//packets += ret;
		
	}
}

static void* do_recvmsg(void *arg)
{
	unsigned int i, j;
	char buf[REC_P][P_LEN];
	struct iovec iovec[REC_P][1];
	struct msghdr datagrams[REC_P];

	memset(buf, 0, sizeof(buf));
	memset(iovec, 0, sizeof(iovec));
	memset(datagrams, 0, sizeof(datagrams));
	
	//sleep(3);
	for (i = 0; i < REC_P; ++i) {
		
		iovec[i][0].iov_base = buf[i];
		iovec[i][0].iov_len = sizeof(buf[i]);
		datagrams[i].msg_iov = iovec[i];
		datagrams[i].msg_iovlen = 1;
		/*if (addr) {
			datagrams[i].msg_name = addr;
			datagrams[i].msg_namelen = sizeof(*addr);
		}*/
	}

	int ret = 0;
	
	cycles_t t_low = 0, t_high = 0;
	while(1)
	{	
		times++;
		t_low = currentcycles();
		for (i = 0; i < REC_P; ++i){	
			
			ret = recvmsg(fdr, datagrams+i, 0);
			if (ret < 0) {
				perror("sendmmsg");
				exit(1);
			}
		
		}	
		t_high = currentcycles();
		
		
		//if(times%50 == 0)
		{
			printf("the difference time is %lld\n", t_high - t_low);
			printf("%d\n", i);
		}

		if (i != REC_P) {
			fprintf(stderr, "sendmmsg returned sent less than batch\n");
		}

		 //packets += i;	
		
	}	
	
}
static void do_udp(const char *host, const char *port, int smmsg, int rmmsg)
{
	int ret, err;
	
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_DGRAM,
		.ai_protocol = IPPROTO_UDP,
		.ai_flags = AI_PASSIVE,
	};
	
	
	//char buf[packet_size];

	ret = getaddrinfo(host, port, &hints, &ainfo);
	if (ret) {
		fprintf(stderr, "error using getaddrinfo: %s\n",
			gai_strerror(ret));
		exit(1);
	}

	fds = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
	if (fds == -1) {
		perror("socket");
		exit(1);
	}
	
	fdr = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
	if (fdr == -1) {
		perror("socket");
		exit(1);
	}
	bind(fdr, ainfo->ai_addr, sizeof(struct sockaddr));

	
	
	
	if(smmsg == 0){
		err = pthread_create(&stid,NULL,do_sendmsg,NULL);
		if(err != 0){
			printf("can't create thread: %s\n",strerror(err));
			exit(1) ;
		}
	}
	else{
		err = pthread_create(&stid,NULL,do_sendmmsg,NULL);
		if(err != 0){
			printf("can't create thread: %s\n",strerror(err));
			exit(1);
		}
	}
	//sleep(1);
	if(rmmsg == 0){
		err = pthread_create(&rtid,NULL,do_recvmsg,NULL);
		if(err != 0){
			printf("can't create thread: %s\n",strerror(err));
			exit(1);
		}
	}
	else{
		err = pthread_create(&rtid,NULL,do_recvmmsg,NULL);
		if(err != 0){
			printf("can't create thread: %s\n",strerror(err));
			exit(1);
		}
	}
	
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
	fprintf(stderr, "Usage: sendmmsg_test -u <host> <port>  "
			" <smmsg> <rmmsg>\n");
}

int main(int argc, char *argv[])
{
	const char *host;
	const char *port;
	const char *interface;
	
	int smmsg;
	int rmmsg;
	
	if (argc != 6) {
		usage();
		exit(1);
	}

	/*signal(SIGALRM, sigalrm_handler);
	alarm(1);*/
	pthread_mutex_init(&mutex, NULL);
	if (!strcmp(argv[1], "-u")) {
		host = argv[2];
		port = argv[3];
		smmsg = atoi(argv[4]);
		rmmsg = atoi(argv[5]);
		do_udp(host, port, smmsg, rmmsg);

	} else {
		usage();
		exit(1);
	}
	pthread_join(stid, NULL);
	pthread_join(rtid, NULL);
	return 0;
}

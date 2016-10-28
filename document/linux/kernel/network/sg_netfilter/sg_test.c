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


static void do_udp(const char *host, const char *port)
{
	struct sockaddr_in server_addr;
	struct iovec iov[2];
	ssize_t nwritten;

	struct msghdr msg;
	char str0[3000] = "hello";
	char str1[2000] = "world";

	
	int fd = socket(AF_INET, SOCK_DGRAM, 0);         
	if (fd == -1)
	{
		perror("socket()");
		return ;
	}
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;                
	server_addr.sin_addr.s_addr = inet_addr(host);
	server_addr.sin_port = htons(atoi(port));

	msg.msg_name = (void*)&server_addr;
	msg.msg_namelen = sizeof(server_addr);

	msg.msg_control = NULL;
        msg.msg_controllen = 0;

	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	
	iov[0].iov_base = str0;
	iov[0].iov_len = 1500;
	iov[1].iov_base = str1;
	iov[1].iov_len = 10;

	
	nwritten = sendmsg(fd, &msg, MSG_MORE);
	if (nwritten == -1)
	{
		perror("writev fail:");
	}
	printf("1 wirten:%ld\n", nwritten);
	
	iov[0].iov_base = str0;
	iov[0].iov_len = 220;
	iov[1].iov_base = str1;
	iov[1].iov_len = 10;
	
	
	nwritten = sendmsg(fd, &msg, MSG_MORE);
	if (nwritten == -1)
	{
		perror("writev fail:");
	}

	printf("2 wirten:%ld\n", nwritten);

	/*
	iov[0].iov_base = str0;
	iov[0].iov_len = 150;
	iov[1].iov_base = str1;
	iov[1].iov_len = 10;

	nwritten = sendmsg(fd, &msg, MSG_MORE);
	if (nwritten == -1)
	{
		perror("writev fail:");
	}

	printf("3 wirten:%ld\n", nwritten);*/
	

	iov[0].iov_base = str0;
	iov[0].iov_len = 300;
	iov[1].iov_base = str1;
	iov[1].iov_len = 1000;

	nwritten = sendmsg(fd, &msg, 0);
	if (nwritten == -1)
	{
		perror("writev fail:");
	}

	printf("finnal wirten:%ld\n", nwritten);
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
	fprintf(stderr, "Usage:  <host> <port> \n");
}

int main(int argc, char *argv[])
{
	const char *host;
	const char *port;
	const char *interface;
	int packet_size;
	int batch_size;
	int mmsg;

	if (argc != 3) 
	{
		usage();
		exit(1);
	}

	signal(SIGALRM, sigalrm_handler);
	alarm(1);
	
	host = argv[1];
	port = argv[2];

	do_udp(host, port);

	return 0;
}


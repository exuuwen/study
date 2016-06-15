#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("./client broadcast_addr port\n");
		return -1;
	}

	struct sockaddr_in broadcast_addr;

	broadcast_addr.sin_family = AF_INET;
	broadcast_addr.sin_port = htons(atoi(argv[2]));
	if (inet_aton(argv[1], &broadcast_addr.sin_addr) == 0)
	{
		printf("fail ip addr\n");
		return -1;
	}

	int sender_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 > sender_sock) {
		printf("failed to create sync receiver. err = %d(%s).", errno,
				strerror(errno));
		return -1;
	}

	int on = 1;
	if (setsockopt(sender_sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))) {
		printf("failed to enable sync receiver broadcast. err = %d(%s).",
				errno, strerror(errno));
		return -1;
	}

	if (setsockopt(sender_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
		printf("failed to set sync receiver reuse. err = %d(%s).", errno,
				strerror(errno));
		return -1;
	}

	unsigned char buf[100];
 
    memset(buf, 0, sizeof(buf));
 
    int ret = 0;
	unsigned int packets = 0;
     
    while(1)
    {
		sprintf(buf, "hahahahahahha %u", packets);
		int n = sendto(sender_sock, buf, strlen(buf), 0, (struct sockaddr *)&broadcast_addr, sizeof(struct sockaddr_in)) ;
        if( n < 0)
        {
            perror("sendto()");
            return ;
        }   
        printf("%s\n", buf); 
		packets++;
		sleep(1);		
    }
}

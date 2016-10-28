#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("./server broadcast_addr port\n");
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

	int receiver_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 > receiver_sock) {
		printf("failed to create sync receiver. err = %d(%s).", errno,
				strerror(errno));
		return -1;
	}

	int on = 1;
	if (setsockopt(receiver_sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on))) {
		printf("failed to enable sync receiver broadcast. err = %d(%s).",
				errno, strerror(errno));
		return -1;
	}

	if (bind(receiver_sock, (struct sockaddr *) &broadcast_addr,
			sizeof(broadcast_addr))) {
		printf("failed to bind sync receiver to broadcast address %s:%d. err = %d(%s).",
				inet_ntoa(broadcast_addr.sin_addr),
				ntohs(broadcast_addr.sin_port), errno, strerror(errno));
		return -1;
	}

	unsigned char buf[100];
    memset(buf, 0, sizeof(buf));
 
    int ret = 0;
     
    while(1)
    {
        ret = recvfrom(receiver_sock, buf, sizeof(buf) - 1, 0, NULL, NULL);
        if (ret < 0) 
        {
            perror("recvmsg");
            return -1;
        }

        printf("%s\n", buf);
    	memset(buf, 0, sizeof(buf));
    }
}

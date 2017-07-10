#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
void test( int sd, struct sockaddr *addr, socklen_t len)
{
        char buf[4];
        connect(sd, (struct sockaddr *)addr, len);
        sendto(sd, buf, 4, 0, (struct sockaddr *)addr, len);
         perror("write");
         sendto(sd, buf, 4, 0, (struct sockaddr *)addr, len);
         perror("write");
         recvfrom(sd, buf, 4, 0, (struct sockaddr *)addr, &len);
         perror("read");
}
int main(int argc, char **argv)
{
        int sd;
        struct sockaddr_in addr;
        if(argc != 2) {
		printf("./udp_refused dst_ip\n");
                return -1;
        }
        bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(12345);
        inet_pton(AF_INET, argv[1], &addr.sin_addr);
        sd = socket(AF_INET, SOCK_DGRAM, 0);
        test(sd, (struct sockaddr *)&addr, sizeof(addr));
        return 0;
}

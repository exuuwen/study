#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#define MAXLINE  20

int main(int  argc, char* argv[])
{
	int server_socketfd, client_socketfd;
	int server_len, client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int i, byte, n;
	char rec_char[MAXLINE];
	
	server_socketfd=socket(AF_INET, SOCK_DGRAM, 0);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(1234);	
	server_len=sizeof(server_address);
	
	bind(server_socketfd, (struct sockaddr *)&server_address, server_len);	

	
	int addr_len = sizeof(client_address);

	printf("now recive the  data  from  client\n");
	while(1)
	{
		if((n = recvfrom(server_socketfd, rec_char, MAXLINE, 0, (struct sockaddr *)&client_address, &addr_len)) == -1)
		{
			perror("recv bad");
			exit(1);
		}
		else 
		{
			if(n == 0)
			{
				printf("server bye\n");
				close(server_socketfd);
				exit(0);
			}
			rec_char[n] = 0;
			printf("receive the data from client is %s", rec_char);
		}		
		sleep(1);
		if((byte = sendto(server_socketfd, rec_char, n, 0, (struct sockaddr *)&client_address, addr_len)) == -1)
		{
			perror("send bad");
			exit(1);
		}
	}

	close(server_socketfd);
}


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#define MAXLINE  20

int main(int  argc, char* argv[])
{
	int socketfd,res;
	int server_len,client_len,c_len,s_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	struct sockaddr_in c_address;
	struct sockaddr_in s_address;
	int i, byte, n, ret;
	char send_char[MAXLINE], rec_char[MAXLINE];
	
	socketfd=socket(AF_INET, SOCK_DGRAM,0);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("192.168.60.235");
	server_address.sin_port = htons(1234);	
	server_len = sizeof(server_address);

	//always ok for the first connect, no matter the server start.
	connect(socketfd, (struct sockaddr *)&server_address, server_len);
	
	//after connect we can get our address and peer address
	c_len = sizeof(c_address);
	getsockname(socketfd, (struct sockaddr *)&c_address, &c_len);
	printf("the client ip is %s \n", inet_ntoa(c_address.sin_addr));
	printf("the client port is %d \n", ntohs(c_address.sin_port));
	
	s_len = sizeof(s_address);
	getpeername(socketfd, (struct sockaddr *)&s_address, &s_len);
	printf("the host ip is %s \n", inet_ntoa(s_address.sin_addr));
	printf("the host port is %d \n", ntohs(s_address.sin_port));

	while(1)
	{
		printf("now input the data:");
		if(fgets(send_char, MAXLINE, stdin) == NULL)
		{	
			printf("client bye\n");
			write(socketfd, send_char, 0);
			close(socketfd);
			exit(0);
		}
		n = strlen(send_char);
		if((byte = write(socketfd, send_char, n)) == -1)
		{
			perror("send bad");
			exit(1);
		}	
		
		if((n = read(socketfd, rec_char, byte)) == -1)
		{
			perror("recv bad");
			exit(1);
		}
		else 
		{
			rec_char[n] = 0;
			printf("receive the data from client: %s", rec_char);
		}	
	}
	close(socketfd);
	return 0;
	
}


#include <stdio.h>
#include <sys/prctl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sched.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UDP_PORT 8888   
#define UDP_DATA "UDP TEST DATA"                           
#define BUFF_SIZE 256 
int main(int argc, char* argv[])
{
	int s, n;
	struct sockaddr_in server_addr;
	struct in_addr localInterface;
     
	if(argc != 2)
	{
		printf("%s dst_ip\n", argv[0]);
		return 0;
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);         
	if (s == -1)
	{
		perror("socket()");
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;                
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(UDP_PORT);
	
	
	char buff[BUFF_SIZE];
	while(1) 
	{
		n = sendto(s, UDP_DATA, sizeof(UDP_DATA), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) ;
		if( n < 0)
		{
		    perror("sendto()");
		    return -2;
		}      
	
		memset(buff, 0, BUFF_SIZE);  
		n = recvfrom(s, buff, BUFF_SIZE, 0, NULL, NULL);
		if( n== -1)
		{
	   		perror("recvfrom()");
		}
		                                    
		printf("Recv  message from server:%s\n", buff);
		sleep(2);                          
	}

	return 0;
}

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

#define UDP_PORT 1234                        
#define BUFF_SIZE 256                           
int main(int argc, char*argv[])
{  
	int s;                                      
	struct sockaddr_in local_addr;  
	struct sockaddr_in peer_addr;            
	int err = -1;

	s = socket(AF_INET, SOCK_DGRAM, 0);     
	if (s == -1)
	{
		perror("socket()");
		return -1;
	}  

                                        
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(UDP_PORT);

                                       
	err = bind(s,(struct sockaddr*)&local_addr, sizeof(local_addr)) ;
	if(err < 0)
	{
		perror("bind()");
		return -2;
	}

	int i;
	int times = 0;
	int addr_len = 0;
	char buff[BUFF_SIZE];
	int n = 0;
		                        
	while(1)
	{
		addr_len = sizeof(local_addr);
		memset(buff, 0, BUFF_SIZE);                 
		                                  
		n = recvfrom(s, buff, BUFF_SIZE, 0, (struct sockaddr*)&peer_addr, &addr_len);
		if( n== -1)
		{
	   		perror("recvfrom()");
		}
		                                    
		printf("Recv %dst message from server:%s\n", times, buff);
		sleep(2); 
		n = sendto(s, buff, BUFF_SIZE, 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) ;
		if( n < 0)
		{
		    perror("sendto()");
		    return -2;
		}      	
	}


	close(s);
	return 0;
}

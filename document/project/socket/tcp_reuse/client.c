#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <net/ethernet.h>
#include <poll.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int set_reuse_addr(int sockfd, int on)
{
        int optval = on ? 1 : 0;

        int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
        return ret;
}

   
int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr;
	struct sockaddr_in local_addr;

	if(argc != 2)
	{
		printf("%s dst_ip\n", argv[0]);
		return 0;
	}

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);                                                                        
	if (sock < 0)                                                                                                 
	{                                                                                                              
		printf("StreamSocket Create socket failed\n");                                                           
		return -1;                                                                                               
	}    
	
	int ret;
	ret = set_reuse_addr(sock, 1);
        if (ret < 0)
	{
		perror("set resue addr");
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);;
	server_addr.sin_port = htons(2152);  

	
	struct sockaddr_in addr;                                                                                       
	socklen_t addr_len;                                                                                            
			                                                                                           
	addr_len = sizeof(addr);                                                                                       
	ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));                                                    
	if (ret == -1)                                                                                                  
	{          
		perror("connect server fail:");   
		close(sock);                                                             
		return -1;                                                                                               
	}       
	
	printf("connect success\n");
	char buf[1024] = "shuwhude";
	int len = 6;//sizeof(buf);
	
	while(1)
	{
		ret = write(sock, (char *)buf, len);      
		if (ret < 0)                                                                                           
		{                                                                                                          
			printf("write() failed. len = %d, ret = %d, errno = %d:%s\n", len, ret, errno, strerror(errno)); 
			return -1;                                                   
		} 
		
		memset(buf, 0, sizeof(buf));

		ret = read(sock, (char *)buf, sizeof(buf));      
		if (ret < 0)                                                                                           
		{                                                                                                          
		    	printf("read() failed. len = %d, ret = %d, errno = %d:%s\n", len, ret, errno, strerror(errno)); 
		    	return -1;                                                   
		} 
		else if(ret == 0)
		{
			printf("server bye\n");
			close(sock);
			return 0;
		}
		else
		{
			sleep(1);
			printf("recieve back buf: %s\n", buf);     
		}

		//close(sock);
	}

}

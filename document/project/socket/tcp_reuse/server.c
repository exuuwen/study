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

int set_reuse_port(int sockfd, int on)
{
	int optval = on ? 1 : 0;

	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
  	return ret;		
}

int set_bind_ifindex(int sockfd, int index)
{
	int ret = setsockopt(sockfd, SOL_SOCKET, SO_BINDTOIFINDEX, &index, sizeof index);
  	return ret;		
}
   
int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr;

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);                                                                        
	if (sock < 0)                                                                                                 
	{                                                                                                              
		printf("StreamSocket Create socket failed\n");                                                           
		return -1;                                                                                               
	}    

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(2152);

        int err = set_reuse_addr(sock, 1);
        if(err < 0)
	{
		perror("reuse port");
		return -1;
	}

        err = set_reuse_port(sock, 1);
        if(err < 0)
	{
		perror("reuse port");
		return -1;
	}

	if (argc == 2) {
        	int err = set_bind_ifindex(sock, atoi(argv[1]));
        	if(err < 0)
		{
			perror("reuse port");
			return -1;
		}
		printf("bind success to %s\n", argv[1]);
	}
  
	err = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) ;  
	if(err < 0)
	{
		perror("bind()");
		return -1;
	}     

	if (listen(sock, 5) < 0)                                                                                 
	{                                                                                                              
		printf("listen server failed\n");  
		close(sock);                                                           
		return -1;                                                                                               
	}    

	int fd;                                                                                                        
	struct sockaddr_in addr;                                                                                       
	socklen_t addr_len;                                                                                            
			                                                                                           
	addr_len = sizeof(addr);                                                                                       
	fd = accept(sock, (struct sockaddr *) &addr, &addr_len);                                                       
	if (fd == -1)                                                                                                  
	{          
		printf("accept server fail\n");                                                                
		return -1;                                                                                               
	}       
	
	printf("accept success\n");

	char buf[1024];
	char sbuf[1024] = "shuwhude";
	int len = sizeof(buf);
	int ret;
	while(1)
	{
		ret = read(fd, (char *)buf, len);      
		if (ret < 0)                                                                                           
		{                                                                                                          
		    printf("read() failed. len = %d, ret = %d, errno = %d:%s\n", len, ret, errno, strerror(errno)); 
		    return -1;                                                   
		}
		else if(ret == 0)
		{
			printf("client bye\n");
			close(fd);
			return 0;
		}
		else
		{
			buf[ret] = '\0';
			printf("recieve buf:%s\n", buf);
		
			sleep(2);
			ret = write(fd, (char*)sbuf, 6);      
			if (ret < 0)                                                                                           
			{                                                                                                          
			    printf("write() failed. len = %d, ret = %d, errno = %d:%s\n", len, ret, errno, strerror(errno)); 
			    return -1;                                                   
			} 

			printf("write %d\n", ret);
		
		
			memset(buf, 0, sizeof(buf));
		}
	}

}

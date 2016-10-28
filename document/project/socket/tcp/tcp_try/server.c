#define _GNU_SOURCE
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
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
/*buffer max: /proc/sys/net/core/wmem_max*/

int SocketReadable(int sockfd, int tmo_ms)
{
    struct pollfd socketToPoll[1];
    socketToPoll[0].fd = sockfd;
    socketToPoll[0].events = POLLIN|POLLPRI;
    socketToPoll[0].revents = 0;

    return (poll(socketToPoll, 1, tmo_ms) > 0) && (socketToPoll[0].revents & (POLLIN|POLLPRI));
}

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

unsigned int get_local_ip(int sd)
{
	struct sockaddr addr;
	int len = sizeof(addr);

	if (getsockname(sd, &addr, (unsigned *)&len) != 0)
	{
		return 0;
	}

	if (addr.sa_family != AF_INET)
	{
		return 0;
	}

	return ntohl(((struct sockaddr_in *)(&addr))->sin_addr.s_addr);
}


int SetKeepalive(int sd, int time, int intvl, int probes)
{
    int start = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &start, sizeof(int)) < 0)
        return -1;
    if (setsockopt(sd, SOL_TCP, TCP_KEEPIDLE, &time, sizeof(int)) < 0)
        return -1;
    if (setsockopt(sd, SOL_TCP, TCP_KEEPINTVL, &intvl, sizeof(int)) < 0)
        return -1;
    if (setsockopt(sd, SOL_TCP, TCP_KEEPCNT, &probes, sizeof(int)) < 0)
        return -1;
    return 0;
}

int IsHalfClose(int sockfd)
{
	struct sockaddr_in peeraddr;
	bzero(&peeraddr, sizeof peeraddr);
	socklen_t addrlen = sizeof(peeraddr);

	if (getpeername(sockfd, &peeraddr, &addrlen) < 0 && errno == ENOTCONN)
		return 1;
	else
		return 0;
}

   
unsigned int GetSendBufferSize(int sock)                                                                               
{                                                                                                                  
	size_t opt = 0;                                                                                                
	socklen_t len = sizeof(opt); 
                                                                                                           
	getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &opt, &len);   				   	
	return opt;                                                                                                                                                                                                   
}  

void sighandler(int sig) 
{
        printf("SIGpipe:I am \n");
}


int SetSendBufferSize(int sock, unsigned int size)                                                                      
{                                                                                                                  
	size_t opt = size;    
	int ret = -1;                                                      
	                                                                                                             
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt));                                                
	                                                                                                             
	return ret;                                                                                                            
}  

int SetReuseAddr(int sockfd, int on)
{
	int optval = on ? 1 : 0;

	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
  	return ret;		
}

int SetSoLinger(int sockfd, int on, int time)
{
	struct linger so_linger;

	so_linger.l_onoff = on;
	so_linger.l_linger = time;

	int ret = setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));

	return ret;
}


int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr;

	if(argc != 3)
	{
		printf("%s server_ip server_port\n", argv[0]);
		return 0;
	}

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);                                                                        
	if (sock < 0)                                                                                                 
	{                                                                                                              
		printf("StreamSocket Create socket failed\n");                                                           
		return -1;                                                                                               
	}    

	int flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0)                                                                                                  
	{          
		printf("F_GETFL fail\n");                                                                
		return -1;                                                                                               
	} 
    	flags |= O_NONBLOCK;
    	if (fcntl(sock, F_SETFL, flags) < 0)
    	{
        	perror(	"sockets::SetNonBlockAndCloseOnExec, non_blocking");
		return -1;
    	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);;
	server_addr.sin_port = htons(atoi(argv[2]));
	
	int ret;
        ret = SetReuseAddr(sock, 1);
	if(ret < 0)
	{
		perror("SetReuseAddr");
		return -1;
	}  
	printf("ip:0x%x, port:%d\n", get_local_ip(sock), get_local_port(sock));                     
	ret = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) ;  
	if(ret < 0)
	{
		perror("bind()");
		return -1;
	}     
	
	printf("ip:0x%x, port:%d\n", get_local_ip(sock), get_local_port(sock));

	if (listen(sock, 5) < 0)                                                                                 
	{                                                                                                              
		printf("listen server failed\n");  
		close(sock);                                                           
		return -1;                                                                                               
	}    
	printf("ip:0x%x, port:%d\n", get_local_ip(sock), get_local_port(sock));
	unsigned int size = GetSendBufferSize(sock);
	printf("buffer size:%d\n", size);  

	ret = SetSendBufferSize(sock, 1024*10000);
	if(ret < 0)
	{
		perror("setbuf fail:");
		return -1;
	}

	size = GetSendBufferSize(sock);
	printf("2 buffer size:%d\n", size); 
	int fd;                                                                                                        
	struct sockaddr_in addr;                                                                                       
	socklen_t addr_len;                                                                                            
			            
	ret = SocketReadable(sock, -1); 
	if(ret != 1)
	{
		printf("accept1 server fail\n");                                                                
		return -1; 
	}                 
                                                        
	addr_len = sizeof(addr);                                                                                       
	fd = accept4(sock, (struct sockaddr *) &addr, &addr_len, O_NONBLOCK);                                                       
	if (fd == -1)                                                                                                  
	{          
		printf("accept server fail\n");                                                                
		return -1;                                                                                               
	}       
	printf("ip:0x%x, port:%d\n", get_local_ip(fd), get_local_port(fd)); 
	printf("ip:0x%x, port:%d\n", get_local_ip(sock), get_local_port(sock)); 
	printf("accept4 success\n");

	ret = SetKeepalive(fd, 20, 5, 2);
	if (ret < -1)                                                                                                  
	{          
		printf("accept server fail\n");                                                                
		return -1;                                                                                               
	} 

	ret = SetSoLinger(fd, 1, 1);  // make close my type
	if (ret < 0)                                                                                                  
	{          
		printf("accept server fail\n");                                                                
		return -1;                                                                                               
	} 

	char buf[1024];
	char sbuf[1024] = "shuwhude";
	int len = sizeof(buf);
	
	
	struct pollfd fds;
	fds.fd = fd;
	fds.events = POLLRDHUP | POLLIN;

	if(IsHalfClose(fd))
	{
		printf("it is a half conection\n");
		return -1;
	}
	
	size = GetSendBufferSize(fd);
	printf("buffer size:%d\n", size);  

	signal(SIGPIPE, sighandler);

	int i;
	//while(1)
	//for(i=0; i<5; i++)
	{
		ret = poll(&fds, 1, -1);
		if(ret < 0)
		{
			perror("poll fail");
			return -1;
		}
		if((fds.revents & POLLRDHUP) == POLLRDHUP)
		{
			printf("events POLLRDHUP\n");
		}
		else if((fds.revents & POLLIN) == POLLIN)
		{
			printf("data\n");
		}

		ret = read(fd, (char*)buf, 1024);      
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
			printf("%d recieve buf:%s\n", ret, buf);
			//sleep(1);
			ret = write(fd, (char*)buf, 1024); 
		     
			if (ret < 0)                                                                                           
			{                                                                                                          
			    printf("write() failed. len = %d, ret = %d, errno = %d:%s\n", len, ret, errno, strerror(errno)); 
			    return -1;                                                   
			} 

			printf("write %d\n", ret);

			if (shutdown(fd, SHUT_RD) < 0)
			{
				perror("shutdown fali:");
				return -1;
			}
			for(i=0; i<170; i++)
			{
			//sleep(2);
			//printf("before wirte\n");
			ret = write(fd, (char*)buf, 1024); 
		     
			if (ret < 0)                                                                                           
			{                                                                                                          
			    printf("write() failed. len = %d, ret = %d, errno = %d:%s\n", len, ret, errno, strerror(errno)); 
			    return -1;                                                   
			} 

			//printf("write %d\n", ret);
			/*
				ret = read(fd, (char*)buf, len);      
			if (ret < 0)                                                                                           
			{                                                                                                          
			    printf("read() failed. len = %d, ret = %d, errno = %d:%s\n", len, ret, errno, strerror(errno)); 
			    return -1;                                                   
			}

				buf[ret] = '\0';
				printf("recieve buf:%s\n", buf);

				if(IsHalfClose(fd))
				{
					printf("it is a half conection\n");
				}
				else
				{
					printf("a full connection\n");
				}

				memset(buf, 0, sizeof(buf));*/
			}		
			
		}
	}
	
	/*flags = fcntl(fd, F_GETFL, 0);
	if(flags < 0)
	{
		perror("fcntl F_GETFL");
       		return -1;
	}
    	flags &= ~O_NONBLOCK;
    	if (fcntl(fd, F_SETFL, flags) < 0)
    	{
        	perror(	"sockets::SetNonBlockAndCloseOnExec, non_blocking");
		return -1;
    	}*/

	printf("before close\n");
	ret = close(fd);

	printf("close ret %d\n", ret);
	return 0;

}

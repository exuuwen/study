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
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
void sighandler(int sig) 
{
        printf("SIGpipe:I am \n");
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

int GetSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = sizeof optval;
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}
int SetRecvBufferSize(int sock, unsigned int size)                                                                      
{                                                                                                                  
	size_t opt = size;    
	int ret = -1;                                                      
	                                                                                                             
	ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt));                                                
	                                                                                                             
	return ret;                                                                                                            
}  

int SocketWritable(int sockfd, int tmo_ms)
{
    struct pollfd socketToPoll[1];
    socketToPoll[0].fd = sockfd;
    socketToPoll[0].events = POLLOUT;
    socketToPoll[0].revents = 0;

    return (poll(socketToPoll, 1, tmo_ms) > 0)  && (socketToPoll[0].revents & POLLOUT);
}
   
int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr;

	if(argc != 3)
	{
		printf("%s dst_ip dst_port\n", argv[0]);
		return 0;
	}

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);                                                                        
	if (sock < 0)                                                                                                 
	{                                                                                                              
		printf("StreamSocket Create socket failed\n");                                                           
		return -1;                                                                                               
	}    

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));  

	signal(SIGPIPE, sighandler);

	int flags = fcntl(sock, F_GETFL, 0);
	if(flags < 0)
	{
		perror("fcntl F_GETFL");
       		return -1;
	}
    	/*flags |= O_NONBLOCK;
    	if (fcntl(sock, F_SETFL, flags) < 0)
    	{
        	perror(	"sockets::SetNonBlockAndCloseOnExec, non_blocking");
		return -1;
    	}
	
	*/
	int ret;                                                                                                        
	struct sockaddr_in addr;                                                                                       
	socklen_t addr_len;                                                                                            
	
	printf("ip:0x%x, port:%d\n", get_local_ip(sock), get_local_port(sock));                                                                     
	addr_len = sizeof(addr);                                                                                       
	ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));   
	int savedErrno = (ret == 0) ? 0 : errno;                                                 
	switch (savedErrno)
        {
            case 0:
            case EINPROGRESS:
            case EINTR:
            case EISCONN:
                {
			if (SocketWritable(sock, 10) > 0)
			{
				if (GetSocketError(sock) == 0)
				{
					printf("connect success\n");
					break;
				}
			}
			
		       
                }                                                                                                  
	    default:   
		perror("connect server fail:");   
		close(sock);                                                             
		return -1;                                                                                               
	}       
	printf("ip:0x%x, port:%d\n", get_local_ip(sock), get_local_port(sock));
	flags = fcntl(sock, F_GETFL, 0);
	if(flags < 0)
	{
		perror("fcntl F_GETFL");
       		return -1;
	}
    	flags &= ~O_NONBLOCK;
    	if (fcntl(sock, F_SETFL, flags) < 0)
    	{
        	perror(	"sockets::SetNonBlockAndCloseOnExec, non_blocking");
		return -1;
    	}
	
	//ret = SetRecvBufferSize(sock, 0);
	if(ret < 0)
	{
		perror("set rev buffer fail:");
		return -1;
	}
	char buf[1024] = "shuwhude";
	int len = 6;//sizeof(buf);
	//close(sock);
	//return 0;
	int count = 0;
	int i;
	while(1)
	//for(i=0; i<10; i++)
	{
		ret = write(sock, (char *)buf, 1024);      
		if (ret < 0)                                                                                           
		{                                                                                                          
			printf("write() failed. len = %d, ret = %d, errno = %d:%s\n", len, ret, errno, strerror(errno)); 
			return -1;                                                   
		} 

		printf("write %d\n", ret);
		sleep(1);
		
		while(1)
		{
		memset(buf, 0, sizeof(buf));
		
		ret = read(sock, (char *)buf, 1024);      
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
			count++;
			buf[ret] = '\0';
			printf("%d ret %d recieve back buf: %s\n", count, ret, buf);     
		}
		}
		
		//close(sock);
	}
	

	//while(1);

}

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
#include <sys/epoll.h>

static int efd;

int epoll_init(unsigned int max_events)
{
    efd = epoll_create(max_events);
    if (efd == -1)
        return -1;

    return 0;
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

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));

	int reuse = 1;
	if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse)) < 0) 
	{
		perror("Setting SO_REUSEADDR error");
		close(sock);
		return -1;
	}

                                       
	int err = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) ;  
	if(err < 0)
	{
		perror("bind()");
		return -1;
	}     

	if (listen(sock, 1) < 0)                                                                                 
	{                                                                                                              
		printf("listen server failed\n");  
		close(sock);                                                           
		return -1;                                                                                               
	}    

	int m;
	err = epoll_init(10);
	if (err < 0)
	{
		perror("epoll_init");
		return -4;
	}

	struct epoll_event ev_accept;
	ev_accept.events = EPOLLIN ;
	ev_accept.data.fd = sock;
	err = epoll_ctl(efd, EPOLL_CTL_ADD, sock, &ev_accept);
	if (err < 0) 
	{
       		perror("epoll_ctl accept");
     		return -1;
	}


	char buf[1024];
	char sbuf[1024] = "shuwhude";
	int len = sizeof(buf);
	int ret, ne, i, ret1;
	struct epoll_event events[10]; 

	while(1)
	{
		ne = epoll_wait(efd, events, 10, -1);
		if (ne != 0)
         		printf("waiting ... got %d events\n", ne);
		for (i = 0; i < ne; i++) 
		{
         		int fd = events[i].data.fd;
			if ((events[i].events & EPOLLIN) && (fd == sock))
			{
				int fds;                                                                                
				struct sockaddr_in addr;                                                                                       
				socklen_t addr_len;                                                                                            
			                                                                                           
				addr_len = sizeof(addr);
				fds = accept(sock, (struct sockaddr *) &addr, &addr_len);                                                      
				if (fds < 0)
				{
					perror("accept fail:");
				}      
	
				printf("accept success\n");

				struct epoll_event ev;
				ev.events = EPOLLIN | EPOLLHUP | EPOLLPRI | EPOLLRDHUP;
				ev.data.fd = fds;
				err = epoll_ctl(efd, EPOLL_CTL_ADD, fds, &ev);
				if (err < 0) 
				{
       					perror("epoll_ctl");
     					return -1;
				}
			}
			else
			{
				if (events[i].events & EPOLLIN)
					printf("epollin\n");
				if (events[i].events & EPOLLPRI)
					printf("epollpri\n");
				if (events[i].events & EPOLLRDHUP)
					printf("epollrdhup\n");
				if (events[i].events & EPOLLHUP)
					printf("epollhup\n");
								

           			if ((events[i].events & EPOLLIN | EPOLLPRI | EPOLLRDHUP)) 
				{
					ret1 = read(fd, (char *)buf, len);      
					if (ret1 < 0)                                                                                          
					{
						printf("read() failed. len = %d, ret = %d, errno = %d:%s\n", len, ret, errno, strerror(errno));
						return -1;                                                   
					}
					else if (!(events[i].events & EPOLLHUP))
					{
						buf[ret1] = '\0';
						printf("recieve buf:%s, %d\n", buf, ret1);
		
						sleep(2);
						ret = write(fd, (char*)sbuf, 6);  
						if (ret < 0)
						{    
			    				printf("write() failed. len = %d, ret = %d, errno = %d:%s\n", len, ret, errno, strerror(errno)); 
			    				return -1;   
						}                                                
						printf("write %d\n", ret);
					} 
		
//					shutdown(fd, SHUT_RD);	
					memset(buf, 0, sizeof(buf));
				}
				
				if((events[i].events & EPOLLHUP) || (events[i].events & EPOLLIN) && (events[i].events & EPOLLRDHUP) && ret1==0)
				{
					printf("client bye\n");
					err = epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
					if (err < 0) 
					{
       						perror("epoll_ctl del");
     						return -1;
					}	
					close(fd);
				}
			}	
		}
	}

}

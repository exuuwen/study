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
#include <sys/epoll.h>

#define MCAST_ADDR "224.0.1.88"     
#define MCAST_INTERVAL 5                        
#define BUFF_SIZE 256   

static int efd;
/* change socket to nonblocking mode */
int make_socket_nonblocking(int sfd)
{
    int flags, s;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) 
	{
        perror("fcntl F_GETFL");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) 
	{
        perror("fcntl F_SETFL");
        return -1;
    }

    return 0;
}           

int epoll_init(unsigned int max_events)
{
    efd = epoll_create(max_events);
    if (efd == -1)
        return -1;

    return 0;
}     
        
int main(int argc, char*argv[])
{  
	int s;                                      
	struct sockaddr_in local_addr;              
	int err = -1;

	if (argc != 3 && argc != 4)
	{
		printf("%s joined_ip port\n", argv[0]);
		return -1;
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);     
	if (s == -1)
	{
		perror("socket()");
		return -1;
	}  

	/* Enable SO_REUSEADDR  can bind several clients on the same ip an port */
	int reuse = 1;
	if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse)) < 0) 
	{
		perror("Setting SO_REUSEADDR error");
		close(s);
		return -1;
	}
                      
	int all = 1;
	if (setsockopt (s, IPPROTO_IP, IP_MULTICAST_ALL, (char *) &all, sizeof(all)) < 0) 
	{
		perror("Setting mulicasr all error");
		close(s);
		return -1;
	}
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = inet_addr(MCAST_ADDR);/*muticast bind only receive from this packet, when you should send a muticast packet it should use IP_MULTICAST_IF to specifc the src of the packet*/
	local_addr.sin_port = htons(atoi(argv[2]));
                                       
	err = bind(s, (struct sockaddr*)&local_addr, sizeof(local_addr)) ;
	if(err < 0)
	{
		perror("bind()");
		return -2;
	}

	err = make_socket_nonblocking(s); 
   	if (err != 0) 
	{
        	perror("nonblocking()");
        	close(s);
        	return -1;
	}

	struct ip_mreqn mreq;                                    
	mreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR); 
	mreq.imr_address.s_addr = inet_addr(argv[1]);//add the interface that the ip addr bind to mcast_addr group
	//mreq.imr_ifindex = 4; //eth0
		                                       
	err = setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	if (err < 0)
	{
		perror("setsockopt():IP_ADD_MEMBERSHIP");
		return -4;
	}

	if (argc ==4)
		goto out;
	
	err = epoll_init(10);
	if (err < 0)
	{
		perror("epoll_init");
		return -4;
	}
	
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = s;
	err = epoll_ctl(efd, EPOLL_CTL_ADD, s, &ev);
	if (err < 0) 
	{
       		perror("epoll_ctl");
     		return -1;
	}

	int times = 0;
	char buff[BUFF_SIZE];
	int n = 0;
	struct epoll_event events[10];     
	int i, ne;  

	while(1)
	{
		ne = epoll_wait(efd, events, 10, -1);
        if (ne != 0)
         	printf("waiting ... got %d events\n", ne);
        for (i = 0; i < ne; i++) 
		{
         	int fd = events[i].data.fd;
           	if ((events[i].events & EPOLLIN) && (fd == s)) 
			{
				memset(buff, 0, BUFF_SIZE);                 
				                          
				n = recvfrom(s, buff, BUFF_SIZE, 0, NULL, NULL);
				if( n == -1)
		   			perror("recvfrom()");
				else				
					printf("Recv %dst message from server:%s\n", times++, buff);

			}
		}
	}

out:
	err = setsockopt(s, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));

	close(efd);
	close(s);

	return 0;
}

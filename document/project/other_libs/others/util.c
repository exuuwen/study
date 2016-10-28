#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <time.h>
#include <assert.h>

#define SEC        1
#define MILLISEC   1000         /* Numb. of millisecs. in a second. */
#define MICROSEC   1000000      /* Numb  of microsecs. in a second */
#define NANOSEC    1000000000   /* Numb. of nanosecs.  in a second */

long diff_time(struct timespec *tnew, struct timespec *told)
{
	long sec;
	long nsec;
	long msec;

	if ((tnew->tv_sec < told->tv_sec) || ((tnew->tv_sec == told->tv_sec) && (tnew->tv_nsec < told->tv_nsec))) 
	{
		msec = -1;
	} 
	else 
	{
		sec = tnew->tv_sec - told->tv_sec;
		nsec =  tnew->tv_nsec - told->tv_nsec;
		msec = (sec * MILLISEC) + (nsec / MICROSEC);
	}

	return msec;
}
int get_current_time(struct timespec *tp)
{
	
	if (clock_gettime(CLOCK_REALTIME, tp) != 0)   //-lrt
	{
		perror("clock_gettime:");
		return -1;
	}

	return 0;
}



int create_dir(const char *dir)
{
   	char *tmpdir;
    	int i, len;

    	tmpdir = malloc(strlen(dir) + 2);
    	if (tmpdir == NULL) 
	{
		free(tmpdir);
        	return -1;
	}

    	strcpy(tmpdir, dir);
    	if (tmpdir[strlen(tmpdir) - 1] != '/')
	{
        	strcat(tmpdir, "/");
	}

    	len = strlen(tmpdir);
    	for (i = 1; i < len; i++) 
	{
        	if (tmpdir[i] == '/') 
		{
            		tmpdir[i] = '\0';
            		if (access(tmpdir, R_OK) != 0) 
			{
                		if (mkdir(tmpdir, 0755) == -1) 
				{
             				free(tmpdir);
                    			return -1;
                		}
            		}
            		tmpdir[i] = '/';
        	}	
    	}

	free(tmpdir);
	return 0;
}

int create_file(const char *path)
{
	char * tmp;
	int i, len;

	if (path == NULL)
	{
		return -1;
	}

	len = strlen(path);
	if (len == 0 || path[len-1] == '/')
	{
		return -1;
	}

	tmp = strdup(path);
	if (tmp == NULL)
	{
		return -1;
	}

	/* dir */
	for (i = 1; i < len; i++) 
	{
		if (tmp[i] == '/')
		{
			tmp[i] = '\0';
			if (access(tmp, R_OK) != 0 && mkdir(tmp, 0755) == -1) 
			{
				free(tmp);
				return -1;
			}
			tmp[i] = '/';
		}
	}

	/* file */
	if (access(tmp, F_OK) != 0) 
	{
		int fd = creat(tmp, 0644);
		if (fd < 0) 
		{
			free(tmp);
			return -1;
		}
		close(fd);
	}

	free(tmp);
	return 0;
}

int set_nonblocksocket(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0);
	if(flags < 0)
	{
		perror("fcntl F_GETFL");
       		return -1;
	}
    	flags |= O_NONBLOCK;
    	if (fcntl(sock, F_SETFL, flags) < 0)
    	{
        	perror(	"sockets::SetNonBlockAndCloseOnExec, non_blocking");
		return -1;
    	}
}

int get_socketerror(int sockfd)
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

int socket_readable(int sockfd, int tmo_ms)
{
	struct pollfd socketToPoll[1];
	socketToPoll[0].fd = sockfd;
	socketToPoll[0].events = POLLIN|POLLPRI;
	socketToPoll[0].revents = 0;

	return (poll(socketToPoll, 1, tmo_ms) > 0) && (socketToPoll[0].revents & (POLLIN|POLLPRI));
}

int socket_writable(int sockfd, int tmo_ms)
{
	struct pollfd socketToPoll[1];
	socketToPoll[0].fd = sockfd;
	socketToPoll[0].events = POLLOUT;
	socketToPoll[0].revents = 0;

	return (poll(socketToPoll, 1, tmo_ms) > 0)  && (socketToPoll[0].revents & POLLOUT);
}

int set_keepalive(int sd, int time, int intvl, int probes)
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

unsigned int get_sendbuffer_size(int sock)                                                                               
{                                                                                                                  
	size_t opt = 0;                                                                                                
	socklen_t len = sizeof(opt); 
                                                                                                           
	getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &opt, &len);  	
	return opt
}

int set_sendbuffer_size(int sock, unsigned int size)                                                                      
{                                                                                                                  
	size_t opt = size;    
	int ret = -1;                                                      
	                                                                                                             
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt));                                                
	                                                                                                             
	return ret;                                                                                                            
}  

unsigned int get_recievebuffer_size(int sock)                                                                               
{                                                                                                                  
	size_t opt = 0;                                                                                                
	socklen_t len = sizeof(opt); 
                                                                                                           
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &opt, &len);  	
	return opt
}

int set_recievebuffer_size(int sock, unsigned int size)                                                                      
{                                                                                                                  
	size_t opt = size;    
	int ret = -1;                                                      
	                                                                                                             
	ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt));                                                
	                                                                                                             
	return ret;                                                                                                            
}  

int set_reuseaddr(int sockfd, int on)
{
	int optval = on ? 1 : 0;

	int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
  	return ret;		
}

int set_solinger(int sockfd, int on, int time)
{
	struct linger so_linger;

	so_linger.l_onoff = on;
	so_linger.l_linger = time;

	int ret = setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));

	return ret;
}

void set_tcp_nodelay(int sockfd, bool on)
{
	int optval = on ? 1 : 0;
	if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval) < 0)
	{
		return -1;
	}
	
	return 0;
}

char* get_local_ip(int sd)
{
	struct sockaddr addr;
	int len = sizeof(addr);
	if (getsockname(sd, &addr, (unsigned *)&len) != 0)
	{
		return NULL;
	}

	if (addr.sa_family != AF_INET)
	{
		return NULL;
	}

	char * ip = inet_ntoa(((struct sockaddr_in *)(&addr))->sin_addr);
	if (ip == NULL)
	{
		return NULL;
	}

	return strdup(ip);
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

int list_all_interface(char *list[], int hw)
{
	struct ifaddrs *ifaddr, *ifa;
	int ip_interface_num = 0;

	if (getifaddrs(&ifaddr) == -1) 
	{
		perror("getifaddrs");
		return -1;
	}

	int ret = -1;
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		char host[20];
		if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
		continue;
		/* get ip */
		int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if (s != 0) 
		{
			perror("getnameinfo");
			goto out;
		}
		/*It also can get ip through following way*/
		/*char * ip = inet_ntoa(((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr);*/
		if(hw)
		{
			/* check hwaddr is real */
			/* get the MAC addr */
			struct ifreq buf;
			bzero(&buf, sizeof(buf));
			s = socket(AF_INET, SOCK_DGRAM, 0);
			strcpy(buf.ifr_name, ifa->ifa_name); /*ifa->ifa_name: eth0*/
			if (ioctl(s, SIOCGIFHWADDR, &buf)) 
			{
				perror("SIOCGIFHWADDR");
				close(s);
				goto out;
			}
			close(s);
			if (buf.ifr_hwaddr.sa_data[0] == 0 &&
			buf.ifr_hwaddr.sa_data[1] == 0 &&
			buf.ifr_hwaddr.sa_data[2] == 0 &&
			buf.ifr_hwaddr.sa_data[3] == 0 &&
			buf.ifr_hwaddr.sa_data[4] == 0 &&
			buf.ifr_hwaddr.sa_data[5] == 0)
			continue;
		}
		
		list[ip_interface_num] = strdup(host);
		ip_interface_num++;
		
	}

	out:
    	freeifaddrs(ifaddr);
    	return ip_interface_num;
}

char* get_interface_ip(char *ifname)
{
	struct ifreq ifr;
	bzero(&ifr, sizeof(ifr));
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, ifname); /*ifa->ifa_name: eth0*/
	if (ioctl(s, SIOCGIFADDR, &ifr)) 
	{
		perror("SIOCGIFADDR");
		close(s);
		return NULL;
	}

	close(s);
	struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
	char *ip = NULL;
	ip = inet_ntoa(ipaddr->sin_addr);

	if(ip == NULL)
	{
		return NULL;
	}
	
	return strdup(ip);
}


int get_interface_mac(char *ifname, unsigned char *mac)
{
	struct ifreq buf;
	bzero(&buf, sizeof(buf));
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(buf.ifr_name, ifname); /*ifa->ifa_name: eth0*/
	if (ioctl(s, SIOCGIFHWADDR, &buf)) 
	{
		perror("SIOCGIFHWADDR");
		close(s);
		return -1;
	}

	close(s);
	memcpy(mac, buf.ifr_hwaddr.sa_data, 6);

	return 0;
}

int main()
{
	return 0;
}

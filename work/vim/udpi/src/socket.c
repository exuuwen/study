#ifndef __cplusplus
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <poll.h>
#include <linux/tcp.h>

#include "main.h"
#include "socket.h"

#define UDPI_WRITE_TIMEOUT 1000

void udpi_init_socket(void)
{
	/*ignore sigpipe*/
	signal(SIGPIPE, SIG_IGN);

	uint32_t i;
	
	for(i=0; i<RTE_MAX_LCORE; i++)
	{
		udpi.dumper_fds[i] = -1;
		udpi.packet_fds[i] = -1;
	}
}

static int get_socketerror(int sockfd)
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

static int socket_writable(int sockfd, int tmo_ms)
{
	struct pollfd socket2poll[1];
	socket2poll[0].fd = sockfd;
	socket2poll[0].events = POLLOUT;
	socket2poll[0].revents = 0;

	return (poll(socket2poll, 1, tmo_ms) > 0) && (socket2poll[0].revents & POLLOUT);
}

static void set_tcp_nodelay(int sockfd, int on)
{
	int optval = on ? 1 : 0;

	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

static unsigned int get_sendbuf_size(int sock)
{
        size_t opt = 0;
        socklen_t len = sizeof(opt);

        getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &opt, &len);
        return opt;
}

static int set_sendbuf_size(int sock, unsigned int size)                                                                      
{                                                                                                                  
    size_t opt = size;    
    int ret = -1;                                                      
                                                                                                                  
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt));                                                
                                                                                                                  
    return ret;                                                                                                            
}  

static int udpi_socket_create(uint32_t server_ip, uint16_t port)
{
	struct sockaddr_in server_addr;

	int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);                                                                        
	if (sock < 0)                                                                                                 
	{
		return -1;
	}
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = server_ip;
	server_addr.sin_port = port;  

	int ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));                                                    
	int savedErrno = (ret == 0) ? 0 : errno;                                                 
	switch (savedErrno)
	{
		case 0:
		case EINPROGRESS:
		case EINTR:
		case EISCONN:
		{
			if (socket_writable(sock, UDPI_WRITE_TIMEOUT) > 0)
			{
				if (get_socketerror(sock) == 0)
				{
					printf("port %u connect success\n", ntohs(port));
					break;
				}
			}
			
		       
		}                                                                                                 
		default:   
			printf("port %u connect fail\n", ntohs(port));
			close(sock);                                                             
			return -1;
	}       

	set_tcp_nodelay(sock, 1);
	
	set_sendbuf_size(sock, 4194304);

	printf("port %u get sendbuf:%u\n", ntohs(port), get_sendbuf_size(sock));

	return sock;
}

void udpi_socket_dumper_create(uint32_t dumper_id)
{
	uint8_t factor = 0;
	uint16_t port;

	if (udpi.dpdk_id >= 4)
		factor = 4;
	port = htons(IPORT + dumper_id + 2*(udpi.dpdk_id - factor));  
	
	udpi.dumper_fds[dumper_id] = udpi_socket_create(udpi.server_ip_be, port);
}

void udpi_socket_packet_create(uint32_t packet_id)
{
	uint16_t port;

	port = htons(PPORT + udpi.dpdk_id);  
	
	udpi.packet_fds[packet_id] = udpi_socket_create(udpi.userver_ip_be, port);
}


static int udpi_socket_write(int fd, uint8_t *data, size_t len)
{
	ssize_t nwrote = 0;
	size_t remaining = len;
	uint32_t offset = 0;

	while (remaining > 0)
	{
		nwrote = write(fd, data + offset, remaining);
		if (nwrote >= 0)
		{
			remaining -= nwrote;
			offset += nwrote;
		}
		else
		{
			int serrno = errno;
			if (serrno != EWOULDBLOCK)
			{
				if (serrno == EPIPE || serrno == ECONNRESET) // FIXME: any others?
				{
					printf("epipe or econnrest error\n");
				}
				
				return -1;
			}
			else
			{
				if (socket_writable(fd, UDPI_WRITE_TIMEOUT) == 0)
				{
					printf(" wouldblock for %d ms\n", UDPI_WRITE_TIMEOUT);
					return -1;
				}
			}
			//printf("hahahah write fail...%d........\n", serrno);
		}
	}

	return 0;
}

void udpi_socket_dumper_write(uint32_t dumper_id, uint8_t *data, size_t len)
{
	int fd = udpi.dumper_fds[dumper_id];
	
	int ret = udpi_socket_write(fd, data, len);
	if (ret)
	{
		printf("dumper id %u write error\n", dumper_id);
		close(udpi.dumper_fds[dumper_id]);
		rte_mb();
		udpi_socket_dumper_create(dumper_id);
	}
}

int udpi_socket_packet_write(uint32_t packet_id, uint8_t *data, size_t len)
{
	int fd = udpi.packet_fds[packet_id];
	
	int ret = udpi_socket_write(fd, data, len);
	if (ret)
	{
		printf("packet id %u write error\n", packet_id);
		close(udpi.packet_fds[packet_id]);
		rte_mb();
		udpi_socket_packet_create(packet_id);
	}

	return ret;
}

static int udpi_socket_read(int fd)
{
	struct pollfd socket2poll[1];
	socket2poll[0].fd = fd;
	socket2poll[0].events = POLLIN | POLLHUP | POLLPRI | POLLRDHUP;
	socket2poll[0].revents = 0;

	char buf[4096];
	int ret = 0;

	if (poll(socket2poll, 1, 0) > 0) 
	{
		if (socket2poll[0].revents & (POLLIN | POLLPRI | POLLRDHUP))
			ret = read(fd, (char *)buf, 1024);
		
		if (((socket2poll[0].revents & POLLHUP) || (socket2poll[0].revents & POLLIN)) && (socket2poll[0].revents & POLLRDHUP) && (ret == 0))
			return -1;
	}

	return 0;
}

void udpi_socket_dumper_read(uint32_t dumper_id)
{
	int fd = udpi.dumper_fds[dumper_id];
	int ret = udpi_socket_read(fd);
	
	if (ret)
	{
		printf("dumper id %u server bye\n", dumper_id);
		close(udpi.dumper_fds[dumper_id]);
		rte_mb();
		udpi_socket_dumper_create(dumper_id);
	}
}

void udpi_socket_packet_read(uint32_t packet_id)
{
	int fd = udpi.packet_fds[packet_id];
	int ret = udpi_socket_read(fd);
	
	if (ret)
	{
		printf("packet id %u server bye\n", packet_id);
		close(udpi.packet_fds[packet_id]);
		rte_mb();
		udpi_socket_packet_create(packet_id);
	}
}


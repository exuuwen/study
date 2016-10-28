#include <stdio.h>  
#include <stdint.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/un.h>   
#include <ev.h>

// socat stdin unix-connect:/tmp/UNIX.udpi

#define UNIX_DOMAIN "/tmp/UNIX.udpi"  
#define MAX_UNIX 2

static struct ev_loop * loop;
static struct ev_io read_watcher[MAX_UNIX];
static uint8_t usage[MAX_UNIX];

static void
read_cb (struct ev_loop *loop, ev_io *w, int revents)
{
	char recv_buf[1024];   
	
	memset(recv_buf, 0, sizeof(recv_buf));

	int ret = read(w->fd, recv_buf, sizeof(recv_buf));
	if (ret < 0)
	{
		printf("read error %d\n", w->fd);
		goto release;
	}
	else if (ret == 0)
	{
		printf("client closed %d\n", w->fd);
		goto release;
	}

	printf("%d:%s", w->fd, recv_buf);

	write(w->fd, recv_buf, ret);

	return;

release:
	close(w->fd);
	int i;
 	ev_io_stop(loop, w);
	for (i=0; i<MAX_UNIX; i++)
	{
		if (w == &read_watcher[i])
		{
			usage[i] = 0;
			break;
		}
	} 
}

static void
accept_cb (struct ev_loop *loop, ev_io *w, int revents)
{
	socklen_t len;  
	struct sockaddr_un clt_addr;  
    int com_fd;  
	

	//have connect request use accept  
	len = sizeof(clt_addr);  
	com_fd = accept(w->fd, (struct sockaddr*)&clt_addr, &len);  
	if(com_fd < 0)  
	{  
		perror("cannot accept client connect request");  
		return;  
	} 

	int i;
	for (i=0; i<MAX_UNIX; i++)
	{
		if (usage[i] == 0)
		{
			usage[i] = 1;
			break;
		}
	} 

	if (i == MAX_UNIX)
	{
		printf("client is more than %d\n", MAX_UNIX);  
		close(com_fd);
		return;  
	}

	ev_init(&read_watcher[i], read_cb);
	ev_io_set(&read_watcher[i], com_fd, EV_READ);
	ev_io_start(loop, &read_watcher[i]);
} 



int main(void)  
{  
	int listen_fd;  
	int ret;  
 	int i;  
	int len;  
	struct sockaddr_un srv_addr;  
    
	listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);  
    if (listen_fd < 0)  
	{  
		perror("cannot create communication socket");  
		return -1;  
	}    
          
	//set server addr_param  
	srv_addr.sun_family = AF_UNIX;  
	strncpy(srv_addr.sun_path, UNIX_DOMAIN, sizeof(srv_addr.sun_path)-1);  
	unlink(UNIX_DOMAIN);  

	//bind sockfd & addr  
	ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));  
	if(ret < 0)  
	{  
		perror("cannot bind server socket");  
		close(listen_fd);  
		unlink(UNIX_DOMAIN);  
		return -1;  
	}  

	//listen sockfd   
	ret = listen(listen_fd, 10);  
	if (ret < 0)  
	{  
		perror("cannot listen the client connect request");  
		close(listen_fd);  
		unlink(UNIX_DOMAIN);
		return -1;  
	}  


	loop = ev_default_loop(0);

	struct ev_io accept_watcher;
	ev_init(&accept_watcher, accept_cb);
	ev_io_set(&accept_watcher, listen_fd, EV_READ);
	ev_io_start(loop, &accept_watcher);

	ev_run(loop, 0);

	return 0;  
}  


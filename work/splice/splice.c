#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <netdb.h>
#include <sys/socket.h>
int main(int argc,char *argv[])
{
        int fd, client_fd, server_fd, ret;
        int yes = 1;
        struct sockaddr_in addr = {0};
        fd = socket(AF_INET,SOCK_STREAM,0);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(1111);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        ret = bind(fd,(struct sockaddr *)(&addr),sizeof(struct sockaddr));
        ret = setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
        ret = listen(fd,5);
        socklen_t len = sizeof(addr);
        client_fd = accept(fd,(struct sockaddr*)(&addr),&len);
        { //接收客户端，连接服务端。
                struct sockaddr_in addr = {0}, addr2= {0};
                struct in_addr x;
                inet_aton("127.0.0.1", &x);
                addr.sin_family = AF_INET;
                addr.sin_addr = x;
                addr.sin_port = htons(2222);
                server_fd = socket(AF_INET,SOCK_STREAM,0);
                ret = connect(server_fd,(struct sockaddr*)&addr,sizeof(addr));
        }
        { //将客户端数据转发到服务端
                struct pollfd pfds[2];
                int pfd[2] = {-1};
                int ret = -1;
                pfds[0].fd = client_fd;
                pfds[0].events = POLLIN|POLLOUT;
                pfds[1].fd = server_fd;
                pfds[1].events = POLLIN|POLLOUT;
                ret = pipe(pfd);
                while (1) {
                        int efds = -1;
                        if ((efds = poll(pfds,2,-1)) < 0) {
                                return -1;
                        }
                        //两次splice，第一次sock_client->pipe，第二次pipe->sock_server
                        if(pfds[0].revents & POLLIN) {
                                int rncount = splice(pfds[0].fd, NULL, pfd[1], NULL, 65535, SPLICE_F_MORE);
                                int wncount = splice(pfd[0], NULL, pfds[1].fd, NULL, 65535, SPLICE_F_MORE);
                        }
                        if(pfds[1].revents & POLLIN) {
                                int rncount = splice(pfds[1].fd, NULL, pfd[1], NULL, 65535, SPLICE_F_MORE);
                                int wncount = splice(pfd[0], NULL, pfds[0].fd, NULL, 65535, SPLICE_F_MORE);
                        }
                }
        }
}

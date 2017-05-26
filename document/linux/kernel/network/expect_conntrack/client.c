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
#include <stdio.h>
  
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

struct test_proto {
     int type;
     int port;
     in_addr_t addr;
};


int main(int argc, char *argv[])
{
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

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

        int err = bind(sock, (struct sockaddr*)&client_addr, sizeof(client_addr)) ;
        if(err < 0)
        {
                perror("bind()");
                return -1;
        }
	printf("local ip 0x%x, port %d\n", get_local_ip(sock), get_local_port(sock));
/*	char* dev = "eth1";
        if (setsockopt (sock, SOL_SOCKET, SO_BINDTODEVICE, dev, sizeof("dev1")) < 0)
        {
                perror("Setting SO_REUSEADDR error");
                close(sock);
                return -1;
        }
*/
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);;
	server_addr.sin_port = htons(atoi(argv[2]));  

	
	int ret;                                                                                                        
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
	printf("local ip 0x%x, port %d\n", get_local_ip(sock), get_local_port(sock));

    char *sbuf;
    struct test_proto ap = {0};
    ap.addr = inet_addr("192.168.0.2");
    ap.type = 100;
    ap.port = 2152;
    int len = sizeof(struct test_proto);
    sbuf = (char*)&ap;

    write(sock, (char *)sbuf, len);

    sleep(10);

    close(sock);


    /*
	char buf[1024] = "shuwhude";
	int len = 6;//sizeof(buf);
	
	while(1)
	{	
		printf("local ip 0x%x, port %d\n", get_local_ip(sock), get_local_port(sock));
		ret = write(sock, (char *)buf, len);      
		if (ret < 0)                                                                                           
		{                                                                                                          
			printf("write() failed. len = %d, ret = %d, errno = %d:%s\n", len, ret, errno, strerror(errno)); 
			return -1;                                                   
		} 
		sleep(1);	
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
			buf[ret]='\0';
			printf("recieve back buf: %s\n", buf);     
		}
	
		//close(sock);
	}
    */

}

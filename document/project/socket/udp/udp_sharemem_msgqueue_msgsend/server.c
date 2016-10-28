#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define BUF_SIZE 32

struct msgsbuf{
	int mtype;
	int offset;
	char mtext[BUF_SIZE];
}msg_sbuf,msg_sbuf2;


int PutFifo()
{
	key_t key;
	int gflags, sflags;
	int msgid;
	int reval;
	
	if((msgid=msgget((key_t)0x12345,0666|IPC_CREAT))==-1)
	{
		perror("can  not  msgget");
		exit(1);
	}
	
	sflags=IPC_NOWAIT;
	reval = msgsnd(msgid, &msg_sbuf, sizeof(msg_sbuf), sflags);
	if(reval == -1){
		//printf("message send error.\n");
		return -1;
	}
	
	return 0;	
}

void dg_echo(int server_socketfd,struct sockaddr_in client_address,int client_len)
{
	int n,byte;
	int offset=0;
	char test[BUF_SIZE];
	void *sharemem=NULL;
	int shmid,i;
	n=0;
	//char rec_char[MAXLINE];
	while(n<10)
	{
	struct msghdr msg;        struct iovec iov[1];       // ssize_t n;        bzero(&msg, sizeof(msg));        msg.msg_name = &client_address;        /* attention this is a pointer to void* type */        msg.msg_namelen = client_len;        iov[0].iov_base = &msg_sbuf;        iov[0].iov_len = sizeof(msg_sbuf);        msg.msg_iov = iov;        msg.msg_iovlen = 1;
	if (recvmsg(server_socketfd, &msg, 0) == -1)                perror("rcv bad");
	
	printf("now reciev the data ok data is :%s.and type is %d and the len is %d  \n",msg_sbuf.mtext,msg_sbuf.mtype,msg_sbuf.offset);
	PutFifo();
	n++;
	}
	sleep(3);
	n=0;
	printf("hhshhahhha............................\n");
	while(n<10)
	{
		memset(test,0,sizeof(test));
	
		if((shmid=shmget((key_t)654321,(size_t)4096,0600|IPC_CREAT))==-1) 
		{
			perror("can not get share mem");
			exit(1);
		}
		if((sharemem=shmat(shmid,NULL,0))==NULL)
		{
			perror("can  mount the  share  mem");
			shmctl(shmid,IPC_RMID,0);
			exit(1);
		}	
		memcpy(&msg_sbuf2,sharemem+offset,sizeof(msg_sbuf2));
		//i=strlen(test);
		//test[i]=0;
		printf("sharemem read  data ok data is :%s.and type is %d and the len is %d  \n",msg_sbuf2.mtext,msg_sbuf2.mtype,msg_sbuf2.offset);
		offset+=sizeof(msg_sbuf2);
		n++;
	}
		shmctl(shmid,IPC_RMID,0);	
	
}

int main(int  argc, char* argv[])
{
	int server_socketfd,client_socketfd;
	int server_len,client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	int i,byte,n;
	
	
	server_socketfd=socket(AF_INET,SOCK_DGRAM,0);

	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=inet_addr("192.168.126.129");
	server_address.sin_port=htons(4321);	
	server_len=sizeof(server_address);
	
	bind(server_socketfd,(struct sockaddr *)&server_address,server_len);	
	
	

	printf("now recive the  data  from  client\n");
	client_address.sin_family=AF_INET;
	client_address.sin_addr.s_addr=inet_addr("127.0.0.1");
	client_address.sin_port=htons(1234);	
	client_len=sizeof(server_address);
	
	dg_echo(server_socketfd,client_address,client_len);
	shutdown(client_socketfd,2);
	shutdown(server_socketfd,2);
	//unlink("server_socket");
	
}


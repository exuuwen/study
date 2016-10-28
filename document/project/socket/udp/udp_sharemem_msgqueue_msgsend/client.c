#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define BUF_SIZE 32

struct msgsbuf{
	int mtype;
	int offset;
	char mtext[BUF_SIZE];
}msg_sbuf,msg_sbuf2;

int GetFifo(){
	key_t key;
	int rflags;
	int msgid;
	int reval;
	if((msgid=msgget((key_t)0x12345,0666|IPC_CREAT))==-1)
	{
		perror("can  not  msgget");
		exit(1);
	}
	
	rflags = MSG_NOERROR; //IPC_NOWAIT do not wait to recevie any message.
	reval = msgrcv(msgid, &msg_sbuf2, sizeof(msg_sbuf2), msg_sbuf.mtype, rflags);
	if(reval == -1){
		printf("message receive error.\n");
		return -1;
	}	
	printf("now reciev the queue ok data is :%s. and type is %d and the len is %d  \n",msg_sbuf2.mtext,msg_sbuf2.mtype,msg_sbuf2.offset);
	return 0;
}

void dg_cli(int socketfd,struct sockaddr_in *server_address,int server_len)
{
	
	int n=0;
	int offset=0;
	int i;
	char test[]="this wenxu  test";
	void *sharemem=NULL;
	int shmid;
	int msgid;
	while(n<10)
	{
	memset(msg_sbuf.mtext,0,sizeof(msg_sbuf.mtext));
	sprintf(msg_sbuf.mtext,"%s %d",test,n);
	
	//printf("the data is %d\n",n);		
	msg_sbuf.mtype=2;
	msg_sbuf.offset=strlen(msg_sbuf.mtext);
	struct msghdr msg;        struct iovec iov[1];        bzero(&msg, sizeof(msg));        msg.msg_name = server_address;        /* attention this is a pointer to void* type */        msg.msg_namelen = server_len;        iov[0].iov_base = &msg_sbuf;        iov[0].iov_len = sizeof(msg_sbuf);        msg.msg_iov = iov;        msg.msg_iovlen = 1;
	if (sendmsg(socketfd, &msg, 0) == -1)                perror("sendmsg bad");
	printf("send buf ok %s \n",msg_sbuf.mtext);
	n++;	
	sleep(1);
	}
	n=0;
	while(n<10)
	{
		GetFifo();
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
		memcpy(sharemem+offset,&msg_sbuf2,sizeof(msg_sbuf2));		
		//printf("child input   data   and   data hhhhhhhh is : %s \n",sharemem+offset); 
		offset+=sizeof(msg_sbuf2);
		n++;	
	}
	if((msgid=msgget((key_t)0x12345,0666|IPC_CREAT))==-1)
	{
		perror("can  not  msgget");
		exit(1);
	}
	if(msgctl(msgid,IPC_RMID,0)==-1)
	{	
		perror("msgid rm bad");
		exit(1);
	}	
	printf("over\n");
}

int main(int  argc, char* argv[])
{
	int socketfd,res;
	int server_len,client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	//struct sockaddr_in reply_address;
	int i,byte,n;
	
	
	socketfd=socket(AF_INET,SOCK_DGRAM,0);
	
	client_address.sin_family=AF_INET;
	client_address.sin_addr.s_addr=inet_addr("127.0.0.1");
	client_address.sin_port=htons(1234);	
	client_len=sizeof(server_address);
	
	bind(socketfd,(struct sockaddr *)&client_address,client_len);

	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=inet_addr("192.168.126.129");
	server_address.sin_port=htons(4321);	
	server_len=sizeof(server_address);
	

	printf("now send the  data  to  server\n");
	
	dg_cli(socketfd,&server_address,server_len);
	close(socketfd);
	return 0;
	
}


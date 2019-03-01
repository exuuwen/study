/*======================================================================
    A test program to access /dev/second
    This example is to help understand async IO 
    
    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
/*接收到异步读信号后的动作*/

#define FIFO_CLEAR 0x1
int fd;
void input_handler(int signum)
{
  int num;
  char buf[20];
  printf("receive a signal from globalfifo,signalnum:%d\n",signum);
  num=read(fd,buf,sizeof(buf));
	if(num<0)
	{
		perror("read bad\n");
		exit(1);
	}	
	buf[num]=0;
	printf("the content is :: %s \n",buf);
	//sleep(2);	
}

main()
{
  int  oflags,num,i=0;
  char buf[20];
  fd_set rfds;
  fd = open("/dev/globafifo", O_RDWR);
  if (fd !=  - 1)
  {
    if(ioctl(fd,FIFO_CLEAR,0)<0)
	{
		perror("ioctl error");
		exit(1);
	}
	//wait_head
	while(i<10)
	{	
		num=read(fd,buf,sizeof(buf));
		if(num<0)
		{
			perror("read bad\n");
			exit(1);
		}	
		buf[num]=0;
		printf("the content is :: %s \n",buf);
		sleep(1);	
		i++;
	}
	if(ioctl(fd,FIFO_CLEAR,0)<0)
	{
		perror("ioctl error");
		exit(1);
	}
	i=1;
        //poll
	 while(i<10)
	{
		FD_ZERO(&rfds);
		FD_SET(fd,&rfds);

		select(fd+1,&rfds,NULL,NULL,NULL);
		if(FD_ISSET(fd,&rfds))
		{	
			printf("you kan read now\n");
			num=read(fd,buf,sizeof(buf));
			if(num<0)
			{
				perror("read bad\n");
				exit(1);
			}	
			buf[num]=0;
			printf("the content is :: %s \n",buf);
			sleep(1);	
			i++;
		}
	}
    
	if(ioctl(fd,FIFO_CLEAR,0)<0)
	{
		perror("ioctl error");
		exit(1);
	}
	//SIGIO
    signal(SIGIO, input_handler); //让input_handler()处理SIGIO信号
    fcntl(fd, F_SETOWN, getpid());
    oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, oflags | FASYNC);
    while(1)
    {
    	sleep(100);
    }
	
  }
  else
  {
    printf("device open failure\n");
  }
}

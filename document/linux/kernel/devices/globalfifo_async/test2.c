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
/*œÓÊÕµœÒì²œ¶ÁÐÅºÅºóµÄ¶¯×÷*/
void input_handler(int signum)
{
  printf("receive a signal from globalfifo,signalnum:%d\n",signum);
}

main()
{
  int fd, oflags,num,i=0;
  char temp[]="wenxusssssllllljj";
  char buf[20];
  fd_set wfds;
  fd = open("/dev/globafifo", O_RDWR);
  if (fd !=  - 1)
  {
    //Æô¶¯ÐÅºÅÇý¶¯»úÖÆ
   /* signal(SIGIO, input_handler); //ÈÃinput_handler()ŽŠÀíSIGIOÐÅºÅ
    fcntl(fd, F_SETOWN, getpid());
    oflags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, oflags | FASYNC);
    while(1)
    {
    	sleep(100);
    }*/while(i<10)
	{
	sprintf(buf,"%s %d",temp,i);
	num=write(fd,buf,sizeof(buf));
	if(num<0)
	{
	perror("write bad");
	exit(1);
	}	
	printf("write the content is :: %s \n",buf);
	i++;
	sleep(2);	
	}
	i=0;
	while(i<10)
	{
	FD_ZERO(&wfds);
	FD_SET(fd,&wfds);

	select(fd+1,NULL,&wfds,NULL,NULL);
	if(FD_ISSET(fd,&wfds))
	{	
		sprintf(buf,"%s%d",temp,i+10);
		printf("you can write now\n");
		num=write(fd,buf,sizeof(buf));
		if(num<0)
		{
		perror("write bad\n");
		exit(1);
		}	
		printf("the content is :: %s \n",buf);
		sleep(1);	
		i++;
	}
	}
	
	i=0;
	while(i<10)
	{
	sprintf(buf,"%s%d",temp,i+20);
	num=write(fd,buf,sizeof(buf));
	if(num<0)
	{
	perror("write bad");
	exit(1);
	}	
	printf("write the content is :: %s \n",buf);
	i++;
	sleep(1);	
	}
	
  }
  
  else
  {
    printf("device open failure\n");
  }
}

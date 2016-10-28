#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>

int main()
{
 char buf[]="hello,this is a test program!\n";
 int fd;
 int ret;
 fd=open("/home/CR7/Program/linux_kernel/init_open/file",O_RDWR|O_NONBLOCK);
 
 ret=write(fd,buf,sizeof(buf)); 
 return 0;

}

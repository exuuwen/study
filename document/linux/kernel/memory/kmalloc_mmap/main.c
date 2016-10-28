#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#define PAGESIZE    4096
#define PAGENUM	    1
#define BUF_SIZE	(PAGESIZE*PAGENUM)
int main(void)
{
	int fd ,i;
	char * read_addr;
	fd=open("/dev/globalmem",O_RDWR);
	if(fd<0)
	{
		perror("open error hahah");
		exit(1);
	}
	read_addr=mmap(0,BUF_SIZE,PROT_READ|PROT_WRITE, MAP_SHARED, fd,0);
	if(read_addr == MAP_FAILED)
	{
  		printf("mmap failed\n");
		if(errno==EBADF)
		printf("bad fd \n");
		else  if(errno==EACCES)
		printf("acces error \n");
		else  if(errno==EINVAL)
		printf("chan shu  start length offset error \n");
		else  if(errno==EAGAIN)
		printf("file suozhu \n");
		else  if(errno==ENOMEM)
		printf("no mem error \n");
		else  if(errno==ENFILE)
		printf("file ENFILE \n");
		else  if(errno==EPERM  )
		printf("file EPERM   \n");
		else  if(errno==ENODEV  )
		printf("file ENODEV  \n");
		else  if(errno==ETXTBSY )
		printf("file ETXTBSY  \n");
		printf("error is %s\n",strerror(errno));
		return 0;
	}	
	 else
		printf("mmap success\n");
	printf("the  addr is %lx\n",read_addr);	//for(i=0;i<10;i++)
	printf("the kernel data is %s\n",read_addr);
	strcpy(read_addr,"hhahhasu ok jsbshsyys\n");
        munmap(read_addr,BUF_SIZE);
        close(fd);
        return 0;
}


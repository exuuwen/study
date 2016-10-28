#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


int main()
{
	int fd;
	char *write_addr;
	char str1[]="hello chenshuo\n";

	fd=open("/home/gupeng/mmap_sharefile/share_file",O_RDWR|O_NONBLOCK);

	ftruncate(fd, 4096);//没有这行会Bus error


	write_addr=mmap(0,4096,PROT_READ|PROT_WRITE, MAP_SHARED, fd,0);

	memcpy(write_addr,str1,sizeof(str1));

	printf("the content writed to the file is : %s",write_addr);

	sleep(5);

	printf("the content read from the file is : %s",write_addr);


}



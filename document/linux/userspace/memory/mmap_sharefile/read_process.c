#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


int main()
{
	int fd;
	char *read_addr;
	char str2[]="is you?\n";
	
	fd=open("/home/gupeng/mmap_sharefile/share_file",O_RDWR|O_NONBLOCK);

	ftruncate(fd, 4096);//没有这行会Bus error

	read_addr=mmap(0,4096,PROT_READ|PROT_WRITE, MAP_SHARED, fd,0);

	//memcpy(write_addr,str1,sizeof(str1));

	printf("the content read from the file is : %s",read_addr);

	memcpy(read_addr+15,str2,sizeof(str2));


}


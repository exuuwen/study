#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

char *data = "hellossssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";

int main(void)
{
	int fd, ret = 0;
	char *vadr;

	if ((fd = open("a.txt", O_RDONLY)) < 0) //must be have read for prot_read and port_write
	{
		perror("open");
		ret = -1;
		goto fail;
	}

	ftruncate(fd, 1024*1024);

	vadr = mmap(0, 1024*1024, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 1024*4);// map_private is not share the file with others, and it will not write back to the file.
	
	if (vadr == MAP_FAILED) {
		perror("mmap");
		ret = -1;
		goto fail_close;
	}
	
	memcpy(vadr, data, strlen(data));

	printf("%s\n", vadr);

	
	//scanf("%d\n", &a);
	if (-1 == munmap(vadr, 1024*1024))
		ret = -1;
fail_close:
	close(fd);
fail:
	exit(ret);
}


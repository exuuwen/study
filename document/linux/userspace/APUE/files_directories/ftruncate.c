#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
/*
If the previous size of the file was greater than length,
the data beyond length is no longer accessible. 
If the previous size was less than length, 
*/
int main(int argc, char *argv[])
{
	int ret;
	int fd;
	int len;

	if (argc != 3)
	{
		printf("usage: a.out <pathname> len");
		assert(argc == 3);	
	}

	fd = open(argv[1], O_RDWR);
	assert(fd >= 0);

	len = atoi(argv[2]);
	printf("len is %d\n", len);

	ret = ftruncate(fd, len);
	assert(ret == 0);

	printf("ftruncate ok\n");

	close(fd);
	
	return 0;
}

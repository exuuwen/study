#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define LEN (10*4096)

int main(void)
{
	int fd, ret = 0;
	char *vadr;

	if ((fd = open("/dev/mapdrv1", O_RDWR)) < 0) {
		perror("open");
		ret = -1;
		goto fail;
	}
	vadr = mmap(0, 3*4096, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 5*4096);

	if (vadr == MAP_FAILED) {
		perror("mmap");
		ret = -1;
		goto fail_close;
	}
	memcpy(vadr, "hello", sizeof("hello"));

	printf("%s\n", vadr);
	if (-1 == munmap(vadr, 3*4096))
		ret = -1;
fail_close:
	close(fd);
fail:
	exit(ret);
}

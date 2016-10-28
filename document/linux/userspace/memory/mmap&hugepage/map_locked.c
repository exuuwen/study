#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 1024*50

int main(void)
{
	int fd, ret = 0;
	char *vadr;

	printf("pid %d\n", getpid());

	scanf("%d", &fd);
	printf("fd %d\n", fd);

	vadr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_LOCKED | MAP_SHARED | MAP_ANONYMOUS, 0, 0)
	/*the size should be limited, the same as MAP_PRIVATE and with file(fd). 
	the offset is meanfuless for MAP_SHARE in anonymous mode,
	allocate the page with mmap and there is no pagefault*/
	if (vadr == MAP_FAILED) {
		perror("mmap");
		ret = -1;
		goto fail;
	}

	scanf("%d", &fd);
	printf("fd %d\n", fd);

	*vadr = 'a';

	//printf("%s\n", vadr);

	if (-1 == munmap(vadr, SIZE))
		ret = -1;
fail:
	exit(ret);
}


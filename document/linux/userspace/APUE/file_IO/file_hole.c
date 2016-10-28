#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

/*
 $ ./a.out
    $ ls -l file.hole                  check its size
    -rw-r--r-- 1 sar          16394 Nov 25 01:01 file.hole
    $ od -c file.hole                  let's look at the actual contents
    0000000   a  b  c  d  e  f  g  h  i  j \0 \0 \0 \0 \0 \0
    0000020  \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0 \0
    *
    0040000   A  B  C  D  E  F  G  H  I  J
    0040012

*/

static char buf1[] = "abcdefghij";
static char buf2[] = "ABCDEFGHIJ";

int main(void)
{
	int fd;
	int ret;

	fd = creat("file.hole", 0660);
	assert(fd >= 0);

	ret = write(fd, buf1, 10);
	assert(ret == 10);

	ret = lseek(fd, 100, SEEK_SET);// can less than zero, but not -1;
	assert(ret != -1);

	ret = write(fd, buf2, 10);
	assert(ret == 10);

	ret = lseek(fd, 0, SEEK_CUR); //cheack the current offset
	assert(ret != -1);

	printf("finnally offset:%d\n", ret);

	exit(0);
}





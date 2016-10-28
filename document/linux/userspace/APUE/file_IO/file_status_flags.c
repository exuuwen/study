#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

/*
	$ ./a.out 0 < /dev/tty
     read only
     $ ./a.out 1 > temp.foo
     $ cat temp.foo
     write only
     $ ./a.out 2 2>>temp.foo
     write only, append
     $ ./a.out 5 5<>temp.foo
     read write

*/

int main(int argc, char *argv[])
{
	int val;
	
	assert(argc == 2);

	val = fcntl(atoi(argv[1]), F_GETFL, 0);
	assert(val >= 0);

	switch (val & O_ACCMODE) {
	case O_RDONLY:
		printf("read only\n");
		break;

	case O_WRONLY:
		printf("write only\n");
		break;

	case O_RDWR:
		printf("read write\n");
		break;

	default:
		printf("unknown access mode\n");
	}

	if (val & O_APPEND)
		printf(", append\n");
	if (val & O_NONBLOCK)
		printf(", nonblocking\n");
	#if defined(O_SYNC)
	if (val & O_SYNC)
		printf(", synchronous writes\n");
	#endif
	#if !defined(_POSIX_C_SOURCE) && defined(O_FSYNC)
	if (val & O_FSYNC)
		printf(", synchronous writes\n");
	#endif
	
	exit(0);
}


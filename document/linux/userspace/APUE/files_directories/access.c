#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

/*Just test for the real user id and real group id's accessiblilites of file
Even though a process might be set-user-ID to root, 
it could still want to verify that the real user can access a given file.
 */

int main(int argc, char *argv[])
{
	int ret;

	if (argc != 2)
	{
		printf("usage: a.out <pathname>");
		assert(argc == 2);	
	}

	if (access(argv[1], W_OK) < 0)
	{
		printf("access error for %s", argv[1]);
		assert(0);
	}
	else
		printf("read access OK\n");

	ret = open(argv[1], O_RDONLY);
	assert(ret >= 0);
	
	printf("open for reading OK\n");
	
	return 0;
}




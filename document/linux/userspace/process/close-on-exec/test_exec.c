#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(int argc, char **args)
{
	char buffer[10];
	int fd = atoi(args[1]);
	memset(buffer, 0, sizeof(buffer));
	ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
	if(bytes < 0)
	{
		perror("test_exec: read fail:");
		return -1;
	}
	else
	{
		printf("test_exec: read %ld, %s\n", bytes, buffer);
	}

	return 0;
}

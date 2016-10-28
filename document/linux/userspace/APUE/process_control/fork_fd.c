#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

/*
cr7@cr7-virtual-machine:~/test/APUE/process_control$ cat file 
helloworld
cr7@cr7-virtual-machine:~/test/APUE/process_control$ ./fork_fd 
child buf:hello
father buf:world
*/

int
main(void)
{
	int      fd;      /* automatic variable on the stack */
	pid_t    pid;
	int ret;
	char buf[5];

	fd = open("./file", O_RDONLY);
	assert(fd >= 0);

	if ((pid = fork()) < 0) 
	{
		perror("fork error");
		exit(1);
	} 
	else if (pid == 0)  // child 
	{     			
		ret = read(fd, buf, sizeof(buf));
		assert(ret >= 0);
		printf("child buf:%s\n", buf);
	}
	else 
	{
		wait(NULL);              // parent 
		ret = read(fd, buf, sizeof(buf));
		assert(ret >= 0);
		printf("father buf:%s\n", buf);
	}
	
	close(fd);
	exit(0);
}

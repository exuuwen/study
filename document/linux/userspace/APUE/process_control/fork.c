#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

/*
	Note the interaction of fork with the I/O functions in the program in Figure 8.1. Recall from Chapter 3 that the write function is not buffered. Because write is called before the fork, its data is written once to standard output. The standard I/O library, however, is buffered. Recall from Section 5.12 that standard output is line buffered if it's connected to a terminal device; otherwise, it's fully buffered. When we run the program interactively, we get only a single copy of the printf line, because the standard output buffer is flushed by the newline. But when we redirect standard output to a file, we get two copies of the printf line. In this second case, the printf before the fork is called once, but the line remains in the buffer when fork is called. This buffer is then copied into the child when the parent's data space is copied to the child. Both the parent and the child now have a standard I/O buffer with this line in it. The second printf, right before the exit, just appends its data to the existing buffer. When each process terminates, its copy of the buffer is finally flushed.
*/

/*
cr7@cr7-virtual-machine:~/test/APUE/process_control$ ./fork 
a write to stdout
before fork
pid = 17330, glob = 7, var = 89
pid = 17329, glob = 6, var = 88
cr7@cr7-virtual-machine:~/test/APUE/process_control$ ./fork > output
cr7@cr7-virtual-machine:~/test/APUE/process_control$ cat output 
a write to stdout
before fork
pid = 17332, glob = 7, var = 89
before fork
pid = 17331, glob = 6, var = 88
*/

int     glob = 6;       /* external variable in initialized data */
char    buf[] = "a write to stdout\n";

int
main(void)
{
	int       var;      /* automatic variable on the stack */
	pid_t     pid;
	int ret;

	var = 88;
	ret = write(STDOUT_FILENO, buf, sizeof(buf)-1);
	assert(ret == sizeof(buf)-1);
	printf("before fork\n");    /* we don't flush stdout */

	if ((pid = fork()) < 0) 
	{
		perror("fork error");
		exit(1);
	} 
	else if (pid == 0)  // child 
	{     			
		glob++;                 // modify variables 
		var++;
	}
	else 
	{
		sleep(2);               // parent 
	}
	
	printf("pid = %d, glob = %d, var = %d\n", getpid(), glob, var);
	exit(0);
}




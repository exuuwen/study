#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/*
cr7@cr7-virtual-machine:~/APUE/process_control$ ./_exit 
before vfork
cr7@cr7-virtual-machine:~/APUE/process_control$ ./_exit > ss
cr7@cr7-virtual-machine:~/APUE/process_control$ cat ss
cr7@cr7-virtual-machine:~/APUE/process_control$ 
*/

/*
Calling the exit function. This function is defined by ISO C and 
includes the calling of all exit handlers that have been registered 
by calling atexit and closing all standard I/O streams. fulsh the buffer.

_exit to provide a way for a process to terminate without running exit
 handlers or signal handlers. standard I/O streams are not flushed in linux
*/
int
main(void)
{
	int     var;        /* automatic variable on the stack */
	pid_t   pid;


	printf("before vfork\n");   /* we don't flush stdio */
	if ((pid = fork()) < 0)
	{
		perror("fork error");
		exit(1);
	} 
	else if(pid == 0)
	{      /* child */
		_exit(0);               /* child terminates */
	}
	else
	{
		wait(NULL);
		_exit(0);
	}
  
}




#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

/*
If either the fork fails or waitpid returns an error other than EINTR, system returns 1 with errno set to indicate the error.

If the exec fails, implying that the shell can't be executed, the return value is as if the shell had executed exit(127).

Otherwise, all three functionsfork, exec, and waitpidsucceed, and the return value from system is the termination status of the shell, in the format specified for waitpid.
*/

/*
cr7@cr7-virtual-machine:~/APUE/process_control$ ./system 
Tue Jun 26 11:36:09 CST 2012
normal termination, exit status = 0
sh: nosuchcommand: not found
normal termination, exit status = 127
cr7      tty7         2012-06-20 17:59 (:0)
cr7      pts/0        2012-06-20 17:59 (:0.0)
cr7      pts/1        2012-06-21 15:39 (:0.0)
normal termination, exit status = 44
*/
void pr_exit(int status)
{
	if (WIFEXITED(status))
		printf("normal termination, exit status = %d\n", WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		printf("abnormal termination, signal number = %d%s\n", WTERMSIG(status), WCOREDUMP(status) ? " (core file generated)" : "");
	else if (WIFSTOPPED(status))
		printf("child stopped, signal number = %d\n", WSTOPSIG(status));
}

int
main(void)
{
	int      status;

	if ((status = system("date")) < 0)
		perror("system() date error");
	pr_exit(status);

	if ((status = system("nosuchcommand")) < 0)
		perror("system() nosuchcommand error");
	pr_exit(status);

	if ((status = system("who; exit 44")) < 0)
		perror("system() who; exit 44 error");
	pr_exit(status);

	exit(0);
}




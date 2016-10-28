#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

/*
Since the process group is orphaned when the parent terminates, 
POSIX.1 requires that every process in the newly orphaned process 
group that is stopped (as our child is) be sent the hang-up signal 
(SIGHUP) followed by the continue signal (SIGCONT).

When the parent terminates, the child is orphaned, so the child's 
parent process ID becomes 1, the init process ID.

the orphan process is in the backgroud so can not read stdin
*/

static void
sig_hup(int signo)
{
    printf("SIGHUP received, pid = %d\n", getpid());
}

static void
pr_ids(char *name)
{
    printf("%s: pid = %d, ppid = %d, pgrp = %d, tpgrp = %d\n",
        name, getpid(), getppid(), getpgrp(), tcgetpgrp(STDIN_FILENO));
    fflush(stdout);
}

int
main(void)
{
	char     c;
	pid_t    pid;

	pr_ids("parent");
	if ((pid = fork()) < 0) 
	{
		perror("fork error");
		exit(1);
	} 
	else if (pid > 0) 
	{   /* parent */
		sleep(2);       /*sleep to let child stop itself */
		exit(0);        /* then parent exits */
	} 
	else {            /* child */
		pr_ids("child");
		signal(SIGHUP, sig_hup);    /* establish signal handler, to prove the orphan process will recieve the SIGHUP sig */
		kill(getpid(), SIGTSTP);    /* stop ourself, to prove the orphan process will recieve the SIGCONT sig */
		sleep(1);
		pr_ids("child");    /* prints only if we're continued, ppid should change to 1*/
		if (read(STDIN_FILENO, &c, 1) != 1)
			printf("read error from controlling TTY, errno = %d\n", errno);  /*the orphan process is in the backgroud so can not read stdin*/
		while(1)
		{
			printf("in the child\n");
			sleep(2);
		}
		exit(0);
	}
}




#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>

/*
cr7@cr7-virtual-machine:~/APUE/signal$ ./sigchld 
child1 pid:5072
child2 pid:5073
child pid = 5073 end
normal termination, exit status = 2
child pid = 5072 end
normal termination, exit status = 1
*/

static int signaled = 0;

void pr_exit(int status)
{
	if (WIFEXITED(status))
		printf("normal termination, exit status = %d\n", WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		printf("abnormal termination, signal number = %d%s\n", WTERMSIG(status), WCOREDUMP(status) ? " (core file generated)" : "");
	else if (WIFSTOPPED(status))
		printf("child stopped, signal number = %d\n", WSTOPSIG(status));
}

static void handler(int signo)   /* interrupts pause() */
{
	pid_t   pid;
	int     status;

	if ((pid = wait(&status)) < 0)      /* fetch child status */
		perror("wait error");
	printf("child pid = %d end\n", pid);
	pr_exit(status); 
}


int main()
{
	pid_t  pid;
	struct sigaction sigact;
	
	sigact.sa_handler = handler;
	sigact.sa_flags = 0;
	sigaction(SIGCHLD, &sigact, NULL);

	//signal(SIGCHLD, handler);
	
	if ((pid = fork()) < 0) 
	{
	        perror("fork error");
	exit(1);
    	} 
	else if (pid == 0) 
	{      /* child */
		printf("child1 pid:%d\n", getpid());
	        sleep(4);
	        _exit(1);
    	}

	if ((pid = fork()) < 0) 
	{
	        perror("fork error");
		exit(1);
    	} 
	else if (pid == 0) 
	{      /* child */
		printf("child2 pid:%d\n", getpid());
	        sleep(1);
	        _exit(2);
    	}
		
	while(1)
	{
		pause();    /* parent */
	}

	exit(0);
}

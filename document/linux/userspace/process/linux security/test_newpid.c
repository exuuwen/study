#include <stdio.h>
#include <sys/prctl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>

#ifndef CLONE_NEWPID
#define CLONE_NEWPID  0x20000000
#endif

void sig_handler(int sig)
{
	if(sig == SIGTERM)//It is the normal way to politely ask a program to terminate. send kill
		printf("SIGTERM recieved,exiting...\n");
	if(sig == SIGINT)//ignal is sent when the user types the INTR character (normally Ctrl-c).
		printf("SIGINT recieved,exiting...\n");
	else if(sig == SIGQUIT)//ignal is sent when the user types the INTR character (normally C-c).
		printf("SIGQUI Trecieved,exiting...\n");
	
	exit(0);
}

int do_newpidns(void)
{
	register pid_t pid, waited;
	int status;
	struct sigaction sighandle;

	sighandle.sa_flags = 0;
	sighandle.sa_handler = sig_handler;
	sigemptyset(&sighandle.sa_mask);

	sigaction(SIGTERM, &sighandle, NULL);
	sigaction(SIGINT, &sighandle, NULL);
	sigaction(SIGQUIT, &sighandle, NULL);

	pid = syscall(SYS_clone, CLONE_NEWPID | SIGCHLD, 0, 0, 0);

	switch (pid) 
	{
	
	case -1:
		perror("clone");
		return -1;

		/* child: we are pid number 1 in the new namespace */
	case 0:
		/* We add an extra check for old kernels because sys_clone() doesn't
		EINVAL on unknown flags */
		if (getpid() == 1)
		{
			
			return 0;
		}
		else
			return -1;

	default:
		/* Let's wait for our child */
		
		//printf("the  father  pid is %d\n",getpid());
        	//printf("the father's father is  %d\n",getppid());
		//printf("uid:%d, euid:%d, gid:%d, egid:%d\n", getuid(), geteuid(), getgid(), getegid());
		printf("in father child pid %d\n", pid);
		//kill(pid, SIGINT);
		
		printf("after kill in father\n");
		waited = waitpid(pid, &status, 0);
		if (waited != pid) 
		{
			perror("waitpid");
			exit(EXIT_FAILURE);
		} 
		else 
		{
			printf("child exit ok!\n");
			/* FIXME: we proxy the exit code, but only if the child terminated normally */
			if (WIFEXITED(status))
				exit(WEXITSTATUS(status));
			exit(EXIT_SUCCESS); 
		}
	}
}


int main()
{
	int ret;

	if (geteuid()) 
	{
		fprintf(stderr, "The sandbox is not seteuid root, aborting\n");
		return EXIT_FAILURE;
	}

	if (!getuid()) 
	{
		fprintf(stderr, "The sandbox is not designed to be run by root, aborting\n");
		return EXIT_FAILURE;
	}

	ret = do_newpidns();
	assert(ret == 0);

	printf("the child  pid is %d\n",getpid());
  	printf("the father's pid is  %d\n",getppid());
	printf("uid:%d, euid:%d, gid:%d, egid:%d\n", getuid(), geteuid(), getgid(), getegid());
	
	return 0;

     


}




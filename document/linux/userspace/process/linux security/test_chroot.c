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
#include <sys/capability.h>
#include <sched.h>

#define SAFE_DIR "/home/cr7/chroot"

int main(void)
{
	int ret;
	register pid_t pid;
	char *safedir=NULL;
	struct stat sdir_stat;
	register pid_t waited;
	int status;
	char *rets;
	char path[100];

	if (!stat(SAFE_DIR, &sdir_stat) && S_ISDIR(sdir_stat.st_mode))
	{
		safedir = SAFE_DIR;
		printf("safe_dir ok\n");	
	}
	else 
	{
		fprintf(stderr, "Helper: %s does not exist ?!\n", SAFE_DIR);
		return -1;
	}


	pid = syscall(SYS_clone, CLONE_FS | SIGCHLD, 0, 0, 0);

	switch (pid) 
	{
	// struct rlimit nf;

	case -1:
		perror("clone");
		return -1;

	/* child */
	case 0:
	/* We share our FS with an untrusted process
	* As a security in depth mesure, we make sure we can't open anything by mistake
	* We need to drop CAP_SYS_RESSOURCE or it's useless */
		/*  nf.rlim_cur = 0;
		nf.rlim_max = 0;
		if (setrlimit(RLIMIT_NOFILE, &nf)) {
		perror("Helper: setrlimit");
		exit(EXIT_FAILURE);
		}

		*/
		
		/* FIXME: change directory + check permissions first. */
		rets = getcwd(path, 100);
		if(rets)
			printf("before chroot path:%s\n", path);
		ret = chroot(safedir);
		if (ret) 
		{
			perror("Helper: chroot");
			exit(EXIT_FAILURE);
		}
		ret = chdir("/");
		if (ret) 
		{
			perror("Helper: chdir");
			exit(EXIT_FAILURE);
		}
		printf("Helper: I chrooted you\n");

		exit(EXIT_SUCCESS);

 	default:
		
		waited = waitpid(pid, &status, 0);
		if (waited != pid) 
		{
			perror("waitpid");
			exit(EXIT_FAILURE);
		} 
		else 
		{
			printf("child exit ok!\n");
			rets = getcwd(path, 100);
			if(rets)
				printf("in main after chroot path:%s\n", path);
			/* FIXME: we proxy the exit code, but only if the child terminated normally */
			/*if (WIFEXITED(status))
				exit(WEXITSTATUS(status));*/
			printf("here ok, run shell\n");
			
			fprintf(stderr, "Executing untrust\n");
			execv("./untrust", NULL);
			perror("exec failed");
			
			exit(EXIT_SUCCESS); 
		}

	}
}

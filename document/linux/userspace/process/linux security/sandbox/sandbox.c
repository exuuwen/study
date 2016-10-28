#include "sandbox.h"

static int set_capabilities(cap_value_t cap_list[], int ncap) 
{
	cap_t caps;
	int ret;

	/* caps should be initialized with all flags cleared... */
	caps = cap_init();
	if (!caps) 
	{
		perror("cap_init");
		return -1;
	}
	/* ... but we better rely on cap_clear */
	if (cap_clear(caps)) 
	{
		perror("cap_clear");
		return -1;
	}

	if ((cap_list) && ncap) 
	{
		if (cap_set_flag(caps, CAP_PERMITTED, ncap, cap_list, CAP_SET) != 0) 
		{
			perror("cap_set_flag CAP_PERMITTED");
			cap_free(caps);
			return -1;
		}
		if (cap_set_flag(caps, CAP_EFFECTIVE, ncap, cap_list, CAP_SET) != 0)
		{
			perror("cap_set_flag CAP_EFFECTIVE");
			cap_free(caps);
			return -1;
		}
		if (cap_set_flag(caps, CAP_INHERITABLE, ncap, cap_list, CAP_SET) != 0)
		{
			perror("cap_set_flag CAP_INHERITABLE");
			cap_free(caps);
			return -1;
		}
		
	}

	ret = cap_set_proc(caps);

	if (ret) 
	{
		perror("cap_set_proc");
		cap_free(caps);
		return -1;
	}

	if (cap_free(caps)) 
	{
		perror("cap_free");
		return -1;
	}

	return 0;
}

static int do_setuid(uid_t uid)
{

	/* We become explicitely non dumpable. Note that normally setuid() takes care
	* of this when we switch euid, but we want to support capability FS. 
	*/

	if (setresuid(uid, uid, uid))
	{
		return -1;
	}

	/* Drop all capabilities. Again, setuid() normally takes care of this if we had
	* euid 0
	*/ 
	if (set_capabilities(NULL, 0)) 
	{
		return -1;
	}

	return 0;
}

static int do_newpidns(void)
{
	register pid_t pid, waited;
	int status;
	uid_t olduid;

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
			return 0;
		else
			return -1;

	default:
		/* Let's wait for our child */
		olduid = getuid();
		do_setuid(olduid);
		waited = waitpid(pid, &status, 0);
		if (waited != pid) 
		{
			perror("waitpid");
			exit(EXIT_FAILURE);
		} 
		else 
		{
			/* FIXME: we proxy the exit code, but only if the child terminated normally */
			printf("father wait ok\n");
			if (WIFEXITED(status))
				exit(WEXITSTATUS(status));
			exit(EXIT_SUCCESS); 
		}
	}
}


int do_chroot(void)
{
	int ret;
	register pid_t pid;
	char *safedir=NULL;
	struct stat sdir_stat;
	register pid_t waited;
	int status;

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
	case -1:
		perror("clone");
		return -1;

	/* child */
	case 0:

		/* FIXME: change directory + check permissions first. */
		ret = chroot(safedir);
		if (ret) 
		{
			perror("Helper: chroot");
			exit(EXIT_FAILURE);
		}
		
		ret = chdir("/");
		
		printf("Helper:chrooted ok\n");

		exit(EXIT_SUCCESS);

 	default:
		
		waited = waitpid(pid, &status, 0);
		if (waited != pid) 
		{
			perror("waitpid");
			return -1;
		} 
		else 
		{
			printf("child exit ok!\n");
			return 0;
		}

	}
}

static int do_seccomp(struct syscall_filter *filter)
{
	int ret;
	
	ret = prctl(PR_ATTACH_SECCOMP_FILTER, filter, 0, 0, 0);
	
	return ret;
}

static int do_getconfig(unsigned char *newpid, unsigned char *chroot, unsigned char *normal_user, struct syscall_filter *syscalls)
{
	int ret = 0;
 
	*newpid = 1;
	*chroot = 1;
	*normal_user = 0;
	
	syscalls->count = 16;
	syscalls->syscall_list = model_syscalls;

	return ret;
}


int main(int argc, char *const argv[])
{
	int ret;
	char* rets = NULL;
	char path[100];
	cap_value_t cap_list[3];
	unsigned char newpid, chroot, normal_user;
	struct syscall_filter syscalls;
	uid_t olduid, ruid, euid, suid;

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
	
	  /* capabilities we need */
 	cap_list[0] = CAP_SETUID;
 	cap_list[1] = CAP_SYS_ADMIN;  /* for CLONE_NEWPID */
  	cap_list[2] = CAP_SYS_CHROOT;

	if (set_capabilities(cap_list, sizeof(cap_list)/sizeof(cap_list[0]))) 
	{
		fprintf(stderr, "Could not adjust capabilities, aborting\n");
		return EXIT_FAILURE;
	}

	ret = do_getconfig(&newpid, &chroot, &normal_user, &syscalls);
	assert(ret == 0);

	if(newpid)
	{
		ret = do_newpidns();
		assert(ret == 0);	
	}

	if(chroot)
	{
		ret = do_chroot();
		assert(ret == 0);		
	}
	
	if(normal_user)
	{
		olduid = getuid();
		ret = do_setuid(olduid);	
		assert(ret == 0);		
	}

	
	getresuid(&ruid, &euid, &suid);
	printf("in child I'm pid=%d, uid=%d, euid=%d, rudi=%d, suid=%d\n", getpid(), getuid(), euid, ruid, suid);
	rets = getcwd(path, 100);
	if(rets)
		printf("in main after chroot path:%s\n", path);

	if(syscalls.count)
	{
		ret = do_seccomp(&syscalls);
		assert(ret == 0);
	}
	char *exec_argv[2];
	exec_argv[0] = "untrust";
	exec_argv[1] = NULL;
	if(execv("./untrust", exec_argv) < 0)
	{
                perror("Err on execl");
		return EXIT_FAILURE;	
	}
		

	return 0;
	

}

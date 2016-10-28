#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <assert.h>
#include <linux/securebits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define _GNU_SOURCE
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#include <sys/prctl.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>

int do_setuid(uid_t uid)
{

	/* We become explicitely non dumpable. Note that normally setuid() takes care
	* of this when we switch euid, but we want to support capability FS. 
	*/

	if ( setresuid(uid, uid, uid)) 
	{
		return -1;
	}

	/* Drop all capabilities. Again, setuid() normally takes care of this if we had
	* euid 0
	*/ 
	/*if (set_capabilities(NULL, 0)) 
	{
		return -1;
	}*/

	return 0;
}


int main(int argc, char *const argv[], char *const envp[])
{
	uid_t olduid, ruid, euid, suid;
	//struct passwd *sbxuser;
	int ch, ret = -1;

	/*if (geteuid()) 
	{
		fprintf(stderr, "The sandbox is not seteuid root, aborting\n");
		return EXIT_FAILURE;
	}

	if (!getuid()) 
	{
		fprintf(stderr, "The sandbox is not designed to be run by root, aborting\n");
		return EXIT_FAILURE;
	}*/
	
	getresuid(&ruid, &euid, &suid);
	fprintf(stderr,"first Hi from the sandbox! I'm pid=%d, uid=%d, euid=%d, rudi=%d, suid=%d\n",
	getpid(), getuid(), geteuid(), ruid, suid);

	olduid = getuid();

	//ret = do_setuid(olduid);
	ret = setresuid(1000, 10, 5000);//setreuid(0, 2000);
	/* could not switch uid */
	if (ret) 
	{
		fprintf(stderr, "Could not properly drop privileges\n");
		return EXIT_FAILURE;
	}

	getresuid(&ruid, &euid, &suid);
	fprintf(stderr,"dorp Hi from the sandbox! I'm pid=%d, uid=%d, euid=%d, rudi=%d, suid=%d\n",
	getpid(), getuid(), geteuid(), ruid, suid);

	ret = setuid(5000);//setreuid(5000, 10, 10);

	/* could not switch uid */
	if (ret) 
	{
		fprintf(stderr, "Could not properly regain privileges\n");
		return EXIT_FAILURE;
	}

	getresuid(&ruid, &euid, &suid);
	fprintf(stderr,"regain Hi from the sandbox! I'm pid=%d, uid=%d, euid=%d, rudi=%d, suid=%d\n",
	getpid(), getuid(), geteuid(), ruid, suid);

	/* if(execv("/home/cr7/seccomp/untrust", NULL) < 0)
	{
                perror("Err on execl");
		return EXIT_FAILURE;	
	}*/

	return 0;
	
}

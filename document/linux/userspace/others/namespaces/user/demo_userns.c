/* demo_userns.c

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   Demonstrate the use of the clone() CLONE_NEWUSER flag.

   Link with "-lcap" and make sure that the "libcap-devel" (or
   similar) package is installed on the system.
*/
#define _GNU_SOURCE
#include <sys/capability.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <net/ethernet.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static int                      /* Startup function for cloned child */
childFunc(void *arg)
{
    cap_t caps;

    for (;;)
	 {
        printf("eUID = %ld;  eGID = %ld;  ",
                (long) geteuid(), (long) getegid());

        caps = cap_get_proc();
        printf("capabilities: %s\n", cap_to_text(caps, NULL));

        if (arg == NULL)
        	break;

		/*int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
		if (sock == -1)
			errExit("sock");*/  //fail
	
		/*int fd = open("a", O_RDONLY);
		if (fd < 0)
			errExit("open");
		*/  //success if the parent is root
        sleep(5);
    }

    return 0;
}

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */

int
main(int argc, char *argv[])
{
    pid_t pid;

    /* Create child; child commences execution in childFunc() */

    pid = clone(childFunc, child_stack + STACK_SIZE,    /* Assume stack
                                                           grows downward */
                CLONE_NEWUSER | SIGCHLD, argv[1]);
    if (pid == -1)
        errExit("clone");

	 printf("feUID = %ld;  feGID = %ld;  ",
                (long) geteuid(), (long) getegid());
	cap_t caps = cap_get_proc();
        printf("fcapabilities: %s\n", cap_to_text(caps, NULL));


    /* Parent falls through to here.  Wait for child. */

    if (waitpid(pid, NULL, 0) == -1)
        errExit("waitpid");

    exit(EXIT_SUCCESS);
}

/* userns_setns_test.c

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   Open a /proc/PID/ns/user namespace file specified on the command
   line, and then create a child process in a new user namespace.
   Both processes then try to setns() into the namespace identified
   on the command line.  The setns() system call requires
   CAP_SYS_ADMIN in the target namespace.
*/
#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

/* Try to join the user namespace identified by the file
   descriptor 'fd'. 'pname' is a per-process string that
   the caller can use to distinguish information maessages
   displayed by this function */

static void
test_setns(char *pname, int fd)
{
    char path[PATH_MAX];
    ssize_t s;

    /* Display caller's user namespace ID */

    s = readlink("/proc/self/ns/user", path, PATH_MAX);
    if (s == -1)
        errExit("readlink");

    printf("%s readlink(\"/proc/self/ns/user\") ==> %s\n", pname, path);

    /* Attempt to join the user namespace specified by 'fd' */

    if (/*setns(fd, CLONE_NEWNS)*/syscall(308, fd, 0) == -1)
        printf("%s setns() failed: %s\n", pname, strerror(errno));
    else
        printf("%s setns() succeeded\n", pname);
}

static int              /* Start function for cloned child */
childFunc(void *arg)
{
    long fd = (long) arg;

    usleep(100000);     /* Avoid intermingling with parent's output */

    /* Test whether setns() is possible from the child user namespace */

    test_setns("child: ", fd);

    return 0;
}

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */

int
main(int argc, char *argv[])
{
    pid_t child_pid;
    long fd;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s /proc/PID/ns/user]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open user namespace file specified on command line */

    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        errExit("open");

    /* Create child process in new user namespace */

    child_pid = clone(childFunc, child_stack + STACK_SIZE,
                      CLONE_NEWUSER | SIGCHLD, (void *) fd);
    if (child_pid == -1)
        errExit("clone");

    /* Test whether setns() is possible from the parent user namespace */

    test_setns("parent:", fd);
    printf("\n");

    if (waitpid(child_pid, NULL, 0) == -1)      /* Wait for child */
        errExit("waitpid");

    exit(EXIT_SUCCESS);
}

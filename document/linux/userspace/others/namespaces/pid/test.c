/* multi_pidns.c

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   Create a series of child processes in nested PID namespaces.
*/
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <limits.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
cr7@ubuntu:~/test/tmp/namespaces/pid$ sudo ./test 
Mounting procfs at /proc4
Mounting procfs at /proc3
Mounting procfs at /proc2
Mounting procfs at /proc1
Mounting procfs at /proc0
Final child sleeping
^Z
[1]+  Stopped                 sudo ./test
cr7@ubuntu:~/test/tmp/namespaces/pid$ ls -d /proc4/[1-9]*
/proc4/1  /proc4/2  /proc4/3  /proc4/4  /proc4/5
cr7@ubuntu:~/test/tmp/namespaces/pid$ ls -d /proc3/[1-9]*
/proc3/1  /proc3/2  /proc3/3  /proc3/4
cr7@ubuntu:~/test/tmp/namespaces/pid$ ls -d /proc2/[1-9]*
/proc2/1  /proc2/2  /proc2/3
cr7@ubuntu:~/test/tmp/namespaces/pid$ ls -d /proc1/[1-9]*
/proc1/1  /proc1/2
cr7@ubuntu:~/test/tmp/namespaces/pid$ ls -d /proc0/[1-9]*
/proc0/1
*/
//cr7@ubuntu:~/test/tmp/namespaces/pid$ grep -H 'Name:.*sleep' /proc?/[1-9]*/status
/*
/proc0/1/status:Name:	sleep
/proc1/2/status:Name:	sleep
/proc2/3/status:Name:	sleep
/proc3/4/status:Name:	sleep
/proc4/5/status:Name:	sleep

$ ps -e
 8376 pts/1    00:00:00 test
 8377 pts/1    00:00:00 test
 8378 pts/1    00:00:00 test
 8379 pts/1    00:00:00 test
 8380 pts/1    00:00:00 test
 8381 pts/1    00:00:00 sleep
 8398 pts/1    00:00:00 ps

cr7@ubuntu:~/test/tmp/namespaces/pid$ sudo readlink /proc/8376/ns/pid
pid:[4026531836]
cr7@ubuntu:~/test/tmp/namespaces/pid$ sudo readlink /proc/8377/ns/pid
pid:[4026532484]
cr7@ubuntu:~/test/tmp/namespaces/pid$ sudo readlink /proc/8378/ns/pid
pid:[4026532485]
cr7@ubuntu:~/test/tmp/namespaces/pid$ sudo readlink /proc/8379/ns/pid
pid:[4026532486]
cr7@ubuntu:~/test/tmp/namespaces/pid$ sudo readlink /proc/8380/ns/pid
pid:[4026532487]
cr7@ubuntu:~/test/tmp/namespaces/pid$ sudo readlink /proc/8381/ns/pid
pid:[4026532488]

*/

/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */
                /* Since each child gets a copy of virtual memory, this
                   buffer can be reused as each child creates its child */

/* Recursively create a series of child process in nested PID namespaces.
   'arg' is an integer that counts down to 0 during the recursion.
   When the counter reaches 0, recursion stops and the tail child
   executes the sleep(1) program. */

static int
childFunc(void *arg)
{
    static int first_call = 1;
    long level = (long) arg;

    if (!first_call) {

        /* Unless this is the first recursive call to childFunc()
           (i.e., we were invoked from main()), mount a procfs
           for the current PID namespace */

        char mount_point[PATH_MAX];

        snprintf(mount_point, PATH_MAX, "/proc%c", (char) ('0' + level));

        mkdir(mount_point, 0555);       /* Create directory for mount point */
        if (mount("proc", mount_point, "proc", 0, NULL) == -1)
            errExit("mount");
        printf("Mounting procfs at %s\n", mount_point);
    }

    first_call = 0;

    if (level > 0) {

        /* Recursively invoke childFunc() to create another child in a
           nested PID namespace */

        level--;
        pid_t child_pid;

        child_pid = clone(childFunc,
                    child_stack + STACK_SIZE,   /* Points to start of
                                                   downwardly growing stack */
                    CLONE_NEWPID | SIGCHLD, (void *) level);

        if (child_pid == -1)
            errExit("clone");

        if (waitpid(child_pid, NULL, 0) == -1)  /* Wait for child */
            errExit("waitpid");

    } else {

        /* Tail end of recursion: execute sleep(1) */

        printf("Final child sleeping\n");
        execlp("sleep", "sleep", "1000", (char *) NULL);
        errExit("execlp");
    }

    return 0;
}

int
main(int argc, char *argv[])
{
    long levels;

    levels = (argc > 1) ? atoi(argv[1]) : 5;
    childFunc((void *) levels);

    exit(EXIT_SUCCESS);
}

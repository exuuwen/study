/* orphan.c

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   Demonstrate that a child becomes orphaned (and is adopted by init(1),
   whose PID is 1) when its parent exits.
*/
#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <wordexp.h>
#include <errno.h>
#include <sys/wait.h>


int
main(int argc, char *argv[])
{
    pid_t pid;

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid != 0) {             /* Parent */
        printf("Parent (PID=%ld) created child with PID %ld\n",
                (long) getpid(), (long) pid);
        printf("Parent (PID=%ld; PPID=%ld) terminating\n",
                (long) getpid(), (long) getppid());
        exit(EXIT_SUCCESS);
    }

    /* Child falls through to here */

    do {
        usleep(100000);
    } while (getppid() != 1);           /* Am I an orphan yet? */


    printf("\nChild  (PID=%ld) now an orphan (parent PID=%ld)\n",
            (long) getpid(), (long) getppid());

    sleep(1);

    printf("Child  (PID=%ld) terminating\n", (long) getpid());
    exit(EXIT_SUCCESS);
}

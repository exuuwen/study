/* userns_child_exec.c

   Copyright 2013, Michael Kerrisk
   Licensed under GNU General Public License v2 or later

   Create a child process that executes a shell command in new
   namespace(s); allow UID and GID mappings to be specified when
   creating a user namespace.
*/
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "rtnetlink_addr.h"
#include "rtnetlink_route.h"
#include "rtnetlink_link_veth.h"

#include "if_ops.h"
#include "bridge.h"
//#include "tun_tap.h"


/* A simple error-handling function: print an error message based
   on the value in 'errno' and terminate the calling process */

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static int pipe_fd[2];  /* Pipe used to synchronize parent and child */
static int verbose = 1;
static int client_done = 0;
static uid_t uid;
static int nid;

/* Update the mapping file 'map_file', with the value provided in
   'mapping', a string that defines a UID or GID mapping. A UID or
   GID mapping consists of one or more newline-delimited records
   of the form:

       ID_inside-ns    ID-outside-ns   length

   Requiring the user to supply a string that contains newlines is
   of course inconvenient for command-line use. Thus, we permit the
   use of commas to delimit records in this string, and replace them
   with newlines before writing the string to the file. */
/*
static void update_map(char *mapping, char *map_file)
{
    int fd, j;
    size_t map_len;     

    // Replace commas in mapping string with newlines 

    map_len = strlen(mapping);
    for (j = 0; j < map_len; j++)
        if (mapping[j] == ',')
            mapping[j] = '\n';

    fd = open(map_file, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "open %s: %s\n", map_file, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (write(fd, mapping, map_len) != map_len) {
        fprintf(stderr, "write %s: %s\n", map_file, strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(fd);
}
*/
/* Start function for cloned child */
static int childFunc(void *arg)          
{
    //struct child_args *args = (struct child_args *) arg;
    char ch;

    printf("uid 0x%d\n", uid);
    /* Wait until the parent has updated the UID and GID mappings. See
       the comment in main(). We wait for end of file on a pipe that will
       be closed by the parent process once it has updated the mappings. */  
    //printf("child euid %ld\n", (long) geteuid());
    close(pipe_fd[1]);    /* Close our descriptor for the write end
                                   of the pipe so that we see EOF when
                                   parent closes its descriptor */
    if (read(pipe_fd[0], &ch, 1) != 0) 
    {
        perror("child pipe");
        exit(EXIT_FAILURE);
    }

    char nboxname[20];
    memset(nboxname, 0, sizeof(nboxname));
    sprintf(nboxname, "nbox-%d", nid);

    if (sethostname(nboxname, strlen(nboxname)) == -1)
        errExit("sethostname");
    if (verbose)
        printf("sethostname to %s\n", nboxname);

    if (mount("proc", "/proc", "proc", 0, NULL) == -1)
        errExit("mount");
    if (verbose)
        printf("Mounting procfs at %s\n", "/proc");

    if (if_up("eth0") < 0)
        errExit("if up eth0");
    if (if_up("lo") < 0)
        errExit("if up eth0");

    char nboxaddr[20];
    memset(nboxaddr, 0, sizeof(nboxaddr));
    sprintf(nboxaddr, "10.0.%d.7/24", nid);

    if (create_addr("eth0", nboxaddr) < 0)
        errExit("create_addr eth0");
    if (verbose)
        printf("create_addr %s on eth0\n", nboxaddr);

    char raddr[20];
    memset(raddr, 0, sizeof(raddr));
    sprintf(raddr, "10.0.%d.1", nid);

    if (create_route(raddr, NULL, NULL) < 0)
        errExit("fail create route");
    if (verbose)
        printf("create default route with gw 10.0.%d.1 through eth0\n", nid);
 
    system("/usr/sbin/sshd");

    while (1) 
    {
        waitpid(-1, NULL, WNOHANG);
        if (client_done) 
        {
            exit(0);
        }
        sleep(1);
     }
}

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */

int main(int argc, char *argv[])
{
    int flags, opt;
    pid_t child_pid;
    //gid_t egid;
    char ch;
    //char gid_map[PATH_MAX];
    //char uid_map[PATH_MAX];
    //char map_path[PATH_MAX];
    
    /* Parse command-line options. The initial '+' character in
       the final getopt() argument prevents GNU-style permutation
       of command-line options. That's useful, since sometimes
       the 'command' to be executed by this program itself
       has command-line options. We don't want getopt() to treat
       those as options to this program. */
    if (argc < 2)
    {
        printf("%s index\n", argv[0]);
        return 0;
    }

    uid = getuid();
    if (uid == 0)
    {
        printf("this program shouldn't be running as root\n");
        return 0;
    }

    
    nid = atoi(argv[1]);

    char nbox_veth[20];
    memset(nbox_veth, 0, sizeof(nbox_veth));
    sprintf(nbox_veth, "nbox%d", nid);
    
    if (if_index(nbox_veth) >= 0)
    {
        printf("user %d already create a nbox with id %d\n", uid, nid);
        return 0; 
    }
    
    flags = CLONE_VM | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWNET | CLONE_NEWPID 
             | CLONE_NEWUTS /*| CLONE_NEWUSER*/;

    /* We use a pipe to synchronize the parent and child, in order to
       ensure that the parent sets the UID and GID maps before the child
       calls execve(). This ensures that the child maintains its
       capabilities during the execve() in the common case where we
       want to map the child's effective user ID to 0 in the new user
       namespace. Without this synchronization, the child would lose
       its capabilities if it performed an execve() with nonzero
       user IDs (see the capabilities(7) man page for details of the
       transformation of a process's capabilities during execve()). */

    if (pipe(pipe_fd) == -1)
        errExit("pipe");

    /* Create the child in new namespace(s) */

    child_pid = clone(childFunc, child_stack + STACK_SIZE, flags | SIGCHLD, NULL);
    if (child_pid == -1)
        errExit("clone");

    /* Parent falls through to here */

    if (verbose)
        printf("%s: PID of child created by clone() is %ld\n", argv[0], (long) child_pid);

    /* Update the UID and GID maps in the child */
    /*euid = geteuid();
    
    //snprintf(uid_map, PATH_MAX, "%ld %ld 1",  (long) euid, (long) euid);
    snprintf(uid_map, PATH_MAX, "0 %ld 1", (long) euid);
    snprintf(map_path, PATH_MAX, "/proc/%ld/uid_map", (long) child_pid);
    update_map(uid_map, map_path);
    
    egid = getegid();
    //snprintf(gid_map, PATH_MAX, "%ld %ld 1",  (long) egid, (long) egid);
    snprintf(gid_map, PATH_MAX, "0 %ld 1", (long) egid);
    snprintf(map_path, PATH_MAX, "/proc/%ld/gid_map", (long) child_pid);
    update_map(gid_map, map_path);
    
    if (verbose)
        printf("uid_map %s, gid_map %s\n", uid_map, gid_map);*/

    if (create_veth_link(nbox_veth, "eth0", child_pid) < 0)
        errExit("nbox veth link");
    if (verbose)
        printf("nbox veth link create\n");

    if (if_up(nbox_veth))
        errExit("nbox veth link up");
    if (verbose)
        printf("nbox veth link up\n");

    char addr[20];
    memset(addr, 0, sizeof(addr));
    sprintf(addr, "10.0.%d.1/24", nid);

    if (create_addr(nbox_veth, addr) < 0)
        errExit("create_addr nbox_veth");
    if (verbose)
        printf("create_addr %s on nbox_veth\n", addr);

    /* Close the write end of the pipe, to signal to the child that we
       have done basic things */

    seteuid(uid);
    close(pipe_fd[1]);

    sleep(1);

    char ssh_con[30];
    memset(ssh_con, 0, sizeof(ssh_con));
    sprintf(ssh_con, "ssh -X 10.0.%d.7", nid);
    system(ssh_con);

    client_done = 1;
    if (waitpid(child_pid, NULL, 0) == -1)      /* Wait for child */
        errExit("waitpid");

    if (verbose)
        printf("%s: terminating\n", argv[0]);

    exit(EXIT_SUCCESS);
}


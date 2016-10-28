#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/* SIGQUIT handler */
static void sig_quit(int signo)
{
    printf("SIGINT is caught \n");
}

int main()
{
    sigset_t new, old, pend;

    /* Handle SIGQUIT */
    if (signal(SIGINT, sig_quit) == SIG_ERR)
    {
        perror("signal");
        exit(1);
    }

    /* Add SIGQUIT to sigset */
    if (sigemptyset(&new) < 0)
        perror("sigemptyset");
    if (sigaddset(&new, SIGINT) < 0)
        perror("sigaddset");

    /* Mask SIGQUIT */
    if (sigprocmask(SIG_SETMASK, &new, &old) < 0)
    {
        perror("sigprocmask");
        exit(1);
    }

    printf("SIGQUIT is blocked ");
    printf("Now try Ctrl \n");

    sleep(10); /* SIGQUIT will pending */

    /* Get pending */
    if (sigpending(&pend) < 0)
        perror("sigpending");

    if (sigismember(&pend, SIGINT))
        printf("SIGINT pending \n");

    /* Restore signal mask */
    if (sigprocmask(SIG_SETMASK, &old, NULL) < 0)
    {
        perror("sigprocmask");
        exit(1);
    }

    printf(" SIGINT unblocked\n ");
    //printf("Now try Ctrl \n");

    sleep(10);

    return 0;
}


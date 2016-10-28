#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include   <setjmp.h>

static jmp_buf  env_alrm;

static void sig_alrm(int signo)
{
    longjmp(env_alrm, 1);
}

unsigned int
sleep2(unsigned int nsecs)
{
    if (signal(SIGALRM, sig_alrm) == SIG_ERR)
        return(nsecs);
    if (setjmp(env_alrm) == 0) {
        alarm(nsecs);       /* start the timer */
        pause();            /* next caught signal wakes us up */
    }
    return(alarm(0));       /* turn off timer, return unslept time */
}


int main(void)
{
	unsigned int        unslept;

	printf("start sleep 5 mins\n");
	unslept = sleep2(5);
	assert(unslept == 0);
	printf("sleep2 returned: %u\n", unslept);
	exit(0);
}



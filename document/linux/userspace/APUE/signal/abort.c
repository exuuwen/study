#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <signal.h>
#include  <setjmp.h>

/*
POSIX.1 also specifies that abort overrides the blocking 
or ignoring of the signal by the process. If the process 
doesn't terminate itself from this signal handler, POSIX.1 
states that, when the signal handler returns, abort 
terminates the process
*/


/*
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void
abort(void)         // POSIX-style abort() function 
{
    sigset_t           mask;
    struct sigaction   action;

      //Caller can't ignore SIGABRT, if so reset to default.
     
    sigaction(SIGABRT, NULL, &action);
    if (action.sa_handler == SIG_IGN) {
        action.sa_handler = SIG_DFL;
        sigaction(SIGABRT, &action, NULL);
    }
    if (action.sa_handler == SIG_DFL)
        fflush(NULL);           //flush all open stdio streams 

    
     // Caller can't block SIGABRT; make sure it's unblocked.
   
    sigfillset(&mask);
    sigdelset(&mask, SIGABRT);  /mask has only SIGABRT turned off 
    sigprocmask(SIG_SETMASK, &mask, NULL);
    kill(getpid(), SIGABRT);    /send the signal 

    
     // If we're here, process caught SIGABRT and returned.
     
    fflush(NULL);               //flush all open stdio streams 
    action.sa_handler = SIG_DFL;
    sigaction(SIGABRT, &action, NULL);  // reset to default 
    sigprocmask(SIG_SETMASK, &mask, NULL);  //just in case ... 
    kill(getpid(), SIGABRT);                // and one more time 
    exit(1);    // this should never be executed ...
}
*/

/*
1. SIG_IGN
cr7@cr7-virtual-machine:~/APUE/signal$ ./abort 
Aborted
2.sig_handler
cr7@cr7-virtual-machine:~/APUE/signal$ ./abort 
in the handler SIGABRT do nothing
Aborted

*/

static void sig_action(int signo)  
{
    printf("in the handler SIGABRT do nothing\n");
}


int main(void)
{
	struct sigaction action;
	sigset_t newmask;

	action.sa_handler = sig_action;
	//action.sa_handler = SIG_IGN
	action.sa_flags = 0;

	
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGABRT);// block the SIGABRT.
	
	if (sigprocmask(SIG_BLOCK, &newmask, NULL) < 0)
	{
		perror("SIG_BLOCK error");
		exit(1);
	}

	sigaction(SIGABRT, &action, NULL);

	abort();//no matter SIG_IGN or no exit handler and block the SIGABRT, the process will be abort!

	printf("never come to here\n");
	
	exit(1);

}

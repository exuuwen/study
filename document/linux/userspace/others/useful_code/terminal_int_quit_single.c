#include <stdio.h>
#include <signal.h>
#include <unistd.h>
 #include <stdlib.h>
/*The SIGKILL signal is sent when the user types the INTR character (normally C-c).*/

void sig_handler(int sig)
{
	if(sig == SIGTERM)/*It is the normal way to politely ask a program to terminate. send kill*/
		printf("SIGTERM recieved,exiting...\n");
	else if(sig == SIGINT)/*ignal is sent when the user types the INTR character (normally Ctrl-c).*/
		printf("SIGINT recieved,exiting...\n");
	else if(sig == SIGQUIT)/*ignal is sent when the user types the INTR character (normally Ctrl-/).*/
		printf("SIGQUI Trecieved,exiting...\n");
	
	exit(0);
}
int main()
{
	struct sigaction sighandle;

	sighandle.sa_flags = 0;
	sighandle.sa_handler = sig_handler;
	sigemptyset(&sighandle.sa_mask);

	sigaction(SIGTERM, &sighandle, NULL);
	sigaction(SIGINT, &sighandle, NULL);
	sigaction(SIGQUIT, &sighandle, NULL);
	while(1)
	{
		printf("hello world\n");
		sleep(2);
	}
}

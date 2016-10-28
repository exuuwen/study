#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static unsigned long times = 0;
static void sigalrm_handler(int junk)
{
	printf("times is %ld\n",  times++);
	alarm(5);
}
int main()
{
	
	signal(SIGALRM, sigalrm_handler);
	alarm(5);
	
	while(1);
}

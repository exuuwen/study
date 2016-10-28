#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
#include <signal.h>

void printCallStacks()
{
#define SIZE 100
    void *buffer[SIZE];
    char **strings;
    int nptrs;
	int i;
    nptrs = backtrace(buffer, SIZE);
    strings = backtrace_symbols(buffer, nptrs);

    if (strings == NULL)
    {
        fprintf(stderr, "cannot get backtrace_symbols\n");
        return;
    }

    for (i = 0; i < nptrs; ++i)
    {
        fprintf(stderr, "%s\n", strings[i]);
    }

    free(strings);
#undef SIZE
}
void dump(int signo)
{
	printCallStacks();
	exit(1);
}

Debug_Printf_FrameInfos()
{
	signal(SIGSEGV, dump);
}
 
void func_c()
{
	char *p = 0;
	*p = 0x9;
}

void func_b()
{
	func_c();
}

void func_a()
{
	func_b();
}

int main()
{
	Debug_Printf_FrameInfos();
	func_a();

	return 0;
}

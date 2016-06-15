#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>   /* For SYS_write etc */
#include <unistd.h>
#include <sys/user.h>  
#include <stddef.h>   
#include <stdint.h>   
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

void get_data(pid_t child, unsigned long addr, int len, char *str)
{
    char *laddr;
    int i, j;
    union u {
            long val;
            char chars[8];
    }data;

    i = 0;
    j = len / 8;
    laddr = str;
    while(i < j) 
	{
        data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 8, NULL);
        memcpy(laddr, data.chars, 8);
        ++i;
        laddr += 8;
    }

    j = len % 8;
    if(j != 0) 
	{
        data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * 8, NULL);
        memcpy(laddr, data.chars, j);
    }

    str[len] = '\0';
}

void put_data(pid_t child, unsigned long addr, int len, char *str)
{   
    char *laddr;
    int i, j;
    union u {
            long val;
            char chars[8];
    }data;

    i = 0;
    j = len / 8;
    laddr = str;
    while(i < j) 
	{
        memcpy(data.chars, laddr, 8);
        ptrace(PTRACE_POKEDATA, child, addr + i * 8, data.val);
        ++i;
        laddr += 8;
    }

    j = len % 8;
    if(j != 0) 
	{
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKEDATA, child, addr + i * 8, data.val);
    }
}

/*
0000000000601048 D opt
*/

int main(int argc, char **argv)
{
    pid_t child;
	unsigned long addr = 0x0000000000601048;
	int len = sizeof(int);
	int val;
	int status;
	
	if (argc < 2)
	{
		printf("%s target_pid\n", argv[0]);
		exit(1);		
	}

	child = atoi(argv[1]);

	printf("child %d\n", child);	
	int ret = ptrace(PTRACE_ATTACH, child, NULL, NULL);
	if (ret < 0)
	{
		perror("attach fail\n");
		exit(1);
	}

	wait(&status);
	if(WIFEXITED(status))
		exit(1);
			
	val = ptrace(PTRACE_PEEKDATA, child, addr, NULL);
	printf("val %d\n", val);

	val = 0;
	ptrace(PTRACE_POKEDATA, child, addr, val);

	val = ptrace(PTRACE_PEEKDATA, child, addr, NULL);
	printf("last val %d\n", val);

	ptrace(PTRACE_DETACH, child, NULL, NULL);

    return 0;
}

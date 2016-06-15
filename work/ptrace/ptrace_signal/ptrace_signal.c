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

void reverse(char *str)
{   
    int i, j;
    char temp;
    for(i = 0, j = strlen(str) - 2; 
        i <= j; ++i, --j) 
	{
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

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
root@wxztt-family:~/work/ptrace/test# ./attach 6297
stop by sig :19
stop by sig :2

stop by sig :3
stop by sig :4
stop by sig :11
stop by sig :13
*/

int main(int argc, char* argv[])
{
    pid_t child;
   	unsigned long long orig_eax, eax;
    unsigned long long params[3];
    int insyscall = 0;
    int status;
	struct user_regs_struct regs;
	int count = 0;

	if (argc < 2)
	{
		printf("%s target_pid\n", argv[0]);
		exit(1);		
	}

	child = atoi(argv[1]);
	int ret = ptrace(PTRACE_ATTACH, child, NULL, NULL);
	if (ret < 0)
	{
		perror("attach fail\n");
		exit(1);
	}

	while(1) 
	{
		wait(&status);
		if(WIFEXITED(status))
			break;
		if (WIFSTOPPED(status))
		{
			printf("stop by sig i:%d\n", WSTOPSIG(status));
		}
		
		ptrace(PTRACE_CONT, child, 2, NULL);
		
	}

    return 0;
}

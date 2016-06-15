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
  4005e5:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
  4005e9:	8b 00                	mov    (%rax),%eax
  4005eb:	85 c0                	test   %eax,%eax
  4005ed:	74 22                	je     400611 <main+0x54>
  4005ef:	48 8b 45 f0          	mov    -0x10(%rbp),%rax
  4005f3:	48 89 c6             	mov    %rax,%rsi

*/

int main(int argc, char **argv)
{
    pid_t child;
	unsigned long addr = 0x00000000004005e9;
	int len = sizeof(int);
	int val;
	int status;
	struct user_regs_struct regs;
	
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

	while(1)
	{
		wait(&status);
		if(WIFEXITED(status))
			exit(1);

		ptrace(PTRACE_GETREGS, child, NULL, &regs);
		if (regs.rip == addr)
		{
			printf("get the addr\n");
			addr = regs.rax;
			goto done;
		}
		else
			ptrace(PTRACE_SINGLESTEP, child, NULL, NULL);

	}

done:
	val = ptrace(PTRACE_PEEKDATA, child, addr, NULL);
	printf("val %d\n", val);

	val = 0;
	ptrace(PTRACE_POKEDATA, child, addr, val);

	val = ptrace(PTRACE_PEEKDATA, child, addr, NULL);
	printf("last val %d\n", val);

	ptrace(PTRACE_DETACH, child, NULL, NULL);

    return 0;
}

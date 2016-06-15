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


void get_data(pid_t child, unsigned long addr, int len, unsigned char *str)
{
    char *laddr;
    int i, j;
    union u {
            long val;
            unsigned char chars[8];
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

void put_data(pid_t child, unsigned long addr, int len, unsigned char *str)
{   
    char *laddr;
    int i, j;
    union u {
            long val;
            unsigned char chars[8];
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


int main(int argc, char **argv)
{
    pid_t child;
	unsigned long addr = 0x00000000004005dc;
	int len = sizeof(int);
	long val;
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
	printf("val 0x%lx\n", val);

/*
4005dc:	c7 45 f4 64 00 00 00 	movl   $0x64,-0xc(%rbp)
4005e3:	48 83 45 f8 01       	addq   $0x1,-0x8(%rbp)
4005e8:	eb aa                	jmp    400594 <main+0x17
*/

	val = 0x4800000000f445c7;
	ptrace(PTRACE_POKEDATA, child, addr, val);

	val = ptrace(PTRACE_PEEKDATA, child, addr, NULL);
	printf("last val 0x%lx\n", val);

	ptrace(PTRACE_DETACH, child, NULL, NULL);

    return 0;
}

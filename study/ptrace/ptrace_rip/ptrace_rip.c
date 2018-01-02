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
#include <sys/mman.h>

#define RED_ZONE 128

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
        ptrace(PTRACE_POKETEXT, child, addr + i * 8, data.val);
        ++i;
        laddr += 8;
    }

    j = len % 8;
    if(j != 0) 
	{
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKETEXT, child, addr + i * 8, data.val);
    }
}

/*
00000000004005bd T hah
*/
int main(int argc, char* argv[])
{
    pid_t child;
    int status;
	struct user_regs_struct regs;
	struct user_regs_struct regs_old;
	unsigned long addr = 0x00000000004005bd;

	unsigned char invalid_opcode[8]; 
	unsigned char old_opcode[8]; 

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

	waitpid(child, &status, 0);
	if(WIFEXITED(status))
		exit(1);

	memset(&regs, 0, sizeof(struct user_regs_struct));
	memset(&regs_old, 0, sizeof(struct user_regs_struct));
	memset(invalid_opcode, 0, sizeof(invalid_opcode));
	memset(old_opcode, 0, sizeof(old_opcode));

	ret = ptrace(PTRACE_GETREGS, child, NULL, &regs_old);
	if (ret < 0)
	{
		perror("1 getresg\n");
	}

	memcpy(&regs, &regs_old, sizeof(struct user_regs_struct));

	//case 1 escape the redzone and add the invalid code
	/* x86_64 ABI says there might be a RED_ZONE under rsp, bypass it to be safe */
	regs.rsp -= sizeof(invalid_opcode) + RED_ZONE;
	/* rsp holds ret addr of the call stack. An invalid opcode will
     * generate SIGSEGV, then we intercept the signal and we're back */

	//case 2 get the data of rsp address, add invalid code(must align with long) to rsp addr. finally restore old data
	//get_data(child, regs.rsp, sizeof(old_opcode), old_opcode);

	put_data(child, regs.rsp, sizeof(invalid_opcode), invalid_opcode);
	regs.rip = addr;

	/* bypass syscall restarting. arch/x86/kernel/signal.c */
	regs.rax = 0xdeadbeef;

	printf("haha\n");

	ret = ptrace(PTRACE_SETREGS, child, NULL, &regs);
	if (ret < 0)
	{
		perror("1 setresg");
	}

	ptrace(PTRACE_CONT, child, NULL, NULL);

	waitpid(child, &status, 0);
	if(WIFEXITED(status))
		exit(1);
	if (WIFSTOPPED(status))
	{
		printf("stop by sig :%d\n", WSTOPSIG(status));
	}

	ptrace(PTRACE_GETREGS, child, NULL, &regs);
	printf("res rax 0x%llx\n", regs.rax);

	ptrace(PTRACE_SETREGS, child, 0, &regs_old);
	//case 2
	//put_data(child, regs_old.rsp, sizeof(old_opcode), old_opcode);
		
	ptrace(PTRACE_DETACH, child, 0, NULL);
		
    return 0;
}

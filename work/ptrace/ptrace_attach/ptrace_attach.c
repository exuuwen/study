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
		
		if (count == 4)
		{
			ptrace(PTRACE_DETACH, child, NULL, NULL);
			return 0;
		}
		
		unsigned long offset = offsetof(struct user_regs_struct, orig_rax);	
        orig_eax = ptrace(PTRACE_PEEKUSER, child, offset, NULL);
		if (orig_eax < 0)
			perror("erron");
        //printf("The child made asystem call %ld\n", orig_eax);
        //ptrace(PTRACE_CONT, child, NULL, NULL);
         
		if(orig_eax == SYS_write) 
		{
			if(insyscall == 0) 
			{ 
				count++;   
               	/* Syscall entry */
               	insyscall = 1;
			
				//ptrace(PTRACE_GETREGS, child, NULL, &regs);
               	//printf("Write called with %llu, %llu, %llu\n", regs.rdi, regs.rsi, regs.rdx);
				
				unsigned long offset = offsetof(struct user_regs_struct, rdi);	
               	params[0] = ptrace(PTRACE_PEEKUSER, child, offset, NULL);
				offset = offsetof(struct user_regs_struct, rsi);	
               	params[1] = ptrace(PTRACE_PEEKUSER, child, offset, NULL);
				offset = offsetof(struct user_regs_struct, rdx);	
               	params[2] = ptrace(PTRACE_PEEKUSER, child, offset, NULL);

				char *str = malloc(params[2]);
				get_data(child, params[1], params[2], str);
               	printf("Write called with %llu, %llu, %llu\n", params[0], params[1], params[2]);
				printf("write contens:%s", str);
				reverse(str);
				put_data(child, params[1], params[2], str);
			}
          	else 
			{ 
				/* Syscall exit */
				unsigned long offset = offsetof(struct user_regs_struct, rax);	
               	eax = ptrace(PTRACE_PEEKUSER, child, offset, NULL);
				printf("Write returned with %llu\n", eax);
				insyscall = 0;
			}
		}
            
		ptrace(PTRACE_SYSCALL, child, NULL, NULL);
	}

    return 0;
}

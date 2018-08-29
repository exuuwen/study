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
#include <dlfcn.h>

#include RED_ZONE 128


/* Retrieve libc start address by scanning /proc/pid/maps */
static unsigned long get_text_start_addr(const char *mapfile, const char *file)
{
	FILE *fp;
	char mapbuf[4096], perms[32], libpath[4096];
	unsigned long start, end, file_offset, inode;
	unsigned int dev_major, dev_minor;

	fp = fopen(mapfile, "rb");
	if (!fp) {
		printf("Failed to open %s\n", mapfile);
		exit(1);
	}

	while (fgets(mapbuf, sizeof(mapbuf), fp)) {
		sscanf(mapbuf, "%lx-%lx %s %lx %x:%x %lu %s", &start, &end, perms, &file_offset, &dev_major, &dev_minor, &inode, libpath);

		if (!strncmp(perms, "r-xp", 4) && !strncmp(libpath, file, strlen(file))) {
			printf("%s: %08lx-%08lx %s %lx %s\n", mapfile, start, end, perms, file_offset, libpath);
			fclose(fp);
			return start;
		}
	}

	fclose(fp);
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
  400622:	55                   	push   %rbp
  400623:	48 89 e5             	mov    %rsp,%rbp
  400626:	48 89 7d f8          	mov    %rdi,-0x8(%rbp)
  40062a:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
  40062e:	48 8b 00             	mov    (%rax),%rax
  400631:	48 8d 50 01          	lea    0x1(%rax),%rdx
  400635:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
  400639:	48 89 10             	mov    %rdx,(%rax)
  40063c:	5d                   	pop    %rbp
  40063d:	c3                   	retq  
*/
int main(int argc, char* argv[])
{
    pid_t child;
    int status;
	struct user_regs_struct regs;
	struct user_regs_struct regs_old;
	/*get mmap addr:
	1. if the target called mmap: nm tracee 
	2. mmap buildin libc, objdump libcx.x.so get offset, and /proc/pid/maps get libc text base. base + offset
	3. traceer dlsym(mmap) - /proc/self/maps libs base = offset, adn /proc/pid/maps get libc text base. base + offset*/
	/*case 1*/
	//unsigned long mmap_addr = 0x0000000000400490;
	/*case 2*/
	//unsigned long mmap_offset = 0x00000000000f58d0;
	/*case 3*/
	unsigned long mmap_addr = 0;
	unsigned long mmap_offset = 0;
	unsigned long alloc_addr = 0x0;
	unsigned long old_func_addr = 0x000000000040057d;

	unsigned char invalid_opcodes[8]; 
	unsigned char new_opcodes[] = 
								"\x55"
								"\x48\x89\xe5"
								"\x48\x89\x7d\xf8"
								"\x48\x8b\x45\xf8"
								"\x48\x8b\x00"
								"\x48\x8d\x50\x02"
								"\x48\x8b\x45\xf8"
								"\x48\x89\x10"
								"\x5d\xc3";
	unsigned char old_opcodes[12];
	char mapfile[256];

	if (argc < 2)
	{
		printf("%s target_pid\n", argv[0]);
		exit(1);		
	}

	child = atoi(argv[1]);

	if (mmap_addr == 0)
	{
		if (mmap_offset == 0)
		{
			mmap_addr = get_text_start_addr("/proc/self/maps", "/lib/x86_64-linux-gnu/libc-2.19.so");
			mmap_offset = (unsigned long)dlsym(NULL, "mmap");
	
			mmap_offset -= mmap_addr;
		}
		
		printf("mmap offset :0x%lx\n", mmap_offset);

		sprintf(mapfile, "/proc/%d/maps", child);
		mmap_addr = get_text_start_addr(mapfile, "/lib/x86_64-linux-gnu/libc-2.19.so");
		mmap_addr += mmap_offset;
	}

	memset(invalid_opcodes, 0, sizeof(invalid_opcodes));
		
	printf("mmap_addr  :0x%lx\n", mmap_addr);

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

	ret = ptrace(PTRACE_GETREGS, child, NULL, &regs_old);
	if (ret < 0)
	{
		perror("1 getresg\n");
	}

	memcpy(&regs, &regs_old, sizeof(struct user_regs_struct));
	/* x86_64 ABI says there might be a RED_ZONE under rsp, bypass it to be safe */
	regs.rsp -= sizeof(invalid_opcodes) + RED_ZONE;
	/* rsp holds ret addr of the call stack. An invalid opcode will
     * generate SIGSEGV, then we intercept the signal and we're back */
	put_data(child, regs.rsp, sizeof(invalid_opcodes), invalid_opcodes);
	regs.rip = mmap_addr;
	
	regs.rdi = 0;
	regs.rsi = sizeof(new_opcodes);
	regs.rdx = PROT_EXEC|PROT_WRITE;
	regs.rcx = MAP_PRIVATE|MAP_ANONYMOUS;
	regs.r8 = 0;
	regs.r9 = 0;
	
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
	alloc_addr = regs.rax;

	put_data(child, alloc_addr, sizeof(new_opcodes), new_opcodes);

	// mov $addr, %rax
	memset(old_opcodes, 0x48, 1);
    memset(old_opcodes + 1, 0xb8, 1);
    memcpy(old_opcodes + 2, &alloc_addr, 8);
	// jmp *%rax
    memset(old_opcodes + 10, 0xff, 1);
    memset(old_opcodes + 11, 0xe0, 1);

	put_data(child, old_func_addr, sizeof(old_opcodes), old_opcodes);

	ptrace(PTRACE_SETREGS, child, 0, &regs_old);
		
	ptrace(PTRACE_DETACH, child, 0, NULL);
		
    return 0;
}

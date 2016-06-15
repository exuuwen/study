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
#include <string.h>

#define RED_ZONE 8

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

    str[len - 1] = '\0';
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

int main(int argc, char* argv[])
{
    pid_t child;
    int status;
	struct user_regs_struct regs;
	struct user_regs_struct regs_old;
	/*get mmap addr:
	1. if the target called dlopen: nm tracee 
	2. mmap buildin libc, objdump libcx.x.so get offset, and /proc/pid/maps get libc text base. base + offset
	3. traceer dlsym(mmap) - /proc/self/maps libs base = offset, adn /proc/pid/maps get libc text base. base + offset*/
	/*case 1*/
	//unsigned long dlopen_addr = 0x0000000000400490;
	/*case 2*/
	//unsigned long dlopen_offset = 0x0000000000137070;
	/*case 3*/
	unsigned long dlopen_addr = 0;
	unsigned long dlopen_offset = 0;
	unsigned long alloc_addr = 0x0;
	unsigned long old_func_addr = 0x000000000040057d;
	unsigned long libpatch_addr = 0x0000000000000675;

	unsigned char invalid_opcodes[8]; 
	unsigned char rsp_opcodes[8]; 
	unsigned char rspoff_opcodes[32]; 
	unsigned char old_opcodes[12];
	char mapfile[256];
	char filename_new_so[32];

	if (argc < 2)
	{
		printf("%s target_pid\n", argv[0]);
		exit(1);		
	}

	child = atoi(argv[1]);

	if (dlopen_addr == 0)
	{
		if (dlopen_offset == 0)
		{
			dlopen_addr = get_text_start_addr("/proc/self/maps", "/lib/x86_64-linux-gnu/libc-2.19.so");
			dlopen_offset = (unsigned long)dlsym(NULL, "__libc_dlopen_mode");
	
			dlopen_offset -= dlopen_addr;
		}
		
		printf("mmap offset :0x%lx\n", dlopen_offset);

		sprintf(mapfile, "/proc/%d/maps", child);
		dlopen_addr = get_text_start_addr(mapfile, "/lib/x86_64-linux-gnu/libc-2.19.so");
		dlopen_addr += dlopen_offset;
	}

	memset(invalid_opcodes, 0, sizeof(invalid_opcodes));
	memset(rsp_opcodes, 0, sizeof(rsp_opcodes));
	memset(rspoff_opcodes, 0, sizeof(rspoff_opcodes));
	memset(filename_new_so, 0, sizeof(filename_new_so));
		
	printf("dlopen_addr  :0x%lx\n", dlopen_addr);
	sprintf(filename_new_so, "/usr/lib/libpatch.so");

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

	/* rsp holds ret addr of the call stack. An invalid opcode will
     * generate SIGSEGV, then we intercept the signal and we're back */
	get_data(child, regs.rsp - RED_ZONE, sizeof(rspoff_opcodes), rspoff_opcodes);
	get_data(child, regs.rsp, sizeof(rsp_opcodes), rsp_opcodes);

	put_data(child, regs.rsp, sizeof(invalid_opcodes), invalid_opcodes);
	put_data(child, regs.rsp - RED_ZONE, strlen(filename_new_so) + 1, filename_new_so);

	regs.rip = dlopen_addr; 
	regs.rdi = regs.rsp - RED_ZONE;
	regs.rsi = RTLD_NOW|RTLD_GLOBAL|RTLD_NODELETE;

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

	sprintf(mapfile, "/proc/%d/maps", child);
	libpatch_addr += get_text_start_addr(mapfile, "/usr/lib/libpatch.so");
	printf("patch func 0x%lx\n", libpatch_addr);

	//sleep(20);

	// mov $addr, %rax
	memset(old_opcodes, 0x48, 1);
    memset(old_opcodes + 1, 0xb8, 1);
    memcpy(old_opcodes + 2, &libpatch_addr, 8);
	// jmp *%rax
    memset(old_opcodes + 10, 0xff, 1);
    memset(old_opcodes + 11, 0xe0, 1);

	put_data(child, old_func_addr, sizeof(old_opcodes), old_opcodes);
	put_data(child, regs_old.rsp, sizeof(rsp_opcodes), rsp_opcodes);
	put_data(child, regs_old.rsp - RED_ZONE, sizeof(rspoff_opcodes), rspoff_opcodes);

	ptrace(PTRACE_SETREGS, child, 0, &regs_old);
		
	ptrace(PTRACE_DETACH, child, 0, NULL);
		
    return 0;
}

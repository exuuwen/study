#ifndef _SANDBOX_H_
#define _SANDBOX_H_

#include <stdio.h>
#include <sys/prctl.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/capability.h>
#include <sched.h>

#define PR_ATTACH_SECCOMP_FILTER	37

#ifndef CLONE_NEWPID
#define CLONE_NEWPID  0x20000000
#endif

#define SAFE_DIR "/home/cr7/chroot"

struct syscall_filter
{
	unsigned short count;  /* Instruction count */
	int *syscall_list; 
};

int model_syscalls[] = {
	__NR_read, __NR_write, __NR_close, __NR_exit_group, __NR_sigreturn, __NR_open, __NR_execve, __NR_brk, __NR_access, __NR_mmap2, __NR_fstat64, __NR_mprotect, 
	__NR_set_thread_area, __NR_munmap, __NR_fcntl64, __NR_stat64, /* null terminated */
};


#endif

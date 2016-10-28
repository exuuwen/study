#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

/*
handle_mm_fault()
{
if (pmd_none(*pmd) && transparent_hugepage_enabled(vma)) {
		int ret = VM_FAULT_FALLBACK;
		if (!vma->vm_ops)
			ret = do_huge_pmd_anonymous_page(mm, vma, address,
					pmd, flags);
		if (!(ret & VM_FAULT_FALLBACK))
			return ret;
	} 
}
*/

/*should be the n times of the HUGESIZE that can allocate hugepage for each and 
also should # echo always > /sys/kernel/mm/transparent_hugepage/enabled */

#define SIZE (4*1024*1024)
#define HUGESIZE 2*1024*1024

int main(void)
{
	int fd, ret = 0;
	void *vadr;

	printf("pid %d\n", getpid());


	scanf("%d", &fd);
	printf("fd %d\n", fd);

	
	posix_memalign(&vadr, HUGESIZE, SIZE);


	//sleep(10);
	scanf("%d", &fd);
	printf("2 fd %d\n", fd);

	*(char*)vadr = 'a';
	*(char*)((char*)vadr + 2*1024*1024) = 'b';

	//printf("%s\n", vadr);
	scanf("%d", &fd);
	printf("3 fd %d\n", fd);

	free(vadr);
	exit(ret);
}


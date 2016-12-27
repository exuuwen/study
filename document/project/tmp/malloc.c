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

/*
# echo mdavise > /sys/kernel/mm/transparent_hugep
*/
#define SIZE (1024*1024*4)
#define HUGESIZE 2*1024*1024

int main(void)
{
	int fd, ret = 0;
	void *vadr;

	printf("pid %d\n", getpid());

	vadr = malloc(SIZE); 

	for (ret=0; ret<SIZE; ret++)
	    *(char*)(vadr + ret) = 'a';

	free(vadr);	
}


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

	scanf("%d", &fd);
	printf("fd %d\n", fd);

	vadr = mmap(0, SIZE, PROT_READ | PROT_WRITE,  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);// only private

	 if (vadr == MAP_FAILED) {
		perror("mmap");
		ret = -1;
		goto fail;
	}

	/*should be the n times of the HUGESIZE that can allocate hugepage for each */
	if (!(SIZE & (HUGESIZE -1)) && (size_t)vadr & (HUGESIZE - 1))
	{
		printf("in remap\n");
		char *old;
		munmap(vadr, SIZE);
	
		vadr = mmap(0, SIZE + HUGESIZE - 1, PROT_READ | PROT_WRITE,  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		old = vadr;
		vadr = (char*)(((size_t)vadr + HUGESIZE - 1) & ~(HUGESIZE - 1));
		if (old != vadr)
			munmap(old, (size_t)vadr - (size_t)old);
		if (vadr != old + HUGESIZE -1)
			munmap(vadr + SIZE, (size_t)old + HUGESIZE - 1 - (size_t)vadr);
	}

	scanf("%d", &fd);
	printf("1 fd %d\n", fd);

	ret = madvise(vadr, SIZE, MADV_HUGEPAGE);
	if (ret < 0)
	{
		perror("madvise:");
		goto out;
	}

	//sleep(10);
	scanf("%d", &fd);
	printf("2 fd %d\n", fd);

	*(char*)vadr = 'a';

	*(char*)((char*)vadr + 2*1024*1024) = 'b';
	//printf("%s\n", vadr);
	scanf("%d", &fd);
	printf("3 fd %d\n", fd);

out:	
	if (-1 == munmap(vadr, SIZE))
		ret = -1;
	
	//free(vadr);
fail:
	exit(ret);
}


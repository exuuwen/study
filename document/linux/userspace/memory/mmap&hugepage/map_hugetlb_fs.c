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
if (unlikely(is_vm_hugetlb_page(vma)))
		return hugetlb_fault(mm, vma, address, flags);
}
*/

/*
It is dependent on nr_hugepages and nr_overcommit_hugepages
*/

int main(void)
{
	int fd, ret = 0;
	char *vadr;

	printf("pid %d\n", getpid());

	//sleep(25);
	scanf("%d", &fd);
	printf("fd %d\n", fd);
	vadr = mmap(0, 1024*1024, PROT_READ | PROT_WRITE,   /*MAP_LOCKED |*/ MAP_HUGETLB | MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);//can't have MAP_LOCKED flags
	 
	if (vadr == MAP_FAILED) {
		perror("mmap");
		ret = -1;
		goto fail;
	}

	scanf("%d", &fd);
	printf("2 fd %d\n", fd);

	*vadr = 'a';

	scanf("%d", &fd);
	printf("3 fd %d\n", fd);
	
	if (-1 == munmap(vadr, 1024*1024))
		ret = -1;
fail:
	exit(ret);
}


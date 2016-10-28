#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

/*
if (!pte_present(entry)) {
		if (pte_none(entry)) {
			if (vma->vm_ops) {
				if (likely(vma->vm_ops->fault))
					return do_linear_fault(mm, vma, address,
						pte, pmd, flags, entry);
			}
			return do_anonymous_page(mm, vma, address,
						 pte, pmd, flags);
		}
		if (pte_file(entry))
			return do_nonlinear_fault(mm, vma, address,
					pte, pmd, flags, entry);
		return do_swap_page(mm, vma, address,
					pte, pmd, flags, entry);
	}

*/

int main(void)
{
	int fd, ret = 0;
	char *vadr;

	printf("pid %d\n", getpid());

	scanf("%d", &fd);
	printf("fd %d\n", fd);

	vadr = mmap(0, 1024*1024, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0); //if add MAP_LOCKED  it will handle_mm_fault with mmap before sleep(10)
	 
	if (vadr == MAP_FAILED) {
		perror("mmap");
		ret = -1;
		goto fail;
	}
	
	scanf("%d", &fd);
	printf("fd %d\n", fd);

	/*get a do_liner_fault*/
	/*
		if (vm_flags & VM_SHARED) {
		error = shmem_zero_setup(vma);
			//vma->vm_file = file;
			//vma->vm_ops = &shmem_vm_ops;
	}
	*/
	*vadr = 'a';

	//printf("%s\n", vadr);

	if (-1 == munmap(vadr, 1024*1024))
		ret = -1;
fail:
	exit(ret);
}


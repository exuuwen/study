#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>

#define SIZE 6*4096 //(1024*1024*2)
#define HUGESIZE 2*1024*1024

/*
/proc/pid/pagemap.  This file lets a userspace process find out which
   physical frame each virtual page is mapped to.  It contains one 64-bit
   value for each virtual page, containing the following data (from
   fs/proc/task_mmu.c, above pagemap_read):

    * Bits 0-54  page frame number (PFN) if present
    * Bits 0-4   swap type if swapped
    * Bits 5-54  swap offset if swapped
    * Bit  55    pte is soft-dirty (see Documentation/vm/soft-dirty.txt)
    * Bits 56-60 zero
    * Bit  61    page is file-page or shared-anon
    * Bit  62    page swapped
    * Bit  63    page present
*/

/*
/proc/kpageflags.  This file contains a 64-bit set of flags for each
   page, indexed by PFN
0. LOCKED
     1. ERROR
     2. REFERENCED
     3. UPTODATE
     4. DIRTY
     5. LRU
     6. ACTIVE
     7. SLAB
     8. WRITEBACK
     9. RECLAIM
    10. BUDDY
    11. MMAP
    12. ANON
    13. SWAPCACHE
    14. SWAPBACKED
    15. COMPOUND_HEAD
    16. COMPOUND_TAIL
    16. HUGE
    18. UNEVICTABLE
    19. HWPOISON
    20. NOPAGE
    21. KSM
    22. THP
*/

/*
 /proc/kpagecount.  This file contains a 64-bit count of the number of
   times each page is mapped, indexed by PFN.
*/

int main(void)
{
    int fd_pm, fd_pc, fd_pf, num_pages, ret = 0;
    void *vadr, *end;
    char path_pm[255];
    char path_pc[255];
    char path_pf[255];
    int page_size;

    printf("pid %d\n", getpid());
    page_size = getpagesize();


    vadr = mmap(0, SIZE, PROT_READ | PROT_WRITE,  MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (vadr == MAP_FAILED) 
    {
        perror("mmap");
        ret = -1;
        goto fail;
    }

    /*if (!(SIZE & (HUGESIZE -1)) && (size_t)vadr & (HUGESIZE - 1))
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

    ret = madvise(vadr, SIZE, MADV_HUGEPAGE);
    if (ret < 0)
    {
        perror("madvise:");
        goto out2;
    }*/

    memset(vadr, 'a', SIZE - 2*4096);

    sprintf(path_pm, "/proc/self/pagemap");
    
    num_pages = SIZE / page_size;

    fd_pm = open(path_pm, O_RDONLY);
    if (fd_pm < 0) 
    {
        printf("%s(): cannot open %s: %s\n", __FUNCTION__, path_pm, strerror(errno));
        goto out2;
    }

    sprintf(path_pc, "/proc/kpagecount");
    fd_pc = open(path_pc, O_RDONLY);
    if (fd_pc < 0) 
    {
       printf("%s(): cannot open %s: %s\n", __FUNCTION__, path_pc, strerror(errno));
       goto out1;
    }

    sprintf(path_pf, "/proc/kpageflags");
    fd_pf = open(path_pf, O_RDONLY);
    if (fd_pf < 0) 
    {
       printf("%s(): cannot open %s: %s\n", __FUNCTION__, path_pf, strerror(errno));
       goto out0;
    }

    long index = ((unsigned long) vadr / page_size) * sizeof(unsigned long long);
    long o;
    ssize_t t;

    /* Seek to appropriate index of pagemap file.*/                   
    o = lseek(fd_pm, index, SEEK_SET);
    if (o < 0)
    {
        perror("lseek error:");
        goto out;
    }

    int i = 1;
    while (num_pages > 0)
    {
        unsigned long long pa;
	unsigned long pfn;
        unsigned long long map_count;
        unsigned long long flags;

        t = read(fd_pm, &pa, sizeof(unsigned long long));
        if (t < 0)
        {
            perror("Error reading file pagemap:");
            goto out;
        }

        if (pa & 0x8000000000000000ULL)
        {
	    pfn =  pa & 0x7fffffffffffffULL;

	    printf("page:%d--->pfn:0x%lx pa:0x%llx, ", i++, pfn, pa);

            o = lseek(fd_pc, pfn * sizeof(unsigned long long), SEEK_SET);
            if (o < 0)
            {
                perror("lseek error:");
                goto out;
            }

            t = read(fd_pc, &map_count, sizeof(unsigned long long));
            if (t < 0)
            {
                perror("Error reading file pagecount:");
                goto out;
            }

            printf("mapcount:%lld, ", map_count);

            o = lseek(fd_pf, pfn * sizeof(unsigned long long), SEEK_SET);
            if (o < 0)
            {
                perror("lseek error:");
                goto out;
            }

             t = read(fd_pf, &flags, sizeof(unsigned long long));
            if (t < 0)
            {
                perror("Error reading file pageflags:");
                goto out;
            }

            printf("flags:0x%llx\n", flags);
        }
        else
        {
            printf("page:%d is not present pa:0x%llx\n", i++, pa);
        }
        
        num_pages --;
    }

out:
    close(fd_pf);
out0:
    close(fd_pc);
out1:
    close(fd_pm);
out2:
    if (-1 == munmap(vadr, SIZE))
        ret = -1;
fail:
    exit(ret);
}


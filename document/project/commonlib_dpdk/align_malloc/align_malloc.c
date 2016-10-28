#include "align_malloc.h"

/* true if x is a power of 2 */
#define POWEROF2(x) ((((x)-1) & (x)) == 0)

/* Allocate memory from the heap */
void *align_malloc(size_t size, unsigned align)
{
        void *ptr;
        unsigned long addr, align_addr;
        unsigned sz;

        /* align must be a power of 2 */
        if (unlikely(align != 0 && !POWEROF2(align)))
                return NULL;
        if (align != 0)
                align -= 1;

        /* allocated size depends on required alignment and must
         * include some space to store malloc'd pointer */
        sz = size + sizeof(void *) + align;
        ptr = malloc(sz);
        if (unlikely(ptr == NULL))
                return NULL;

        /* get aligned address */
        addr = (unsigned long)ptr;
        align_addr = (addr + sizeof(void *) + align) & ~((size_t)align);

        /* save allocated address for future free */
        *(void **)(align_addr - sizeof(void *)) = ptr;

        return (void *)align_addr;
}

/*Allocate zero'ed memory from the heap */
void *align_zmalloc(size_t size, unsigned align)
{
        void *ptr;
        ptr = align_malloc(size, align);
        if (unlikely(ptr == NULL))
                return NULL;
        memset(ptr, 0, size);
        return ptr;
}

/* Frees the memory space pointed to by ptr */
void align_free(void *ptr)
{
        unsigned long addr;
        addr = (unsigned long)ptr - sizeof(void *);
        free(*(void **)addr);
}




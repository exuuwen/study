#include <linux/module.h>
#include <linux/vmalloc.h>
#include <asm/pgtable.h>
 
MODULE_AUTHOR("wenxu");
MODULE_DESCRIPTION("This is a module sample.");
MODULE_LICENSE("GPL");

/*
VmallocTotal:     122880 kB: decide the max vmalloc size  VmallocTotal =  (VMALLOC_END - VMALLOC_START)/1024
VmallocUsed:        2560 kB
VmallocChunk:     113040 kB
*/
/* 
	#define VMALLOC_START	((unsigned long)high_memory + VMALLOC_OFFSET)
	#ifdef CONFIG_HIGHMEM
	# define VMALLOC_END	(PKMAP_BASE - 2 * PAGE_SIZE)
	#else
	# define VMALLOC_END	(FIXADDR_START - 2 * PAGE_SIZE)
	#endif	
*/

 
/*Godbach added module parameter*/
static int memsize = 100;/*Unit: Mbyte*/
module_param(memsize, int, S_IRUGO);
__u8 *data;

unsigned long *high_memory = (void *)0xc0a2038c;
int
init_module (void)
{
        data = vmalloc(1024 * 1024 * memsize);
	
	printk("VMALLOC_START:0x%lx, VMALLOC_END:0x%lx\n", (*high_memory) + VMALLOC_OFFSET, VMALLOC_END);
 
        if (!data)
                return -ENOMEM;
 
        memset(data, 0xff, 1024 * 1024 * memsize);
        printk("module loaded.\n");
        return 0;
}
 
 
void
cleanup_module(void)
{
        vfree(data);
        printk("module unloaded.\n");
}

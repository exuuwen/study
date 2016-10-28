#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/gfp.h>
#include <linux/vmalloc.h>


void* vaddr = NULL;
void* kaddr = NULL;
void* pkaddr = NULL;
char *str = "hello world!";
struct page* vpage;
unsigned long vphy = 0;
unsigned long vpfn = 0;
unsigned long offset = 0x123;
void* nvaddr = NULL;

unsigned long temp_addr = 0;

static inline unsigned long vmalloc_to_kaddr(unsigned long adr)
{
	unsigned long kva;

	kva = (unsigned long) page_address(vmalloc_to_page((void *)adr));
	kva |= adr & (PAGE_SIZE-1); /* restore the offset */

	return kva;
}

static int __init filp_init(void)
{
	
	printk("%s, %d\n", __func__, __LINE__);

	vaddr = vmalloc(4*1024*1024);
	
	memset(vaddr, 0, 4*1024*1024);	
	
	nvaddr = vaddr + offset;
	memcpy(nvaddr, str, 12);
	
	printk("vmalloc:%s\n", (char*)nvaddr);
	
	vpage = vmalloc_to_page(nvaddr);  /*just vmalloc to page, u can not do the op convertly*/
	
	pkaddr = page_address(vpage);      /*find the kernel addrss which directly map in kernel*/
	printk("pkaddr is 0x%lx\n", (unsigned long)pkaddr);

	if (pkaddr)
	{
                /*if the pkaddr is not zero, this page is dirctly map in the kernel or is the highmem*/
		kaddr = (void*)vmalloc_to_kaddr((unsigned long)nvaddr);
		printk("kaddr is 0x%lx\n", (unsigned long)kaddr);
		printk("kernel:%s\n", (char*)kaddr);
	}

	vpfn = vmalloc_to_pfn(nvaddr);  /*just vmalloc to pfn, u can not do the op convertly*/
	printk("vpfn is 0x%lx\n", vpfn);

	vphy = (unsigned long)(vpage - mem_map) << PAGE_SHIFT; /*page to phy*/  //page - mem_map is pfn
	vphy |= ((unsigned long)nvaddr)&(PAGE_SIZE - 1);
	printk("vphy is 0x%lx\n", vphy);

	return 0;

}

static void __exit filp_exit(void)
{
	printk("%s, %d\n", __func__, __LINE__);
	vfree(vaddr);	
}
module_init(filp_init);
module_exit(filp_exit);

MODULE_AUTHOR("wx");
MODULE_LICENSE("GPL");

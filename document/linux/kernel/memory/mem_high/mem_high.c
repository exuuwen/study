#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/gfp.h>
#include <linux/vmalloc.h>
#include <linux/highmem.h>

struct page *page = NULL;
struct page *page1= NULL;

unsigned long phy_addr = 0, virt_addr = 0;
unsigned long phy_addr1 = 0, virt_addr1 = 0;

static int __init init(void)
{
	
	printk("%s\n", __func__);
	
	page = alloc_page(GFP_HIGHUSER); 
	phy_addr = (unsigned long)(page - mem_map) << PAGE_SHIFT;
	page = pfn_to_page(phy_addr >> PAGE_SHIFT);
	if(phy_addr == ((unsigned long)(page - mem_map) << PAGE_SHIFT))
		printk("(unsigned long)(page - mem_map) << PAGE_SHIFT; ok\n");
	virt_addr = (unsigned long)page_address(page);
	printk("alloc_page(GFP_HIGHUSER)  phy_addr is 0x%lx,page addr is 0x%lx\n", phy_addr, virt_addr);
	if((virt_addr == 0) && (phy_addr > 0x38000000))
	{
		printk("virt_addr == 0 highmem\n");
		virt_addr = (unsigned long)kmap(page);//another  time the page is be yinshe forever 
		printk("got virt_addr is 0x%lx, page_address(page) is 0x%p\n", virt_addr, page_address(page));//there is a address for this page for permanent mapping 
		memcpy((void*)virt_addr, "xuxu1", 6);
		printk("virt_addr is %s\n", (char*)virt_addr);
		
	}
	else
	{
		page = virt_to_page(virt_addr);
		printk("is already map back page_address(page) is 0x%p\n", page_address(page));	
		kunmap(page);
	}
	
	

	page1 = alloc_page(GFP_HIGHUSER);  
	phy_addr1 = (unsigned long)(page1 - mem_map) << PAGE_SHIFT;
	page1 = pfn_to_page(phy_addr1 >> PAGE_SHIFT);
	if(phy_addr1 == ((unsigned long)(page1 - mem_map) << PAGE_SHIFT))
		printk("(unsigned long)(page1 - mem_map) << PAGE_SHIFT; ok\n");
	virt_addr1 = (unsigned long)page_address(page1);
	printk("alloc_page(GFP_HIGHUSER)  phy_addr is 0x%lx,page addr is 0x%lx\n", phy_addr1, virt_addr1);
	
	if((virt_addr1 == 0) && (phy_addr > 0x38000000))
	{
		printk("virt_addr1 == 0 highmem\n");
		virt_addr1 = (unsigned long)kmap_atomic(page1);  
		printk("got virt_addr1 is 0x%lx, page_address(page1) is 0x%p\n", virt_addr1, page_address(page1)); //page_address only for lowmem and kmap mapping mem, not use for  this(kmap_atomic);
		memcpy((void*)virt_addr1, "tingting2", 9);
		printk("virt_addr is %s\n", (char*)virt_addr1);
		kunmap_atomic((void*)virt_addr1);// kunmap_atomic should be the same process with the kmap_atomic, so it can't be in the _exit.
	}
	else
	{	
		page1 = virt_to_page(virt_addr1);
		printk("is already map back page_address(page) is 0x%p\n", page_address(page1));
	}
        return 0;
}

static void __exit fini(void)
{
	printk("%s\n", __func__);
        if (page)
		__free_page(page);
	if (page1)
		__free_page(page1);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");

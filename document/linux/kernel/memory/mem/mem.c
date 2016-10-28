/*
 joshua zhan 2010/09/04
*/

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

void * kmalloc_addr = NULL;
void * kmalloc_h_addr = NULL;
void * kmalloc_d_addr = NULL;

void * gfp_addr = NULL;
void * gfp_d_addr = NULL;
void * gfp_h_addr = NULL;

struct page *page_dma = NULL;
struct page *page = NULL;
struct page *page_high = NULL;

struct page *new_page = NULL;
struct page *page_back = NULL;
struct page *page_back_phy= NULL;

void* vmalloc_addr = NULL;

static int __init filp_init(void)
{
	
	unsigned long phy_addr = 0;
	unsigned long virt_addr = 0;
	printk("%s, %d\n", __func__, __LINE__);

	kmalloc_d_addr = kmalloc(10, __GFP_DMA); //it is shall be less 3G+16M ,malloc size less than 128k
	printk("kmalloc dma addr is 0x%lx\n", kmalloc_d_addr);
	kmalloc_addr = kmalloc(10, GFP_KERNEL); //it is shall be less 3G+896M
	printk("kmalloc addr is 0x%lx\n", kmalloc_addr);
	kmalloc_h_addr = kmalloc(10, GFP_HIGHUSER); //it is shall be less 3G+896M  is only  dma and normal, if not dma  it is normal  
	printk("kmalloc highmem addr is 0x%lx\n", kmalloc_h_addr);
	printk("\n");
	gfp_d_addr = __get_free_page(__GFP_DMA);  // it is shall 3G+16M, size less than 4M
	printk("__get_free_page dma addr is 0x%lx\n", gfp_d_addr);
	gfp_addr = __get_free_page(GFP_KERNEL);  //it is shall be less 3G+896M
	printk("__get_free_page addr is 0x%lx\n", gfp_addr);
	gfp_h_addr = __get_free_page(GFP_HIGHUSER);  // it is shall be 0x0,if it is not yinshe;
	printk("__get_free_page highmem addr is 0x%lx\n", gfp_h_addr);
	printk("\n");
	page_dma = alloc_page(__GFP_DMA); 		//size less than 4M
 	printk("phy_addr is 0x%lx, page_dma addr is 0x%lx\n",(unsigned long)(page_dma - mem_map) << PAGE_SHIFT, page_address(page_dma));  // phy_addr is 0 - x,addr is shall 3G+x
	page = alloc_page(GFP_KERNEL);
 	printk("phy_addr is 0x%lx,page addr is 0x%lx\n",(unsigned long)(page - mem_map) << PAGE_SHIFT, page_address(page));  //phy_addr is x to 896,addr is shall be less 3G+896M
	page_high = alloc_page(GFP_HIGHUSER);
 	printk("phy_addr is 0x%lx,page hight addr is 0x%lx\n",(unsigned long)(page_high - mem_map) << PAGE_SHIFT, page_address(page_high)); // phy_addr shall be bigger than 896M, addr is shall be 0x0  if it is not yinshe;
	printk("\n");
	
/*page,phy_addr,virt_addr convert each other ,when highmem is  yinshe ,it has the virt_addr  from 3G+896M to 4G,then you can get the virt_addr by page_address and when only  virt_addr is for lowmem you can  find the page through the  virt_to_page*/
	
	new_page = alloc_page(GFP_KERNEL);
	
	virt_addr = page_address(new_page);      /*page to virt note: page_address for highmem just ok when highmem is yinshe*/
	printk("virt_addr is 0x%lx\n", virt_addr);
	page_back = virt_to_page(virt_addr);  /*just for lowmem which less 896 */
	printk("back virt_addr is 0x%lx\n",  page_address(page_back));
	printk("\n");

	phy_addr = (unsigned long)(new_page - mem_map) << PAGE_SHIFT; /*page to phy*/  //page - mem_map is pfn
	printk("phy_addr is 0x%lx\n", phy_addr);
	page_back_phy = pfn_to_page(phy_addr >> PAGE_SHIFT);
	printk("back phy_addr is 0x%lx\n",  (unsigned long)(page_back_phy - mem_map) << PAGE_SHIFT);
	printk("\n");

	phy_addr = __pa(virt_addr);
	printk("pa back phy_addr is 0x%lx\n", phy_addr); /*virt to phy only when phy is less 896m or first find the page then convert*/
	virt_addr = __va(phy_addr);
	printk("va back virt_addr is 0x%lx\n", virt_addr);

	new_page = alloc_page(GFP_HIGHUSER);
	
	virt_addr = page_address(new_page);      /*page to virt note: page_address for highmem just ok when highmem is yinshe*/
	printk("virt_addr is 0x%lx\n", virt_addr);
	if(virt_addr != 0)
	{
		printk("virt_addr is not zero\n");
		page_back = virt_to_page(virt_addr);  /*only for less 896m ,highmem should search the pte   this is wrong not the real page*/
		printk("back virt_addr is 0x%lx\n",  page_address(page_back));
		printk("\n");
	}

	phy_addr = (unsigned long)(new_page - mem_map) << PAGE_SHIFT; /*page to phy*/  //page - mem_map is pfn
	printk("phy_addr is 0x%lx\n", phy_addr);
	page_back_phy = pfn_to_page(phy_addr >> PAGE_SHIFT);
	printk("back phy_addr is 0x%lx\n",  (unsigned long)(page_back_phy - mem_map) << PAGE_SHIFT);
	printk("\n");


	//
	return 0;

}

static void __exit filp_exit(void)
{
	printk("%s, %d\n", __func__, __LINE__);
	//kfree(kmalloc_addr);
	//free_page(gfp_addr);
	
}
module_init(filp_init);
module_exit(filp_exit);

MODULE_AUTHOR("wx");
MODULE_LICENSE("GPL");

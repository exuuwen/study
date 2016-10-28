#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/kmod.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/slab.h>

#define condition 0

static int __init main_init(void)
{	
	printk("in %s function\n", __func__);
	BUILD_BUG_ON(condition);/*if the macro "confition" is not zero, the program can not be compiled to success. so the BUILD_BUG_ON used for the build of program*/
    	
	WARN_ON(!condition);/*if the macro "confition" is not zero, there will print the satck information for this program. so the WARN_ON used for debugof program*/
	BUG_ON(condition);  /*if the macro "confition" is not zero, The kernel will occur an Oops errro "Unable to handle kernel NULL pointer dereference at virtual address 00000000". so the BUG_ON used for ending the bug program*/
	return 0;
}

static void __exit main_exit(void)
{
	printk("in %s function\n", __func__);
}

module_init(main_init);
module_exit(main_exit);
MODULE_LICENSE("GPL");



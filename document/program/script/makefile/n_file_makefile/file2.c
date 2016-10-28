//#include <linux/moduleloader.h>
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

extern void test();

static int __init main_init(void)
{
	printk(KERN_INFO "Hello World start\n");
	test();
    	return 0;
}

static void __exit main_exit(void)
{
	printk(KERN_INFO "Hello World end\n");
}

module_init(main_init);
module_exit(main_exit);
MODULE_LICENSE("GPL");



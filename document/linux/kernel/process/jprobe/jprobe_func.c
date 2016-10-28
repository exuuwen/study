#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/timer.h>
#include <linux/kallsyms.h>





int test_function(int *number, unsigned long size)
{
	printk(KERN_INFO "test_function: number:%d, size:%ld\n", *number, size);
	return *number;
}

EXPORT_SYMBOL(test_function);


static int __init test_init(void)
{	
	printk(KERN_INFO "test_init\n");

	
	return 0;
}

static void __exit test_exit(void)
{
	printk(KERN_INFO "test_exit\n");
}

module_init(test_init)
module_exit(test_exit)
MODULE_LICENSE("GPL");





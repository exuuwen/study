#include "test.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>

//extern void print_setup();
//extern void print_finit();
//extern int a;
static int __init init(void)
{
        print_setup();
	printk("a is %d\n", a);
	return 0;
}

static void __exit fini(void)
{
	printk("a is %d\n", a);
        print_finit();
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");

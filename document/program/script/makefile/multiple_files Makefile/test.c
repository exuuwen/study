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
int a = 110;
void print_setup()
{
	printk("we are in the print setup\n");
}
void print_finit()
{
	printk("we are in the print finit\n");
}

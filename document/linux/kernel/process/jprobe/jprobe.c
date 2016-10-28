#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/timer.h>
#include <linux/kallsyms.h>

#define TIME_OUT (jiffies + 3 * HZ)
static struct timer_list test_timer; // just for debug



extern int test_function(int *number, unsigned long size);
static int i = 0;

void test_timer_func(unsigned long __data)
{
	int ret = 0;
	
	ret = test_function(&i, 100);
	i++;
	mod_timer(&test_timer, TIME_OUT);
}
/* Proxy routine having the same arguments as actual do_fork() routine */
static long jtest_function(int *number, unsigned long size)
{
	printk(KERN_INFO "jprobe: number:%d, size:%ld\n", *number, size);
	//*number = 0;
	/* Always end with a call to jprobe_return(). */
	jprobe_return();
	return 0;
}

static struct jprobe my_jprobe = {
	.entry			= jtest_function,
	.kp = {
		.symbol_name	= "test_function",
	},
};

static int __init jprobe_init(void)
{
	int ret;
	
	setup_timer(&test_timer, test_timer_func, 0);
	test_timer.expires = TIME_OUT;

	ret = register_jprobe(&my_jprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
		return -1;
	}
	add_timer(&test_timer);
	printk(KERN_INFO "Planted jprobe at %p, handler addr %p\n",
	       my_jprobe.kp.addr, my_jprobe.entry);

	
	return 0;
}

static void __exit jprobe_exit(void)
{
	unregister_jprobe(&my_jprobe);
	del_timer_sync(&test_timer);
	printk(KERN_INFO "jprobe at %p unregistered\n", my_jprobe.kp.addr);
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");




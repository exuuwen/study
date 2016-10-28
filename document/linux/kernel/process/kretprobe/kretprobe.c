#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/timer.h>
#include <linux/kallsyms.h>
#include <linux/sched.h>
#include <linux/ktime.h>
#include <linux/limits.h>

#define TIME_OUT (jiffies + 3 * HZ)
static struct timer_list test_timer; // just for debug

extern int ktest_function(int *number, unsigned long size);
static int i = 0;

void test_timer_func(unsigned long __data)
{
	int ret = 0;
	printk(KERN_INFO "test_timer_func before kfunction_test\n");
	ret = ktest_function(&i, 100);
	i++;
	mod_timer(&test_timer, TIME_OUT);
}


static char func_name[NAME_MAX] = "ktest_function";
module_param_string(func, func_name, NAME_MAX, S_IRUGO);
MODULE_PARM_DESC(func, "Function to kretprobe; this module will report the"
			" function's execution time");


/* per-instance private data */
struct my_data {
	ktime_t entry_stamp;
};

/* Here we use the entry_hanlder to timestamp function entry */
static int entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	struct my_data *data;

	//if (!current->mm)
	//	return 1;	/* Skip kernel threads */
	printk(KERN_INFO "%s entry_handler\n", func_name);
	data = (struct my_data *)ri->data;
	data->entry_stamp = ktime_get();
	return 0;
}

/*
 * Return-probe handler: Log the return value and duration. Duration may turn
 * out to be zero consistently, depending upon the granularity of time
 * accounting on the platform.
 */
static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	int retval = regs_return_value(regs);
	struct my_data *data = (struct my_data *)ri->data;
	s64 delta;
	ktime_t now;

	now = ktime_get();
	delta = ktime_to_ns(ktime_sub(now, data->entry_stamp));
	printk(KERN_INFO "%s returned %d and took %lld ns to execute\n",
			func_name, retval, (long long)delta);
	return 0;
}


static struct kretprobe my_kretprobe = {
	.handler		= ret_handler,
	.entry_handler		= entry_handler,
	.data_size		= sizeof(struct my_data),
	/* Probe up to 20 instances concurrently. */
	.maxactive		= 20,
};

static int __init kretprobe_init(void)
{
	int ret;
	
	my_kretprobe.kp.symbol_name = func_name;
	ret = register_kretprobe(&my_kretprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_kretprobe failed, returned %d\n",
				ret);
		return -1;
	}
	if(strcmp(func_name, "ktest_function") == 0)
	{
		setup_timer(&test_timer, test_timer_func, 0);
		test_timer.expires = TIME_OUT;
		add_timer(&test_timer);
	}

	printk(KERN_INFO "Planted return probe at %s: %p\n",
			my_kretprobe.kp.symbol_name, my_kretprobe.kp.addr);
	return 0;
}

static void __exit kretprobe_exit(void)
{
	unregister_kretprobe(&my_kretprobe);
	printk(KERN_INFO "kretprobe at %p unregistered\n",
			my_kretprobe.kp.addr);
	if(strcmp(func_name, "ktest_function") == 0)
		del_timer_sync(&test_timer);
	/* nmissed > 0 suggests that maxactive was set too low. */
	printk(KERN_INFO "Missed probing %d instances of %s\n",
		my_kretprobe.nmissed, my_kretprobe.kp.symbol_name);
}

module_init(kretprobe_init)
module_exit(kretprobe_exit)
MODULE_LICENSE("GPL");

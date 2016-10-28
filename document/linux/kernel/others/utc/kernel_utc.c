#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/time.h> 

#include "utc.h"
static char time_str[100];

static int __init utc_init(void)
{
	struct timespec time_now;
	struct timeval time_val;
	int ret;

	printk("kernel utc init ---------------------------\n");
	rt_create_utc();
	time_now = current_kernel_time();
	ret = rt_get_time(time_str, time_now.tv_sec);
	if (ret == 0)
		printk("utc time:%s\n", time_str);

	do_gettimeofday(&time_val);
	ret = rt_get_time(time_str, time_val.tv_sec);
	if (ret == 0)
		printk("val utc time:%s\n", time_str);

	return 0;
}

/* Just before the module is removed, execute the stop routine. */
static void __exit utc_exit(void)
{	
	struct timespec time_now;
	struct timeval time_val;
	int ret;

	rt_set_utc_offset(-(3600*8));
	time_now = current_kernel_time();
	ret = rt_get_time(time_str, time_now.tv_sec);
	if (ret == 0)
		printk("utc time:%s\n", time_str);

	do_gettimeofday(&time_val);
	ret = rt_get_time(time_str, time_val.tv_sec);
	if (ret == 0)
		printk("val utc time:%s\n", time_str);

	rt_remove_utc();
	printk("kernel utc exit ---------------------------\n");
}

module_init(utc_init);
module_exit(utc_exit);

/* Some info to supply to modinfo */
MODULE_AUTHOR("wenx05124561@163.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel utc example");

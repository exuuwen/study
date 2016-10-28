#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>

#include "log.h"

void print(void)
{
	KERNEL_LOGGER_ERR("err\n");
	KERNEL_LOGGER_WARNING("warning\n");
	KERNEL_LOGGER_NOTICE("notice\n");
	KERNEL_LOGGER_INFO("info\n");
	KERNEL_LOGGER_DEBUG("debug\n");
	KERNEL_LOGGER_DEBUG2("debug2\n");
	KERNEL_LOGGER_DEBUG3("debug3\n");
}

static int __init log_init(void)
{
	printk("kernel log init ---------------------------\n");
	print();
	setloglevel(KernelWarning);	
	print();

	return 0;
}

/* Just before the module is removed, execute the stop routine. */
static void __exit log_exit(void)
{	
	setloglevel(KernelDebug3);	
	print();
	printk("kernel log exit ---------------------------\n");
}

module_init(log_init);
module_exit(log_exit)

/* Some info to supply to modinfo */
MODULE_AUTHOR("wenx05124561@163.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel log example");

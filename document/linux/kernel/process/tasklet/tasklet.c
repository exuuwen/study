#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>

static struct  tasklet_struct my_tasklet;

static void tasklet_handler (unsigned long data)
{
        printk("tasklet_handler is running.\n");
}

static int __init test_init(void)
{
	printk("tasklet is init.\n");
        tasklet_init(&my_tasklet,tasklet_handler,0);
        tasklet_schedule(&my_tasklet);
        return 0;
}

static  void __exit test_exit(void)
{
	tasklet_schedule(&my_tasklet);
        tasklet_kill(&my_tasklet);
        printk("test_exit is running.\n");
}
MODULE_LICENSE("GPL");

module_init(test_init);
module_exit(test_exit);

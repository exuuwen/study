#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/types.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/if.h>


#define DEFAULT_ARRAY_INT { [0 ... 2] = 10 }
static char *mycharp = "mycharp";
static int myint = 7;
static int myarray[3] = DEFAULT_ARRAY_INT;

int __init module_params_init(void)
{
    printk("my charp %s\n", mycharp);
    printk("myint %d\n", myint);
    printk("myarray[0] %d\n", myarray[0]);
    printk("myarray[1] %d\n", myarray[1]);
    printk("myarray[2] %d\n", myarray[2]);

    
    return 0;
}

void __exit module_params_exit(void)
{
}

module_param(mycharp, charp, S_IRUGO);
module_param(myint, int, S_IRUGO);
module_param_array(myarray, int, NULL, S_IRUGO);

module_init(module_params_init);
module_exit(module_params_exit);
MODULE_LICENSE("GPL");

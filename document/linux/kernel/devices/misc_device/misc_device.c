#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h> 
static ssize_t read(struct file *file, char __user *user, size_t size, loff_t *o)
{ 
	printk("it is read haha........\n");
	return 0;
}

static ssize_t write(struct file *file, const char __user *in, size_t size, loff_t *off)
{
	printk("it is write haha........\n");
	return 0;
} 

static struct file_operations mymisc_fops = { 
	.owner  = THIS_MODULE, 
	.write  = write, 
	.read  = read,
}; 

static struct miscdevice mymisc_dev = { 
	.minor = MISC_DYNAMIC_MINOR, 
	.name = "mymisc", 
	.fops = &mymisc_fops,
};
 
int __init mymisc_device_init(void)
{ 
	int ret;
	ret=misc_register(&mymisc_dev);
	printk("misc_register  ok ,ret is %d..\n",ret);
	return ret;
} 

void __exit mymisc_device_remove(void)
{ 
	misc_deregister(&mymisc_dev);
}
 
module_init(mymisc_device_init);
module_exit(mymisc_device_remove); 
MODULE_LICENSE("Dual BSD/GPL");

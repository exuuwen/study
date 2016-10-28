#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/version.h>
#include <linux/spinlock.h>

#include "ringbuf_mgmt.h"
#include "ringbuf_log.h"


static ssize_t ringbuf_read(struct file *file, char *buf, size_t count, loff_t *offset) 
{
	return rb_read(file, buf, count, offset);
}

static ssize_t ringbuf_write(struct file *file, const char *buf, size_t count, loff_t *offset) 
{
	return rb_write(file, buf, count, offset);
}

#ifdef CONFIG_COMPAT
static long ringbuf_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
	return rb_ioctl(file, cmd, arg);
}
#endif


static long ringbuf_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
	return rb_ioctl(file, cmd, arg);
}

static int ringbuf_open(struct inode *inode, struct file *file) 
{
	int res;

	res = nonseekable_open(inode, file);
	if (res < 0) 
	{
		printk(KERN_ERR "Failed to do nonseekable_open\n");
		return res;
	}

	/* Tag number is minor number DIV devices per tag and
	* device type is minor number MOD devices per tag */
	return rb_open(file, iminor(inode) / RB_DEV_PER_TAG, iminor(inode) % RB_DEV_PER_TAG);
}

static int ringbuf_release(struct inode *inode, struct file *file) 
{
	/* Remove warning */
	inode = inode;
	return rb_release(file);
}

static struct file_operations ringbuf_device_fops = {
  .owner = THIS_MODULE,
  .llseek = no_llseek,
  .read = ringbuf_read,
  .write = ringbuf_write,
  .unlocked_ioctl = ringbuf_ioctl,
#ifdef CONFIG_COMPAT
  .compat_ioctl = ringbuf_compat_ioctl,
#endif
  .open = ringbuf_open,
  .release = ringbuf_release
};

static struct cdev ringbuf_cdev;

static int __init ringbuf_init(void)
{
	int res;

	/* Set up non driver specific stuff */
	res = rb_init();
	if (res < 0) 
	{
		RINGBUF_LOGGER_ERR("Failed to do rb_init");
		rb_stop();
		return res;
	}

	/* Setting up internal char device. Do this last since after this
	 the devices are "live" */
	cdev_init(&ringbuf_cdev, &ringbuf_device_fops);
	ringbuf_cdev.owner = THIS_MODULE;
	res = cdev_add(&ringbuf_cdev, rb_get_dev_t(), RB_MAX_NO_TAGS * RB_DEV_PER_TAG);
	if (res < 0) 
	{
		RINGBUF_LOGGER_ERR("Failed to add internal char device\n");
		return res;
	}

	rb_proc_init();

	RINGBUF_LOGGER_INFO("Ringbuf character driver Registered\n");

	/*
	atomic_notifier_chain_register(&panic_notifier_list, &ringbuf_panic_notifier);
	register_klog_proc();
	printk(KERN_NOTICE "Registered panic notifier\n");
	*/

	return 0;
}

/* Just before the module is removed, execute the stop routine. */
static void __exit ringbuf_exit(void)
{
	//atomic_notifier_chain_register(&panic_notifier_list, &ringbuf_panic_notifier);
	rb_stop();
	cdev_del(&ringbuf_cdev);
	rb_proc_exit();
	RINGBUF_LOGGER_INFO("Ringbuf character driver Ungistered\n");
}

module_init(ringbuf_init);
module_exit(ringbuf_exit)

/* Some info to supply to modinfo */
MODULE_AUTHOR("xu.wen@ericsson.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Ringbuf log");


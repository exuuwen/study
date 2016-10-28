#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/device.h>

#define DEVS_LOGGER_ERR  printk
#define DEVS_LOGGER_INFO printk

#define DEV_NUM 	5
#define DEV_NAME "multi_dev"

typedef struct {
  dev_t dev;
  struct class* class; 
} Multi_dev;

static struct cdev multi_cdev;
static Multi_dev multi_dev;


static ssize_t devs_read(struct file *file, char *buf, size_t count, loff_t *offset) 
{
	return 0;
}

static ssize_t devs_write(struct file *file, const char *buf, size_t count, loff_t *offset) 
{
	return 0;
}

//#ifdef CONFIG_COMPAT
static long devs_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
	return 0;
}
//#endif

static long devs_ioctl(struct file *file, unsigned int cmd, unsigned long arg) 
{
	return 0;
}

static int devs_open(struct inode *inode, struct file *file) 
{
	int res;

	res = iminor(inode);
	DEVS_LOGGER_INFO("Multi devices open devices:%d\n", res);

	return 0;
}

static int devs_release(struct inode *inode, struct file *file) 
{
	return 0;
}

static struct file_operations devs_device_fops = {
  .owner = THIS_MODULE,
  .llseek = no_llseek,
  .read = devs_read,
  .write = devs_write,
  .unlocked_ioctl = devs_ioctl,
//#ifdef CONFIG_COMPAT
  .compat_ioctl = devs_compat_ioctl,
//#endif
  .open = devs_open,
  .release = devs_release
};

static int devs_create(void) 
{
	int i, res;
	struct device *d;

	/* Get major device number if not already allocated to me */
	if (0 == MAJOR(multi_dev.dev)) 
	{
		res = alloc_chrdev_region(&(multi_dev.dev), 0, DEV_NUM, DEV_NAME);
		if (0 != res) 
		{
			DEVS_LOGGER_ERR("create_devices(): Failed to alloc major device number\n");
			multi_dev.dev = MKDEV(0, 0);
			return res;
		}
	}

	/* Create a class for use by udev if not already created */
	if (NULL == multi_dev.class) 
	{
		multi_dev.class = class_create(THIS_MODULE, DEV_NAME);
	
		if (IS_ERR(multi_dev.class)) 
		{
			DEVS_LOGGER_ERR("Failed to create class %s\n", DEV_NAME);
			multi_dev.class = NULL;

			return PTR_ERR(multi_dev.class);
		}
	}

	for (i=0; i<DEV_NUM; i++) 
	{
		d = device_create(multi_dev.class, NULL, MKDEV(MAJOR(multi_dev.dev), i), NULL, "multi_dev_%d", i);
		if (IS_ERR(d)) 
		{
			DEVS_LOGGER_ERR("Failed to create class device\n");
			return PTR_ERR(d);
		}
	}

	return 0;
}

static int __init devs_init(void)
{
	int res;

	/* Set up non driver specific stuff */
	res = devs_create();
	if (res < 0) 
	{
		DEVS_LOGGER_ERR("Failed to do devs_init");
		return res;
	}

	cdev_init(&multi_cdev, &devs_device_fops);
	multi_cdev.owner = THIS_MODULE;
	res = cdev_add(&multi_cdev, multi_dev.dev, DEV_NUM);
	if (res < 0) 
	{
		DEVS_LOGGER_ERR("Failed to add internal char device\n");
		return res;
	}

	DEVS_LOGGER_INFO("Multi devices demo character driver Registered\n");

	return 0;
}

static void devs_destroy(void)
{
	int i;

	for (i=0; i<DEV_NUM; i++) 
	{
		device_destroy(multi_dev.class, MKDEV(MAJOR(multi_dev.dev), i));
	}

	if (NULL != multi_dev.class) 
	{
		class_destroy(multi_dev.class);
		multi_dev.class = NULL;
	}

	if (0 != MAJOR(multi_dev.dev)) 
	{
		unregister_chrdev_region(multi_dev.dev, DEV_NUM);
		multi_dev.dev = MKDEV(0, 0);
	}
}


static void __exit devs_exit(void)
{
	devs_destroy();
	cdev_del(&multi_cdev);
	
	DEVS_LOGGER_INFO("Multi devices demo character driver Ungistered\n");
}

module_init(devs_init);
module_exit(devs_exit)

/* Some info to supply to modinfo */
MODULE_AUTHOR("wenx05124561@163.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Multi devices demo");


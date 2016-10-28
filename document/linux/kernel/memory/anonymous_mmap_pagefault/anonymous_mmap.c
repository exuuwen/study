#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <linux/mman.h>
#include <asm/atomic.h>
#include <linux/cdev.h>

#define MAPLEN PAGE_SIZE*10
struct mapdrv{
	struct cdev mapdev;
	atomic_t usage;
	/*Since this is read-only, we don't need sem or locks.*/
};

extern struct mm_struct init_mm;
static int major;		/*major number of device */
struct mapdrv* md;



/* device open method */
static int mapdrv_open(struct inode *inode, struct file *file)
{
	struct mapdrv *md;
	md = container_of(inode->i_cdev, struct mapdrv, mapdev);
	atomic_inc(&md->usage);
	return 0;
}

/* device close method */
static int mapdrv_release(struct inode *inode, struct file *file)
{
	struct mapdrv* md;
	md = container_of(inode->i_cdev, struct mapdrv, mapdev);
	atomic_dec(&md->usage);
	return 0;
}

static int mapdrv_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long size = vma->vm_end - vma->vm_start;
	
	if (offset + size > MAPLEN) 
	{
		printk("size too big\n");
		return -ENXIO;
	}

	return 0;
}

static struct file_operations mapdrv_fops = {
	.owner = THIS_MODULE,
	.mmap = mapdrv_mmap,
	.open = mapdrv_open,
	.release = mapdrv_release,
};



static int __init mapdrv_init(void)
{
	int result, err;
	dev_t dev = 0;
	
	md = kmalloc(sizeof(struct mapdrv), GFP_KERNEL);
	if (!md)
		goto fail1;
	result = alloc_chrdev_region(&dev, 0, 1, "mapdrv1");
	major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "mapdrv: can't get major %d\n", major);
		goto fail2;
	}
	cdev_init(&md->mapdev, &mapdrv_fops);
	md->mapdev.owner = THIS_MODULE;
	md->mapdev.ops = &mapdrv_fops;
	err = cdev_add (&md->mapdev, dev, 1);
	if (err) 
	{
		printk(KERN_NOTICE "Error %d adding mapdrv", err);
		goto fail3;
	}
	atomic_set(&md->usage, 0);
	
	return 0;
fail3:
	unregister_chrdev_region(dev, 1);
fail2:
	kfree(md);
fail1:
	return -1;
}

static void __exit mapdrv_exit(void)
{
	dev_t devno = MKDEV(major, 0);
	
	cdev_del(&md->mapdev);
	/* unregister the device */
	unregister_chrdev_region(devno, 1);
	kfree(md);
}



MODULE_LICENSE("GPL");
module_init(mapdrv_init);
module_exit(mapdrv_exit);


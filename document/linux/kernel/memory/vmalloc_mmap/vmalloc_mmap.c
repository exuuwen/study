/*======================================================================
    A globalmem driver as an example of char device drivers  
    This example is to introduce how to use locks to avoid race conditions
    
    The initial developer of the original code is Baohua Song
    <author@linuxdriver.cn>. All Rights Reserved.
======================================================================*/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <linux/mman.h>
#include <linux/slab.h>

#define PAGESIZE    4096
#define PAGENUM	    10
#define BUF_SIZE	(PAGESIZE*PAGENUM)	
#define MEM_CLEAR 0x1 
#define GLOBALMEM_MAJOR 234   

unsigned long off; 
char *test="hello world!";
static int globalmem_major = GLOBALMEM_MAJOR;

struct globalmem_dev                                     
{                                                        
  struct cdev cdev;                      
  char* buff;        
};

struct globalmem_dev *globalmem_devp; 


int globalmem_open(struct inode *inode, struct file *filp)
{
	filp->private_data = globalmem_devp;
	printk("globalmem_open is called now.\n");
	return 0;
}


int globalmem_release(struct inode *inode, struct file *filp)
{
	printk("globalmem_release is called now\n");
	printk("the coentens is off :%ld close :%s", off ,globalmem_devp->buff + off*PAGE_SIZE);
	return 0;
}


static int globalmem_mmap(struct file *filp,struct vm_area_struct *vma)
{
	int ret;
	unsigned long start = vma->vm_start;
	unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);
	
	off = vma->vm_pgoff;

	printk("mmap is called now and the start is 0x%lx the size is 0x%lx, offset is 0x%lx\n", start, size, vma->vm_pgoff);
	if(size > BUF_SIZE)
		return -EINVAL;

	ret = remap_vmalloc_range(vma, (void *)globalmem_devp->buff,  vma->vm_pgoff);
	if(ret)
	{
		printk("mmap remap_vmalloc_range failed:%d\n", ret);
		return(-EAGAIN);
  }
	strcpy(globalmem_devp->buff + PAGE_SIZE*vma->vm_pgoff, test);
	printk("the coentens is in mmap :%s",globalmem_devp->buff + PAGE_SIZE*vma->vm_pgoff);

	return 0;
}

static const struct file_operations globalmem_fops =
{
	.owner = THIS_MODULE,
	.open = globalmem_open,
	.release = globalmem_release,
	.mmap=globalmem_mmap,
};


static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
	int err, devno = MKDEV(globalmem_major, index);

	cdev_init(&dev->cdev, &globalmem_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &globalmem_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}


int globalmem_init(void)
{
	int result;
	//unsigned long virt_addr;
	dev_t devno = MKDEV(globalmem_major, 0);


	if(globalmem_major)
		result = register_chrdev_region(devno, 1, "globalmem");
	else  
	{
		result = alloc_chrdev_region(&devno, 0, 1, "globalmem");
		globalmem_major = MAJOR(devno);
	}  
	if (result < 0)
		return result;


	globalmem_devp = kmalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
	if (!globalmem_devp)   
	{
		result =  - ENOMEM;
		goto fail_malloc;
	}
	memset(globalmem_devp, 0, sizeof(struct globalmem_dev));
	globalmem_devp->buff = vmalloc_user(BUF_SIZE);
	
	globalmem_setup_cdev((void*)globalmem_devp, 0);   

	return 0;

	fail_malloc: unregister_chrdev_region(devno, 1);

	return result;
}


void globalmem_exit(void)
{
	cdev_del(&globalmem_devp->cdev);  
	vfree(globalmem_devp->buff); 
	kfree(globalmem_devp);     
	unregister_chrdev_region(MKDEV(globalmem_major, 0), 1); 
}


MODULE_AUTHOR("Xu Wen");
MODULE_LICENSE("Dual BSD/GPL");

module_param(globalmem_major, int, S_IRUGO);

module_init(globalmem_init);
module_exit(globalmem_exit);


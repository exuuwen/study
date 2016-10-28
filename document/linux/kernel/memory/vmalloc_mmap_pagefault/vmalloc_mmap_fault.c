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

struct mapdrv{
	struct cdev mapdev;
	atomic_t usage;
	/*Since this is read-only, we don't need sem or locks.*/
};

#define MAPLEN (PAGE_SIZE*10)

extern struct mm_struct init_mm;
static void *vmalloc_area = NULL;
static int major;		/*major number of device */
struct mapdrv* md;


/* open handler for vm area */
static void map_vopen(struct vm_area_struct *vma)
{
	/* needed to prevent the unloading of the module while
	   somebody still has memory mapped */
}

/* close handler form vm area */
static void map_vclose(struct vm_area_struct *vma)
{
}

/* page fault handler */
static int map_vfault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	unsigned long offset = vmf->pgoff;
	/* determine the offset within the vmalloc'd area  */
	//offset = address - vma->vm_start + (vma->vm_pgoff << PAGE_SHIFT);
	/* translate the vmalloc address to kmalloc address  */
	
	/* increment the usage count of the page */
	vmf->page = vmalloc_to_page(vmalloc_area + (offset << PAGE_SHIFT));
	get_page(vmf->page);
	printk("map_drv: page fault for offset 0x%lx (kseg 0x%p)\n", offset, vmalloc_area);

	return 0;
}

static struct vm_operations_struct map_vm_ops = {
	.open = map_vopen,
	.close = map_vclose,
	.fault = map_vfault,
};


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

	/*  only support shared mappings. */
	if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED)) 
	{
		printk("writeable mappings must be shared, rejecting\n");
		return -EINVAL;
	}
	/* do not want to have this area swapped out, lock it */
	vma->vm_flags |= VM_LOCKED;
	vma->vm_ops = &map_vm_ops;
	/* call the open routine to increment the usage count */
	map_vopen(vma);

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
	int i, result, err;
	dev_t dev = 0;
	unsigned long addr = 0;
	
	md = kmalloc(sizeof(struct mapdrv), GFP_KERNEL);
	if (!md)
		goto fail1;
	result = alloc_chrdev_region(&dev, 0, 1, "mapdrv0");
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
	/* get a memory area that is only virtual contigous. */
	vmalloc_area = vmalloc_user(MAPLEN);
	if (!vmalloc_area)
		goto fail4;
	/* set a hello message to kernel space for read by user */
	addr = (unsigned long)vmalloc_area;
	for (i=0; i<10; i++)
	{
		sprintf((char *)addr, "hello world from kernel space %d!", i);
		addr += PAGE_SIZE;
	}
	printk("vmalloc_area at 0x%p (phys 0x%lx)\n", vmalloc_area, page_to_pfn(vmalloc_to_page(vmalloc_area)) << PAGE_SHIFT);
	return 0;
fail4:
	cdev_del(&md->mapdev);	
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
	
	/* and free the two areas */
	if (vmalloc_area)
		vfree(vmalloc_area);
	cdev_del(&md->mapdev);
	/* unregister the device */
	unregister_chrdev_region(devno, 1);
	kfree(md);
}



MODULE_LICENSE("GPL");
module_init(mapdrv_init);
module_exit(mapdrv_exit);


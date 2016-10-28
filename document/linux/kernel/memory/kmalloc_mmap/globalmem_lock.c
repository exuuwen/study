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

#define PAGESIZE    4096
#define PAGENUM	    1
#define BUF_SIZE	(PAGESIZE*PAGENUM)	
#define MEM_CLEAR 0x1 
#define GLOBALMEM_MAJOR 234    
char *test="hello world!\n";
static int globalmem_major = GLOBALMEM_MAJOR;

struct globalmem_dev                                     
{                                                        
  struct cdev cdev;                      
  char* buff;     
  struct semaphore sem;    
};

struct globalmem_dev *globalmem_devp; 


int globalmem_open(struct inode *inode, struct file *filp)
{
  
  filp->private_data = globalmem_devp;
  printk("globalmem_open is called now......................\n");
  printk("the coentens is in open :........%s",globalmem_devp->buff);
  return 0;
}


int globalmem_release(struct inode *inode, struct file *filp)
{
  printk("globalmem_release is called now......................\n");
  return 0;
}

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size,
  loff_t *ppos)
{
  unsigned long p =  *ppos;
  unsigned int count = size;
  int ret = 0;
  struct globalmem_dev *dev = filp->private_data; /*获得设备结构体指针*/

  /*分析和获取有效的写长度*/
  if (p >= BUF_SIZE)
    return count ?  - ENXIO: 0;
  if (count > BUF_SIZE - p)
    count = BUF_SIZE - p;

  if (down_interruptible(&dev->sem))
  {
    return  - ERESTARTSYS;
  }

  /*内核空间->用户空间*/
  if (copy_to_user(buf, (void*)(dev->buff + p), count))
  {
    ret =  - EFAULT;
  }
  else
  {
    *ppos += count;
    ret = count;

    printk(KERN_INFO "read %d bytes(s) from %ld\n", count, p);
  }
  up(&dev->sem); //释放信号量

  return ret;
}

/*写函数*/
static ssize_t globalmem_write(struct file *filp, const char __user *buf,
  size_t size, loff_t *ppos)
{
  unsigned long p =  *ppos;
  unsigned int count = size;
  int ret = 0;
  struct globalmem_dev *dev = filp->private_data; /*获得设备结构体指针*/

  /*分析和获取有效的写长度*/
  if (p >= BUF_SIZE)
    return count ?  - ENXIO: 0;
  if (count > BUF_SIZE - p)
    count = BUF_SIZE - p;

  if (down_interruptible(&dev->sem))//获得信号量
  {
    return  - ERESTARTSYS;
  }
  /*用户空间->内核空间*/
  if (copy_from_user(dev->buff + p, buf, count))
    ret =  - EFAULT;
  else
  {
    *ppos += count;
    ret = count;

    printk(KERN_INFO "written %d bytes(s) from %ld\n", count, p);
  }
  up(&dev->sem); //释放信号量
  return ret;
}

/* seek文件定位函数 */
/*static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
{
  loff_t ret = 0;
  switch (orig)
  {
    case 0:   
      if (offset < 0)
      {
        ret =  - EINVAL;
        break;
      }
      if ((unsigned int)offset > GLOBALMEM_SIZE)
      {
        ret =  - EINVAL;
        break;
      }
      filp->f_pos = (unsigned int)offset;
      ret = filp->f_pos;
      break;
    case 1:   
      if ((filp->f_pos + offset) > GLOBALMEM_SIZE)
      {
        ret =  - EINVAL;
        break;
      }
      if ((filp->f_pos + offset) < 0)
      {
        ret =  - EINVAL;
        break;
      }
      filp->f_pos += offset;
      ret = filp->f_pos;
      break;
    default:
      ret =  - EINVAL;
      break;
  }
  return ret;
}*/
/*static int globalmem_ioctl(struct inode *inodep, struct file *filp, unsigned
  int cmd, unsigned long arg)
{
  struct globalmem_dev *dev = filp->private_data; 
  switch (cmd)
  {
    case MEM_CLEAR:
      if (down_interruptible(&dev->sem))
      {
        return  - ERESTARTSYS;
      }
      memset(dev->mem, 0, GLOBALMEM_SIZE);
      up(&dev->sem); 

      printk(KERN_INFO "globalmem is set to zero\n");
      break;

    default:
      return  - EINVAL;
  }
  return 0;
}*/

static int globalmem_mmap(struct file *filp,struct vm_area_struct *vma)
{
  unsigned long pos,phys;
  unsigned long start=vma->vm_start;
  unsigned long size=(unsigned long)(vma->vm_end-vma->vm_start);
  unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
  printk("mmap is called now and the start is %lx the size is %lx and offset is %lx......................\n",start,size,offset);
  if(size>BUF_SIZE)
	return -EINVAL;
  pos=(unsigned long)globalmem_devp->buff + offset;

  //phys=page_address(vmalloc_to_page(pos));
   phys=virt_to_phys((void*)pos);//kmalloc
  if(remap_pfn_range(vma,start,phys>>PAGE_SHIFT,size,vma->vm_page_prot))
	return -EAGAIN;
  
  return 0;
}

static const struct file_operations globalmem_fops =
{
  .owner = THIS_MODULE,
  //.llseek = globalmem_llseek,
  .read = globalmem_read,
  .write = globalmem_write,
 // .ioctl = globalmem_ioctl,
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
/*volatile void *vaddr_to_kaddr(volatile void *address)
{
	pgd_t *pgd;
	pmd_t *pmd;
	pte_t *ptep, pte;
	unsigned long va, ret = 0UL;
	va = (unsigned long)address;
	// get the page directory. Use the kernel memory map. 
	pgd = pgd_offset_k(va);
	// check whether we found an entry 
	if (!pgd_none(*pgd)) {
		// get the page middle directory 
		pmd = pmd_offset(pgd, va);
		// check whether we found an entry 
		if (!pmd_none(*pmd)) {
			//get a pointer to the page table entry 
			ptep = pte_offset_kernel(pmd, va);
			pte = *ptep;
			//check for a valid page 
			if (pte_present(pte)) {
				// get the address the page is refering to 
				ret =
				    (unsigned long)page_address(pte_page(pte));
				///add the offset within the page to the page address 
				ret |= (va & (PAGE_SIZE - 1));
			}
		}
	}
	return ((volatile void *)ret);
}*/

int globalmem_init(void)
{
  struct page * page;
  int result;
  //unsigned long virt_addr;
  dev_t devno = MKDEV(globalmem_major, 0);

 
  if (globalmem_major)
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
  globalmem_devp->buff=kmalloc(BUF_SIZE,GFP_KERNEL);
  strcpy(globalmem_devp->buff,test);
 printk("the coentens is in init :........%s ",globalmem_devp->buff);
  for(page=virt_to_page(globalmem_devp->buff);page<virt_to_page(globalmem_devp->buff+BUF_SIZE);page++)
	SetPageReserved(page);//kmalloc
  /*for (virt_addr = (unsigned long)globalmem_devp->buff;
	     virt_addr < (unsigned long)(&(globalmem_devp[BUF_SIZE / sizeof(int)]));
	     virt_addr += PAGE_SIZE) {

		SetPageReserved(virt_to_page
				(vaddr_to_kaddr((void *)virt_addr)));
	}
  printk("vmalloc_area at 0x%p (phys 0x%lx)\n", globalmem_devp->buff,
	       virt_to_phys((void *)vaddr_to_kaddr(globalmem_devp->buff)));*/
  globalmem_setup_cdev((void*)globalmem_devp, 0);
  init_MUTEX(&globalmem_devp->sem);     
 
  return 0;

  fail_malloc: unregister_chrdev_region(devno, 1);
  return result;
}


void globalmem_exit(void)
{
  cdev_del(&globalmem_devp->cdev);  
  kfree(globalmem_devp->buff); 
  kfree(globalmem_devp);     
  unregister_chrdev_region(MKDEV(globalmem_major, 0), 1); 
}


MODULE_AUTHOR("Song Baohua");
MODULE_LICENSE("Dual BSD/GPL");

module_param(globalmem_major, int, S_IRUGO);

module_init(globalmem_init);
module_exit(globalmem_exit);

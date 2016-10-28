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
#include <linux/pagemap.h>
#include <asm/pgtable.h>
#include <asm/page.h>

#define GLOBALMEM_SIZE	0x1000	
#define MEM_CLEAR 0x1  
#define GLOBALMEM_MAJOR 235    
struct ioctl_arg
{
        pid_t pid;
        unsigned long vaddr;
};

#define KMAP_IOC_MAGIC        'q'
#define KMAP_IOC_SET_PID_AND_VADDR        _IOW(KMAP_IOC_MAGIC, 1, char *)

static int globalmem_major = GLOBALMEM_MAJOR;

struct globalmem_dev                                     
{                                                        
  struct cdev cdev;                     
  unsigned char mem[GLOBALMEM_SIZE];      
  struct semaphore sem;    
};

struct globalmem_dev *globalmem_devp; 


int globalmem_open(struct inode *inode, struct file *filp)
{
  
  filp->private_data = globalmem_devp;
  return 0;
}


int globalmem_release(struct inode *inode, struct file *filp)
{
  return 0;
}

static struct page *
my_follow_page(struct vm_area_struct *vma, unsigned long addr)
{
		pgd_t *pgd;
                pud_t *pud;
                pmd_t *pmd;
                pte_t *pte;
                spinlock_t *ptl;
        struct page *page = NULL;
        struct mm_struct *mm = vma->vm_mm;
                pgd = pgd_offset(mm, addr);
                if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd))) {
                        goto out;
        }
                pud = pud_offset(pgd, addr);
                if (pud_none(*pud) || unlikely(pud_bad(*pud)))
                        goto out;
                pmd = pmd_offset(pud, addr);
                if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd))) {
                        goto out;
        }
                pte = pte_offset_map_lock(mm, pmd, addr, &ptl);
                if (!pte)
                        goto out;
        if (!pte_present(*pte))
            goto unlock;
        page = pfn_to_page(pte_pfn(*pte));
            if (!page)
            goto unlock;
        get_page(page);
unlock:
        pte_unmap_unlock(pte, ptl);
out:
        return page;
}

static int globalmem_ioctl(struct inode *inodep, struct file *filp, unsigned
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
    case  KMAP_IOC_SET_PID_AND_VADDR :
	{
	pid_t pid = 0;
        unsigned long vaddr = 0;
	int err = 0, retval = 0;
	 struct ioctl_arg k_arg;

        __memset_generic(&k_arg, 0, sizeof(struct ioctl_arg));

        /*get arg*/
        err = !access_ok(VERIFY_READ, arg, sizeof(struct ioctl_arg));
        if ( err )
        {
                printk("access vaild failed\n");
                return -EFAULT;
        }

        err = copy_from_user((void *)&k_arg, (const void *)arg,
                                                        sizeof(struct ioctl_arg));
        pid = k_arg.pid;
        vaddr = k_arg.vaddr;

        printk("pid:%d vaddr:%lx\n", pid, vaddr);

	struct vm_area_struct *vma;
    struct mm_struct *mm = current->mm;
    struct page *page;
    unsigned long kernel_addr;
    printk("mtest_write_val\n");
    down_read(&mm->mmap_sem);
    vma = find_vma(mm, vaddr);
    if (vma && vaddr >= vma->vm_start && (vaddr + 15) < vma->vm_end) {
        if (!(vma->vm_flags & VM_WRITE)) {
            printk("vma is not writable for 0x%lx\n", vaddr);
            goto out;
        }
        page = my_follow_page(vma, vaddr);
        if (!page) {    
            printk("page not found  for 0x%lx\n", vaddr);
            goto out;
        }
        
        kernel_addr = (unsigned long)page_address(page);
        kernel_addr += (vaddr&~PAGE_MASK);
       // printk("write 0x%lx to address 0x%lx\n", val, kernel_addr);
       // *(unsigned long *)kernel_addr = val;
        put_page(page);
	printk("kernel addr is %lx \n", kernel_addr);
        strcpy((char *) kernel_addr, "hello kmap messi rony");
	
        return 0;
       
    } else {
        printk("no vma found for %lx\n", vaddr);
    }
	out:
    	up_read(&mm->mmap_sem);
	return  - EINVAL;

        /*convert phy to kernel logic addr*/
       // laddr = __va(phy);
	
	}
    default:
      return  - EINVAL;
  }
	return 0;
}


static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size,
  loff_t *ppos)
{
  unsigned long p =  *ppos;
  unsigned int count = size;
  int ret = 0;
  struct globalmem_dev *dev = filp->private_data; 

  
  if (p >= GLOBALMEM_SIZE)
    return count ?  - ENXIO: 0;
  if (count > GLOBALMEM_SIZE - p)
    count = GLOBALMEM_SIZE - p;

  if (down_interruptible(&dev->sem))
  {
    return  - ERESTARTSYS;
  }

  
  if (copy_to_user(buf, (void*)(dev->mem + p), count))
  {
    ret =  - EFAULT;
  }
  else
  {
    *ppos += count;
    ret = count;

    printk(KERN_INFO "read %d bytes(s) from %ld\n", count, p);
  }
  up(&dev->sem); 

  return ret;
}


static ssize_t globalmem_write(struct file *filp, const char __user *buf,
  size_t size, loff_t *ppos)
{
  unsigned long p =  *ppos;
  unsigned int count = size;
  int ret = 0;
  struct globalmem_dev *dev = filp->private_data; 

  
  if (p >= GLOBALMEM_SIZE)
    return count ?  - ENXIO: 0;
  if (count > GLOBALMEM_SIZE - p)
    count = GLOBALMEM_SIZE - p;

  if (down_interruptible(&dev->sem))
  {
    return  - ERESTARTSYS;
  }
  
  if (copy_from_user(dev->mem + p, buf, count))
    ret =  - EFAULT;
  else
  {
    *ppos += count;
    ret = count;

    printk(KERN_INFO "written %d bytes(s) from %ld\n", count, p);
  }
  up(&dev->sem); 
  return ret;
}


static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
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
}


static const struct file_operations globalmem_fops =
{
  .owner = THIS_MODULE,
  .llseek = globalmem_llseek,
  .read = globalmem_read,
  .write = globalmem_write,
  .ioctl = globalmem_ioctl,
  .open = globalmem_open,
  .release = globalmem_release,
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
  
  globalmem_setup_cdev(globalmem_devp, 0);
  init_MUTEX(&globalmem_devp->sem);   
  return 0;

  fail_malloc: unregister_chrdev_region(devno, 1);
  return result;
}


void globalmem_exit(void)
{
  cdev_del(&globalmem_devp->cdev);  
  kfree(globalmem_devp);     
  unregister_chrdev_region(MKDEV(globalmem_major, 0), 1); 
}

MODULE_AUTHOR("Song Baohua");
MODULE_LICENSE("Dual BSD/GPL");

module_param(globalmem_major, int, S_IRUGO);

module_init(globalmem_init);
module_exit(globalmem_exit);

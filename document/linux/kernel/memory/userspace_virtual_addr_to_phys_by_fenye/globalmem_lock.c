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
        unsigned long phy = 0;
        unsigned long laddr= 0;
	unsigned long laddr1=0;
        int err = 0, retval = 0;
	//struct task_struct *pcb_tmp = NULL;
        pgd_t *pgd = NULL;
        pud_t *pud = NULL;
        pmd_t *pmd = NULL;
        pte_t *pte = NULL;

        struct page *mypage;

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
        /*
        *convert vaddr to kernel logic addr
        *1,get the current task by pid
        *2,get the phyaddr
        *3,convert phyaddr to kernel logic addr
        *4,write sth. in the buffer

        *NOTE:step 1 is not necessay,because now current is pointed to task
        */

        printk("The pid which user passed in is:%d\n"
                                " The current pid is:%d\n", pid, current->pid);
	 /*if(!(pcb_tmp = find_task_by_vpid(pid))) 
	{
		printk(KERN_INFO"Can't find the task %d .\n",pid);
		return 0;
	}
	// 判断给出的地址va是否合法(va<vm_end)
	 if(!find_vma(pcb_tmp->mm,vaddr))
	{
		printk(KERN_INFO"virt_addr 0x%lx not available.\n",vaddr);
		return 0;
	}
        
        pgd = pgd_offset(pcb_tmp->mm, vaddr);*///we can find other pid task_struct like this
	/*get phyaddr*/
	 pgd = pgd_offset(current->mm, vaddr);
        if ( pgd_none(*pgd) || pgd_bad(*pgd) )
        {
                printk("invalid pgd\n");
                retval = -1;
                goto error;
        }

        pud = pud_offset(pgd, vaddr);
        if ( pud_none(*pud) || pud_bad(*pud) )
        {
                printk("invalid pud\n");
                retval = -1;
                goto error;
        }

        pmd = pmd_offset(pud, vaddr);
        if ( pmd_none(*pmd) || pmd_bad(*pmd) )
        {
                printk("invalid pmd\n");
                retval = -1;
                goto error;
        }
        printk("before pte_offset_map\n");


        pte = pte_offset_map(pmd, vaddr);

        printk("after pte_offset_map\n");
        pte_unmap(pte);
        if ( pte_none(*pte) )
        {
                printk("bad pte va: %lX pte: %p pteval: %lX\n", vaddr, pte, pte_val(*pte));
                retval = -1;
                goto error;
        }

        phy = (pte_val(*pte) & PAGE_MASK)|(vaddr & ~PAGE_MASK);//way 2

       // printk("the phy addr is %lx\n", phy);//before

        mypage = pte_page(*pte);
        set_bit(PG_locked, &mypage->flags);
        atomic_inc(&mypage->_count);

        /*convert phy to kernel logic addr*/
        laddr1 = __va(phy);//way 2  laddr1=laddr
	laddr=(unsigned long)page_address(mypage);
	laddr+=(vaddr & ~PAGE_MASK);
	printk("kernel addr is %lx addr1 is %lx\n",laddr,laddr1);
        strcpy((char *)laddr, "hello kmap messi rony");

        error:
        return retval;

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

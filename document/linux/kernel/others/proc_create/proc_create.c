#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/log2.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/poll.h>
#include <linux/time.h>
MODULE_LICENSE("Dual BSD/GPL");
//#define atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0)
/*struct _test_msg{
	int one;
	int two;
	int three;
};*/
#define WX_PROCDIR                           "wx_procdir"
#define WX_KEY					"wx_key"
//#define DPI_IPHEAD_KFIFOSIZE				"dpi_iphead_kfifosize"
static  unsigned long key=0x12345678;
static struct proc_dir_entry *proc_entry;
//static struct proc_dir_entry *proc_entry2;
struct proc_dir_entry *wx_procdir ;



ssize_t wx_proc_read(char *page, char **start, off_t off, int count,
  int*eof, void *data)  {
	int len;
	
	if (off > 0)
	{
		*eof = 1;
		return 0;
	}
	len=sprintf(page, "%lx\n", key);
	printk("in the read wxwxwx ahhah\n");
	return len;
}



ssize_t wx_proc_write(struct file *filp, const char __user *buff, unsigned
  long len, void *data)
{
  #define MAX_UL_LEN 8
  char k_buf[MAX_UL_LEN];
  char *endp;
  unsigned long new;
  int count;
  int ret;

	printk("in the write hahahha\n");
	if(MAX_UL_LEN<=len)
	count=MAX_UL_LEN;
	else count=len;

 

  if (copy_from_user(k_buf, buff, count))
  
  {
    ret =  - EFAULT;
    goto err;
  }
  else
  {
   /* new = simple_strtoul(k_buf, &endp, 16); 
    if (endp == k_buf)
 
    {
      ret =  - EINVAL;
      goto err;
	printk("input error\n");
    }*/
	sscanf(k_buf,"%lx",&key);
	
   // key = new;
	printk("haha now key is %lx\n",key);
    return count;
  }
  err:
  return ret;
}


int  test_proc_init(void) {
	
	wx_procdir=proc_mkdir(WX_PROCDIR , NULL);
	proc_entry=create_proc_entry(WX_KEY,0666, wx_procdir); 
	//proc_entry2=create_proc_entry(DPI_IPHEAD_KFIFOSIZE,0666, dpi_iphfifo_procdir); 

	proc_entry->read_proc = wx_proc_read;
	proc_entry->write_proc = wx_proc_write;
	printk("create  dpi_iphkfifo_proc done ...\n");
	return 0;
}


void  test_proc_exit(void)
{
	//remove_proc_entry(DPI_IPHEAD_KFIFOADDR, dpi_iphfifo_procdir);
	remove_proc_entry(WX_KEY, wx_procdir);
	remove_proc_entry(WX_PROCDIR, NULL);
	printk("removing dpi_kfifo_proc_dir done ...\n");
}










static int __init init_test(void) {
	printk("## wx test in mmp_kernel proc_create ##\n");
	//msg_init();	test_proc_init();

	return 0;
}

static void __exit exit_test(void) {
	printk("## wx test in exit  mmp_kernel proc_create ##\n");
	//msg_exit();
	test_proc_exit();	
}


module_init(init_test);
module_exit(exit_test);
MODULE_AUTHOR("wenxu");

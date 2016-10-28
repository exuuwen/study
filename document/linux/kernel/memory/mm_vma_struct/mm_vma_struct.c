/****************************************************************
*
* FILENAME
*   ca_dev.c
*
* PURPOSE
*   This sample program is the sample code to make it indicate
*   a sample image with OSD by using DTV solution Board manufactured
*   by NEC electronics which EMMA3TL was carried on.
*
*   This program uses that as a work buffer on the assumption
*   that the space of 0xA8000000 - 0xAFFFFFFF is un-use.
*
* AUTHOR
*   Dragontec
*
* DATE
*   21 Apr 2010
*
* HISTORY
*   2010.04.21  Rev.1.0  1st revision
*
****************************************************************/



/****************************************************************/
/*          INCLUDE FILES                                       */
/****************************************************************/
#include <linux/init.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/cacheflush.h>
#include <linux/semaphore.h>
#include <linux/pagemap.h>
#include <asm/pgtable.h>
#include <asm/page.h>






/****************************************************************/
/*          MACROS/DEFINES                                      */
/****************************************************************/
#define CADEV_MAJOR                 (227)
#define CADEV_NAME                  "ca"

#define WX_DEBUG
#ifdef WX_DEBUG
#define PRINT_WX(frmt, args...)            printk(frmt, ## args)
#else
#define PRINT_WX(frmt, args...)
#endif



		
       	
static int ca_dev_open( struct inode    *psInode,
                        struct file     *psFile )
{
	int ret=0;
	int num = 0;
	//nDev_minor = MINOR( psInode->i_rdev );
	struct vm_area_struct *vma;
	struct mm_struct *mm = current->mm;

	printk("%s\n", __func__);
	if(mm == current->active_mm)
		printk("mm == current->active_mm hahhaha\n");
	
	printk("vm info.....................\n");
	printk("total_vm is %ld\n", mm->total_vm);
	printk("locked_vm is %ld\n", mm->locked_vm);
	printk("shared_vm is %ld\n", mm->shared_vm);
	printk("exec_vm is %ld\n", mm->exec_vm);
	
	printk("code and data info.............\n");
	printk("start_code is 0x%lx\n", mm->start_code);
	printk("end_code is 0x%lx\n", mm->end_code);
	printk("start_data is 0x%lx\n", mm->start_data);
	printk("end_data is 0x%lx\n", mm->end_data);

	printk("heap and stack info.............\n");
	printk("start_brk is 0x%lx\n", mm->start_brk);
	printk("brk is 0x%lx\n", mm->brk);
	printk("start_stack is 0x%lx\n", mm->start_stack);
	//printk("start_code is 0x%lx\n", mm->start_code);

	printk("arg and env env.............\n");
	printk("arg_start is 0x%lx\n", mm->arg_start);
	printk("arg_end is 0x%lx\n", mm->arg_end);
	printk("env_start is 0x%lx\n", mm->env_start);
	printk("env_end is 0x%lx\n", mm->env_end);
	num = 1;
	vma = mm->mmap;
	while(vma)
	{
		printk("vma num is %d\n", num);
		printk("vm_start is 0x%lx\n",vma->vm_start);
		printk("vm_end is 0x%lx\n", vma->vm_end);
		num++;
		vma = vma->vm_next;
	}
		
	return ret;
}

/****************************ca_reset*************************************
* Summary of function:
*  charactor device release function
*
* Type : int release(struct inode *, struct file *)
*   slot_num: 0
    slot_type: CA_DESCR | CA_SC
    flags: CA_CI_MODULE_PRESENT | CA_CI_MODULE_READY
*
* Notes:
*
*****************************************************************/
static int ca_dev_release( struct inode  *psInode,struct file     *psFile )
{

    return(0);
}

/*****************************************************************
* Summary of function:
*  charactor device write function
*
* Type : ssize_t write(struct file *, const char *, size_t, loff_t *)
*
*
* Notes:
*
*****************************************************************/
static int ca_dev_write( struct file     *psFile,
                              const char      *pcBuff,
                              size_t          nCount,
                              loff_t          *psOff )
{
	
    return (nCount);
}

/*****************************************************************
* Summary of function:
*  charactor device read function
*
* Type : ssize_t read(struct file *, char *, size_t, loff_t *)
*
*
* Notes:COMMAND_STRUCT              command;
    UI32                        headerLen = sizeof(command);
    MMAC_SCARD_TRANSACTION_IOPB transBlock;
*
*****************************************************************/
static int ca_dev_read( struct file     *psFile,
                             char            *pcBuff,
                             size_t          nCount,
                             loff_t          *psOff )
{
    return(0);
}

/*****************************************************************
* Summary of function:
*  charactor device read function
*
* Type : int ioctl(struct inode *, struct file *, unsigned int, unsigned long)
*
*
* Notes:
*
*****************************************************************/
static int ca_dev_ioctl (struct inode *psInode,
							struct file *psFile,
							unsigned int cmd,
							unsigned long arg){
	int ret = 0;

	switch(cmd){
		

	}
	return ret;
}

/****************************************************************/
/* Charactor device for CA file_operations                   */
/****************************************************************/
static struct file_operations ca_dev_fops = {
    write:      ca_dev_write,
    read:       ca_dev_read,
    open:       ca_dev_open,
    release:    ca_dev_release,
    ioctl:		ca_dev_ioctl,
};
///////////////////////////////////////////////////////////////////

/****************************************************************/
/*          Module initialize                                   */
/****************************************************************/
int ca_major = CADEV_MAJOR;
static int ca_setup( void )
{
	int ret = 0;
	printk("%s\n", __func__);
	ret = register_chrdev( ca_major, CADEV_NAME, &ca_dev_fops );
	if (ret < 0)
	{
		PRINT_WX("Device registration error - %s(%d): (%d)\r\n", CADEV_NAME, ca_major, ret);
		return ret;
	}
	else
	{
		if(ca_major == 0)
			ca_major = ret;
	}
	return ret;
}

/****************************************************************/
static void ca_cleanup(void)
{
	printk("%s\n", __func__);
	unregister_chrdev( ca_major, CADEV_NAME );
    return ;
}

/****************************************************************/
module_init(ca_setup);
module_exit(ca_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wenx05124561@163.com");
MODULE_DESCRIPTION("ca module");


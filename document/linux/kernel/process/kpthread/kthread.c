/*
 joshua zhan 2010/09/04
*/

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *dx_task;

static int dx_thread(void * unused)
{
	int count = 0;
	
	//allow_signal(SIGKILL);
	printk("%s\n", __func__);


	
	while (!kthread_should_stop()) {  //in the while can not be a always loop should like follow  or in the loop you have the "break";
		// do something

		if(count % 2 == 1)//some condition you do not do someting,let the cup to others
		{
			schedule_timeout(2*HZ);
		}
		else  //some condition you should do someting
		{
			printk("count is %d\n", count);
							
		}
		count++;
		//printk("1count is %d\n", count);
	}

	printk(KERN_DEBUG "dx_thread11 exits\n");
	return 0;
}

static int __init filp_init(void)
{
	printk("%s\n", __func__);

	dx_task = kthread_run(dx_thread, NULL, "%s", "dx_thread");
	if (IS_ERR(dx_task) {
		printk(KERN_DEBUG"kthread_run failed");
		return -1;
	}
	return 0;
}

static void __exit filp_exit(void)
{
	printk("%s\n", __func__);
	
	if(!IS_ERR(dx_task))
	{
		printk("dx!=0 we need stop it\n");
		kthread_stop(dx_task);//must sure the kthread is not exit or it will be wrong
		dx_task = NULL;
	}	
}
module_init(filp_init);
module_exit(filp_exit);

MODULE_AUTHOR("wx");
MODULE_LICENSE("GPL");

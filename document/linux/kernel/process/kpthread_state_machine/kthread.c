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
static DECLARE_WAIT_QUEUE_HEAD(event_waitqueue);
typedef enum 
{
	FE_IDLE = 0,
	FE_RUNNING ,
	FE_QUIT
}STATE;
static int dx_thread(void * unused)
{
	STATE state = FE_IDLE;
	DECLARE_WAITQUEUE(wait, current);
	int once = 0;
	printk("we are in the %s\n", __func__);

	
	allow_signal(SIGKILL);
	add_wait_queue(&event_waitqueue, &wait);

	
	while (!kthread_should_stop()) {  
		if (signal_pending(current)) break;

		switch (state) {
		case FE_IDLE:
			printk(KERN_DEBUG"%s, go to sleep ...\n", __func__);
			set_current_state(TASK_INTERRUPTIBLE);
			schedule();
			if (signal_pending(current)) 
			break;
			printk(KERN_DEBUG"%s, waken up\n", __func__);
			
			break;
		case FE_RUNNING:
			printk("FE_RUNNING\n");
			printk("FE_RUNNING\n");
			printk("FE_RUNNING\n");
			if(!once)
			{	
				state = FE_RUNNING;
				once = 1;			
			}
			else
				state = FE_QUIT;
			break;
		case FE_QUIT:
			goto out;
			break;
		}
	}
out:
	printk(KERN_DEBUG "dx_thread exits\n");
	return 0;	
}

static int __init filp_init(void)
{
	printk("we are in the %s\n", __func__);


	dx_task = kthread_run(dx_thread, NULL, "%s", "dx_thread");
	if (IS_ERR(dx_task)) {
		printk(KERN_DEBUG"kthread_run failed");
		return -1;
	}
	msleep(1000*4);
	wake_up(&event_waitqueue);
	return 0;
}

static void __exit filp_exit(void)
{
	printk("we are in the %s\n", __func__);
	
	/*msleep(1000*4);
	wake_up(&event_waitqueue);
	
	msleep(1000*2);*/
	if (!IS_ERR(dx_task)) {
		printk("in the kthread_stop\n");
		kthread_stop(dx_task);	
	}	
	
}
module_init(filp_init);
module_exit(filp_exit);

MODULE_AUTHOR("wx");
MODULE_LICENSE("GPL");

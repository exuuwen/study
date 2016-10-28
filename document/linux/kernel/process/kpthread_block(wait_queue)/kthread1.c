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
#include <linux/wait.h>

static struct task_struct *dx_task;
static int dx_stop = 0;
wait_queue_head_t  wait_queue;
wait_queue_head_t  wait_queue2;
int flag = 0;
int flag2 = 0;
static int dx_thread(void * unused)
{
	int count = 0;
	
	//allow_signal(SIGKILL);
	printk("we are in the2 dx thread\n");
	printk("we are in the2 dx thread\n");
	printk("we are in the2 dx thread\n");

	
	while (!kthread_should_stop()) {  //in the while can not be a always loop should like follow  or in the loop you have the "break";
		// do something
		wait_event(wait_queue, (flag == 1));
		printk("hahaha\n");
		wait_event(wait_queue2, (flag2 == 1));
	}
	dx_stop = 1;
	printk(KERN_DEBUG "dx_thread11 exits\n");
	return 0;
}

static int __init filp_init(void)
{
	printk("we are in the2 dx_init\n");
	printk("we are in the2 dx_init\n");
	printk("we are in the2 dx_init\n");
	init_waitqueue_head(&wait_queue);
	init_waitqueue_head(&wait_queue2);
	dx_task = kthread_run(dx_thread, NULL, "%s", "dx_thread");
	if (!dx_task) {
		printk(KERN_DEBUG"kthread_run failed");
		return -1;
	}
	return 0;
}

static void __exit filp_exit(void)
{
	printk("we are in the2 dx_exit\n");
	printk("we are in the2 dx_exit\n");
	printk("we are in the2 dx_exit\n");
	
	
	if(!dx_stop)
	{
		printk("dx!=0 we need stop it\n");
		if(flag == 0)
		{
			flag = 1;
			wake_up(&wait_queue);
		}
		if(flag2 == 0)
		{
			flag2 = 1;
			wake_up(&wait_queue2);
		}
		kthread_stop(dx_task);//must sure the kthread is not exit or it will be wrong
		printk("after kthread_stop\n");
		dx_task = NULL;
	}	
	
}
module_init(filp_init);
module_exit(filp_exit);

MODULE_AUTHOR("wx");
MODULE_LICENSE("GPL");

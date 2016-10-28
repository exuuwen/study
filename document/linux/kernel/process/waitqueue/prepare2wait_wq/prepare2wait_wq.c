#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/sched.h>


struct delayed_work work;
wait_queue_head_t  wait_queue;
int flag = 0;

void work_handler(struct work_struct *data)
{
    printk(KERN_ALERT "work handler function.\n");

    /*wake_up: can wake interruptible(wait_event_interruptible) and uninterruptible(wait_event)*/
    /*wake_up_interruptible: only can wake up interrptible one*/
    wake_up_interruptible(&wait_queue);
}

static int __init waitqueue_init(void)
{
    int ret;

    printk("prepare2wait waitqueue init!\n");

    INIT_DELAYED_WORK(&work, work_handler);
    init_waitqueue_head(&wait_queue);

    schedule_delayed_work(&work, 2*HZ); 
    
    DEFINE_WAIT(wait);
    prepare_to_wait(&wait_queue, &wait, TASK_INTERRUPTIBLE);
    ret = schedule_timeout(8*HZ);
    finish_wait(&wait_queue, &wait);
    /*TASK_INTERRUPTIBLE can be wake up by signal*/
    if (signal_pending(current))
    {
        printk(KERN_ALERT "received a signal\n");
        return -ERESTARTSYS;
    }

    if (ret == 0)
        printk("timeout, no wakeup\n");
    else
        printk("wakeup success\n");   
    
		
    return 0;
}

static void __exit waitqueue_exit(void)
{
    printk("prepare2wait waitqueue exit!\n");
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(waitqueue_init);
module_exit(waitqueue_exit);



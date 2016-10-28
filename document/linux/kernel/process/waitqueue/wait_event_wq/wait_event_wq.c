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

    flag = 1;
    /*wake_up: can wake interruptible(wait_event_interruptible) and uninterruptible(wait_event)*/
    /*wake_up_interruptible: only can wake up interrptible one*/
    wake_up_interruptible(&wait_queue);
}


static int __init waitqueue_init(void)
{
    int ret;

    printk("waitqueue init!\n");
    INIT_DELAYED_WORK(&work, work_handler);
    init_waitqueue_head(&wait_queue);

    schedule_delayed_work(&work, 2*HZ); 
    ret = wait_event_interruptible_timeout(wait_queue, (flag == 1), 8*HZ);
    if (ret == 0) 
        printk("it is wait timeout not set flag, flag is %d\n", flag);
    else if (ret == 1)
        printk("it is wait timeout but set flag, flag is %d\n", flag);
    else if (ret == -ERESTARTSYS)
        printk("wake up by signal, flag is %d\n", flag);
    else
        printk("it is wait success flag is %d\n", flag);

    
		
    return 0;
}

static void __exit waitqueue_exit(void)
{
    printk("waitqueue exit!\n");
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(waitqueue_init);
module_exit(waitqueue_exit);



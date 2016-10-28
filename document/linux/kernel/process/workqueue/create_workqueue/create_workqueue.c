#include <linux/module.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/cpumask.h>
#include <linux/cpu.h>
#include <linux/sched.h>
#include <linux/delay.h>

static struct work_struct work;
static struct work_struct work_on[32];
static struct delayed_work delay_work;
static struct delayed_work delay_work_on[32];
static struct workqueue_struct *my_wq;

static void work_handler(struct work_struct *data)
{
    printk(KERN_ALERT "work handler function.\n");
}

static void work_on_handler(struct work_struct *data)
{
    printk(KERN_ALERT "work on handler function run on %d core\n", task_cpu(current));
}

static int __init my_wq_init(void)
{
    int i;

    printk("init my_workquuee\n");
    /* create_singlethread_workqueue: create one thread not on all cpus*/
    my_wq = create_workqueue("my_workqueue");

    INIT_WORK(&work, work_handler);
    INIT_DELAYED_WORK(&delay_work, work_handler);

    queue_work(my_wq, &work);
    queue_delayed_work(my_wq, &delay_work, 10*HZ);
    
    get_online_cpus();
    for_each_online_cpu(i)
    {
        INIT_WORK(work_on + i, work_on_handler);
        queue_work_on(i, my_wq, work_on + i);
        if (i == 0)
            /*cancel the work if it is pending or wait it to complete*/
            cancel_work_sync(work_on + i);
        else
            /*wait the execute complete*/
            flush_work_sync(work_on + i);
    }
    put_online_cpus();

    /*force the delay worker execute and cancel the timer and wait the execute complete*/
    flush_delayed_work_sync(&delay_work);
 
    get_online_cpus();
    for_each_online_cpu(i)
    {
        INIT_DELAYED_WORK(delay_work_on + i, work_on_handler);
        queue_delayed_work_on(i, my_wq, delay_work_on + i, 2*HZ);
    }
    put_online_cpus();
    /*modify the new timer and new cpu for pending delay work*/
    mod_delayed_work_on(1, my_wq, delay_work_on, 20*HZ); 
    /*cancel the delay work if it is pending or wait it to complete (cancel_delayed_work_sync: if work is running and it do not wait)*/
    cancel_delayed_work_sync(delay_work_on + 1);

    return 0;
}

static void __exit my_wq_exit(void)
{
    printk("exit my_workqueue\n");
    destroy_workqueue(my_wq);
}

MODULE_LICENSE("GPL");
module_init(my_wq_init);
module_exit(my_wq_exit);


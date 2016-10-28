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

static void work_handler(struct work_struct *data)
{
    printk(KERN_ALERT "work handler function.\n");
}

static void work_on_handler(struct work_struct *data)
{
    printk(KERN_ALERT "work on handler function run on %d core\n", task_cpu(current));
}

static int __init system_wq_init(void)
{
    int i;

    printk("init system_workquuee\n");

    INIT_WORK(&work, work_handler);
    INIT_DELAYED_WORK(&delay_work, work_handler);

    schedule_work(&work);
    schedule_delayed_work(&delay_work, 10*HZ);
    
    get_online_cpus();
    for_each_online_cpu(i)
    {
        INIT_WORK(work_on + i, work_on_handler);
        schedule_work_on(i, work_on + i);
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
        schedule_delayed_work_on(i, delay_work_on + i, (i+6)*HZ);
    }
    put_online_cpus();
    /*cancel the delay work if it is pending or wait it to complete (cancel_delayed_work_sync: if work is running and it do not wait)*/
    cancel_delayed_work_sync(delay_work_on + 1);

    return 0;
}

static void __exit system_wq_exit(void)
{
    printk("exit sysytem_workqueue\n");
}

MODULE_LICENSE("GPL");
module_init(system_wq_init);
module_exit(system_wq_exit);


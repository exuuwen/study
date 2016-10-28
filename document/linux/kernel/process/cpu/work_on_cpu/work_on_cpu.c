#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/cpumask.h>
#include <linux/cpu.h>
#include <linux/sched.h>

void process_context_func(void *info)
{
    printk("process task_cpu:%d\n", task_cpu(current));
    msleep(3000);
}

void interupt_context_func(void *info)
{
    printk("interupt ask_cpu:%d\n", task_cpu(current));
    mdelay(3000);
}

int my_work_on_cpu(int cpu, void (*func_on_cpu)(void*), void* data, bool sleep)
{
    int err = 0;

    get_online_cpus();
    if (!cpu_online(cpu))
        err = -EINVAL;
    else
    {
        if (sleep)
            err = work_on_cpu(cpu, (long (*)(void*))func_on_cpu, data);
        else
            err = smp_call_function_single(cpu, func_on_cpu, data, true);
    }
    put_online_cpus();

    return err;
}

static int __init cpu_hotplug_init(void)
{
    int i;
    printk("cpu_hotplug init\n");

    for_each_possible_cpu(i)
    {
        my_work_on_cpu(i, process_context_func, NULL, true);
    }

    for_each_possible_cpu(i)
    {
        my_work_on_cpu(i, interupt_context_func, NULL, false);
    }

    return 0;
}

static void __exit cpu_hotplug_exit(void)
{
    printk("cpu_hotplug exit\n");
}

module_init(cpu_hotplug_init)
module_exit(cpu_hotplug_exit)
MODULE_LICENSE("GPL");




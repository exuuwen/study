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

static int __cpuinit cpu_hotplug_callback(struct notifier_block *nfb,
                                            unsigned long action, void *hcpu)
{
    unsigned int cpu = (unsigned long)hcpu;
    switch (action) 
    {
    case CPU_ONLINE:
    case CPU_ONLINE_FROZEN:
        printk("cpu_hotplug notify %d cpu online\n", cpu);
        break;
    case CPU_DEAD:
    case CPU_DEAD_FROZEN:
        printk("cpu_hotplug notify  %d cpu offline\n", cpu);
        break;
    }

    return NOTIFY_OK;
}
static struct notifier_block __cpuinitdata cpu_hotplug_notifer =
{
    .notifier_call = cpu_hotplug_callback,
};

static int __init cpu_hotplug_init(void)
{
    int i, ret;
    printk("cpu_hotplug init\n");

    for_each_possible_cpu(i)
    {
        printk("cpu %d possible\n", i);
    }

    printk("\n");

    for_each_present_cpu(i)
    {
        printk("cpu %d present\n", i);
    }

    printk("\n");

    /*get_online_cpus() block online and offline operations for cpus, 
     if only avoid all the online cpu go away, 
    use preempt_disable() and preempt_enable()*/
    get_online_cpus();
    for_each_online_cpu(i)
    {
        printk("cpu %d online\n", i);
    }
    ret = register_hotcpu_notifier(&cpu_hotplug_notifer);
    put_online_cpus();

    if (ret == 0)
    {
        printk("register_cpu_notifier success\n");
    }

    return ret;
}

static void __exit cpu_hotplug_exit(void)
{
    get_online_cpus();
    unregister_hotcpu_notifier(&cpu_hotplug_notifer);
    put_online_cpus();

    printk("unregister_cpu_notifier success\n");
    printk("cpu_hotplug exit\n");
}

module_init(cpu_hotplug_init)
module_exit(cpu_hotplug_exit)
MODULE_LICENSE("GPL");




#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/percpu.h>

#include <linux/cpumask.h>
#include <linux/cpu.h>
#include <linux/sched.h>

static DEFINE_PER_CPU(int, var);

static void set_percpu(void *info)
{
    get_cpu_var(var) = task_cpu(current) + 100;
    put_cpu_var(var); 

    printk("set_percpu:run cpu %d, get percpu var : %d\n", task_cpu(current), get_cpu_var(var));
    put_cpu_var(var);
}

static int __init percpu_init(void)
{
    int i;

    printk("percpu init\n");

    get_online_cpus();
    for_each_online_cpu(i) 
    {
        smp_call_function_single(i, set_percpu, NULL, true);
    }
    put_online_cpus();

    printk("run cpu %d, get percpu var : %d\n", task_cpu(current), get_cpu_var(var));
    put_cpu_var(var);
    
    return 0;
}

static void __exit percpu_exit(void)
{
	printk("percpu exit\n");
	
}

module_init(percpu_init)
module_exit(percpu_exit)
MODULE_LICENSE("GPL");




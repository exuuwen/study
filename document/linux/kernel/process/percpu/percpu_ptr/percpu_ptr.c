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

static int * __percpu var1;
static int * __percpu var2;

static void set_percpu(void *info)
{
    int *p;
    p = get_cpu_ptr(var1);
    *p = task_cpu(current) + 100;
    put_cpu_ptr(var1); 
}

static int __init percpu_init(void)
{
    int i;
    int *p;

    printk("percpu init\n");
    var1 = alloc_percpu(int);
    get_online_cpus();
    for_each_online_cpu(i) 
    {
        smp_call_function_single(i, set_percpu, NULL, true);
    }
    put_online_cpus();

    p = get_cpu_ptr(var1);
    printk("run cpu %d, get percpu var1 : %d\n", task_cpu(current), *p);
    put_cpu_ptr(var1);


    var2 = alloc_percpu(int);
    get_online_cpus();
    for_each_online_cpu(i) 
    {
        get_cpu();
        p = per_cpu_ptr(var2, i);
        *p = i + 200;
        put_cpu();
    }
    put_online_cpus();
    
    get_cpu();
    p = per_cpu_ptr(var2, task_cpu(current));
    printk("run cpu %d, get percpu var2 : %d\n", task_cpu(current), *p);
    put_cpu();

    return 0;
}

static void __exit percpu_exit(void)
{
    printk("percpu exit\n");
    free_percpu(var1);
    free_percpu(var2);
}

module_init(percpu_init)
module_exit(percpu_exit)
MODULE_LICENSE("GPL");




#include <linux/sched.h>
#include <linux/module.h>
#include <linux/kthread.h>

MODULE_LICENSE("GPL");
static struct task_struct *thread_k = NULL;
static int fun_thread(void *nothing)
{
        long timeout;
        allow_signal(SIGINT);
        do{
                printk(KERN_INFO "thread is running...\n");
                timeout = HZ;
                while(timeout > 0)
                        timeout = schedule_timeout(timeout);
                if (signal_pending(current)){
                        printk(KERN_INFO "Got signal\n");
                        thread_k = NULL;
                        break;
                }
        }while(!kthread_should_stop());
        printk(KERN_ALERT "fun_thread exit\n");
	
        return 0;
}
static int __init init_thread(void)
{
        thread_k = kthread_run(fun_thread, NULL, "ltwthread");
        if(IS_ERR(thread_k)){
                printk(KERN_ALERT "faild to start lthread\n");
                return 0;
        }
        return 0;
}
static void __exit fini_thread(void)
{
	
        if ((thread_k != NULL) && (!IS_ERR(thread_k)))
	{
		printk("need stop kthread\n");
                kthread_stop(thread_k);
	}
}
module_init(init_thread);
module_exit(fini_thread);

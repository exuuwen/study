#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/mmu_context.h>

#define MAX_COUNT 10

static int exit_ = 0;
static unsigned long count = 0;
static int pids;
static unsigned long addr;

module_param(pids, int, S_IRUGO);
module_param(addr, ulong, S_IRUGO);

static int main_loop(void)
{
    struct task_struct *t = NULL;

    if (!pids || !addr)
        return 0;
	
    while (exit_) {
        /* 正确的做法是：pid应该由netlink或者procfs等U/K通信接口告知内核 */
        struct pid *pid = find_get_pid(pids);
        t = pid_task(pid, PIDTYPE_PID);
        if (t) {
            /* 正确的做法是：内存指针应该通过U/K通信接口告知内核 */
            char *pshare = (char *)addr;
            struct mm_struct *mm = t->mm;
            struct mm_struct *old_mm = current->mm;
            /* 
             * 注意！这是在具体的task上下文而不是任意上下文，否则use_mm就不行！
             * 因为用户内存是可被换出的，也是可能缺页的，缺页的处理可能会睡眠
             * 所以不要指望在网络协议栈接收/发送软中断中进行用户内存的操作，除非
             * 你能确认它们处在task上下文。
             **/
            use_mm(mm);
            /* 
             * 访问用户进程的内存
             * 这里仅仅是一个最简单的例子，事实上，在这里可以任意的像在用户进程自身一样
             * 访问这个地址空间，关键是你要知道符号名称以及位置，这需要设计一个机制，使
             * 符号的位置可以映射到一个名称...
             *
             * 此时，只要用户进程维护一个内存表，该内存表的物理内存当然可以在HIGHUSER区域
             * 被分配！实际上这只是一块虚拟地址空间，缺页中断会映射物理内存。
             *
             * 最好存些什么呢？我知道，在这里-内核空间可以访问sqlite数据库了...
             **/
            printk("pid %d kernel info:%s  %p\n", pids, pshare, pshare);
            unuse_mm(mm);
            if (old_mm)
	    	use_mm(old_mm);
    	
	    count ++;
            if (count == MAX_COUNT)
	    	exit_ = 0;
        }  
	schedule_timeout(10000);
    }  
    return 0;
}

static int __init tt_init(void)
{
    struct task_struct *tsk;
    exit_ = 1;

    printk("pid %d, addr %lu\n", pids, addr);
    /* 不要直接用kernel_thread创建内核线程，而应该委托专门的线程来做这件事 */
    /* pid = kernel_thread(test_it, NULL, SIGCHLD); */
    tsk = kthread_run((void *)main_loop, NULL, "KERNEL-ACCESS-USER");
    if (IS_ERR(tsk)) {
        return -1;
    }

    return 0;
}

static void tt_cleanup(void)
{
    return;
}

module_init(tt_init);
module_exit(tt_cleanup);
MODULE_LICENSE("GPL");

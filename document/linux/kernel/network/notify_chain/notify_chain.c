#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");

static struct timer_list test_timer;
#define TIME_OUT (jiffies + 3 * HZ)
static int times = 0;
/*
 * 定义自己的通知链头结点以及注册和卸载通知链的外包函数
 */
 
/*
 * RAW_NOTIFIER_HEAD是定义一个通知链的头部结点，
 * 通过这个头部结点可以找到这个链中的其它所有的notifier_block
 */
 
static RAW_NOTIFIER_HEAD(test_chain);

/*
 * 自定义的注册函数，将notifier_block节点加到刚刚定义的test_chain这个链表中来
 * raw_notifier_chain_register会调用notifier_chain_register
 */
 
static int register_test_notifier(struct notifier_block *nb)
{
        return raw_notifier_chain_register(&test_chain, nb);
}


static int unregister_test_notifier(struct notifier_block *nb)
{
        return raw_notifier_chain_unregister(&test_chain, nb);
}

/*
 * 自定义的通知链表的函数，即通知test_chain指向的链表中的所有节点执行相应的函数
 */
 
static int test_notifier_call_chain(unsigned long val, void *v)
{
        return raw_notifier_call_chain(&test_chain, val, v);
}



static int test_event1(struct notifier_block *this, unsigned long event, void *ptr)
{
        printk("In Event 1: Event Number is %ld\n", event);
        return 0; 
}

static int test_event2(struct notifier_block *this, unsigned long event, void *ptr)
{
        printk("In Event 2: Event Number is %ld\n", event);
        return 0; 
}

static int test_event3(struct notifier_block *this, unsigned long event, void *ptr)
{
        printk("In Event 3: Event Number is %ld\n", event);
        return 0; 
}

/*
 * 事件1，该节点执行的函数为test_event1
 */
 
static struct notifier_block test_notifier1 =
{
        .notifier_call = test_event1,
};

/*
 * 事件2，该节点执行的函数为test_event1
 */
 
static struct notifier_block test_notifier2 =
{
        .notifier_call = test_event2,
};

/*
 * 事件3，该节点执行的函数为test_event1
 */
 
static struct notifier_block test_notifier3 =
{
        .notifier_call = test_event3,
};
/*
 * 对这些事件进行注册
 */
 
static int reg_notifier(void)
{
        int err;
        printk("Begin to register:\n");
        
        err = register_test_notifier(&test_notifier1);
        if (err)
        {
                printk("register test_notifier1 error\n");
                return -1; 
        }
        printk("register test_notifier1 completed\n");

        err = register_test_notifier(&test_notifier2);
        if (err)
        {
                printk("register test_notifier2 error\n");
                return -1; 
        }
        printk("register test_notifier2 completed\n");

        err = register_test_notifier(&test_notifier3);
        if (err)
        {
                printk("register test_notifier3 error\n");
                return -1; 
        }
        printk("register test_notifier3 completed\n");
        return err;
}

/*
 * 卸载刚刚注册了的通知链
 */
 
static void  unreg_notifier(void)
{
        printk("Begin to unregister\n");
        unregister_test_notifier(&test_notifier1);
        unregister_test_notifier(&test_notifier2);
        unregister_test_notifier(&test_notifier3);
        printk("Unregister finished\n");
}


static void test_timer_func(unsigned long __data)
{
	int err = 0;
	printk("==============================\n");
        err = test_notifier_call_chain(times, NULL);
        printk("==============================\n");
        if (err)
        {
		printk("notifier_call_chain error\n");
	        return;
	}	
	times++;
	mod_timer(&test_timer, TIME_OUT);
}

/*
 * init and exit 
 */
 
static int __init init_notifier(void)
{
	int err;
      	
	printk("init_notifier\n");
	err = reg_notifier();
	
	setup_timer(&test_timer, test_timer_func, 0);
	test_timer.expires = TIME_OUT;
	add_timer(&test_timer);

        return err;
        
}

static void __exit exit_notifier(void)
{
	printk("exit_notifier\n");
        unreg_notifier();
	del_timer_sync(&test_timer);
}

module_init(init_notifier);
module_exit(exit_notifier);

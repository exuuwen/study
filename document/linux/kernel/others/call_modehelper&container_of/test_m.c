#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/string.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/netfilter.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/list.h>
#include <linux/kmod.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#define err(msg) printk(KERN_INFO "%s failed.\n", msg)
#define TIME_OUT (jiffies + 3 * HZ)

struct user_data{
	int id;
	char file_name[15];
};
struct workqueue_data {
	struct work_struct worker;
	struct user_data worker_data;
};
static int count = 0;
static struct workqueue_data work;
static struct timer_list test_timer; // just for debug
static char path[25] = "/home/cr7/echo.sh";
static char file_name[15] = "/home/cr7/wx";
static char *envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

void work_handler(struct work_struct *work)
{
	int err = 0;;
	struct workqueue_data *data = NULL;
	char id[5];
	

        printk("work handler function.\n");
	data = container_of(work, struct workqueue_data, worker);

	sprintf(id, " %d", data->worker_data.id);
	char *argv[] = { path, data->worker_data.file_name, id, NULL };
	printk("file_data id:%d:%s\n", data->worker_data.id, data->worker_data.file_name);

	err = call_usermodehelper(path, argv, envp, 1);
    	if (err < 0) {    
		err("call_usermodehelper");
    	}
	count++;
}


static void test_timer_func(unsigned long __data)
{
	//int err = 0;
	
	//struct work_struct *work = NULL;
	//work = kmalloc(sizeof(struct work_struct), GFP_KERNEL);
	printk("in the test_timer_func\n");
	INIT_WORK(&(work.worker), work_handler);
	work.worker_data.id = count;
	memcpy(&(work.worker_data.file_name), file_name, sizeof(file_name));
	schedule_work(&(work.worker)); 

	mod_timer(&test_timer, TIME_OUT);
}


static int __init main_init(void)
{	
	printk("in the test_m \n");
	setup_timer(&test_timer, test_timer_func, 0);
	test_timer.expires = TIME_OUT;
	add_timer(&test_timer);
    	return 0;
}

static void __exit main_exit(void)
{
	printk("out the test_m\n");
	del_timer_sync(&test_timer);
}

module_init(main_init);
module_exit(main_exit);
MODULE_LICENSE("GPL");


/*
 * drivers/input/keyboard/s3c-keypad.c
 * KeyPad Interface on S3C 
 *
 * $Id: s3c-keypad.c,v 1.1 2008/06/09 07:43:16 dark0351 Exp $
 * 
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h> 
#include <linux/errno.h>
#include <linux/semaphore.h>


#include "dvb_event.h"

#define MAX_EVENT_NR	64	



int event_code[] = {
                1,2,3,4,5,6,7,8,
                9,10,11,12,13,14,15,16,
                17,18,19,20,21,22,23,24,
                25,26,27,28,29,30,31,32,
                33,34,35,36,37,38,39,40,
                41,42,43,44,45,46,47,48,
                49,50,51,52,53,54,55,56,
                57,58,59,60,61,62,63,64
       };



#define  WX_DEBUG
#ifdef WX_DEBUG
#define PRINT_WX(fmt,arg...) printk(fmt, ##arg)
#else
#define PRINT_WX(fmt,arg...)		/* !!!! */
#endif

#define DEVICE_NAME "dvb_event"


static struct semaphore mutex;
//static struct timer_list test_timer;
static struct input_dev *input_dev;

//#define TIME_OUT  (jiffies + 5*HZ)

static void send_event(unsigned char data)
{
	down(&mutex);
	input_report_key(input_dev, event_code[data], 1);
	input_sync(input_dev);	
	input_report_key(input_dev, event_code[data], 0);
	input_sync(input_dev);
	up(&mutex);
}


static int ioctl(struct inode *inodep, struct file *filp, unsigned
  int cmd, unsigned long arg)
{
  	unsigned char data;

  	switch (cmd)
  	{
		case SEND_EVENT:
		copy_from_user(&data ,(unsigned char*)arg ,sizeof(data));

		if(data < 0 || data >= 64)
		return -EINVAL;
		PRINT_WX("the dat is %d........\n", data);
		send_event(data);           
      		break;

    		default:
      		return  - EINVAL;
  	}

  	return 0;
}
static ssize_t read(struct file *file, char __user *user, size_t size, loff_t *o)
{ 
	printk("it is read haha........\n");
	return 0;
}

static ssize_t write(struct file *file, const char __user *in, size_t size, loff_t *off)
{
	printk("it is write haha........\n");
	return 0;
} 

static struct file_operations mymisc_fops = { 
	.owner  = THIS_MODULE, 
	.write  = write, 
	.read  = read,
	//.open = open,
	.ioctl = ioctl
}; 

static struct miscdevice mymisc_dev = { 
	.minor = MISC_DYNAMIC_MINOR, 
	.name = "mymisc", 
	.fops = &mymisc_fops,
};

int  dvb_event_init(void)
{
	int ret = 0;
	int key, code;
	
	input_dev = input_allocate_device();

	if (!input_dev) {
		input_free_device(input_dev);
		return -ENOMEM;
	}

	/* create and register the input driver */
	set_bit(EV_KEY, input_dev->evbit);
	//set_bit(EV_REP, input_dev->evbit);

	for(key = 0; key < MAX_EVENT_NR ; key++){
		code = event_code[key];
		if(code<=0)
			continue;
		set_bit(code & KEY_MAX, input_dev->keybit);
	}

	input_dev->name = DEVICE_NAME;
	input_dev->phys = "dvb_event/input0";
	
	
	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0100;

	input_dev->keycode = event_code;
	//input_dev->ketcodesize = sizeof(unsigned char);
	//input_dev->keycodemax = 64;
	
	ret = input_register_device(input_dev);
	if (ret) {
		printk("Unable to register dvb_event input device!!!\n");
		goto out;
	}

	ret=misc_register(&mymisc_dev);
	PRINT_WX("misc_register  ok ,ret is %d..\n",ret);
	sema_init(&mutex, 1);
	/* Scan timer init */
	/*init_timer(&test_timer);
	test_timer.function = test_timer_handler;
	//test_timer.data = (unsigned long)s3c_keypad;
	test_timer.expires = TIME_OUT;
	add_timer(&test_timer);*/

	PRINT_WX("%s Initialized...........\n", DEVICE_NAME);
	PRINT_WX("%s Initialized...........\n", DEVICE_NAME);
	PRINT_WX("%s Initialized...........\n", DEVICE_NAME);
	return 0;

out:
	input_unregister_device(input_dev);
	return ret;
}

void  dvb_event_exit(void)
{
	misc_deregister(&mymisc_dev);
	input_unregister_device(input_dev);
	//del_timer(&test_timer);	
	PRINT_WX("%s Removed......\n", DEVICE_NAME);
	PRINT_WX("%s Removed......\n", DEVICE_NAME);
	PRINT_WX("%s Removed......\n", DEVICE_NAME);
	return ;
}




module_init(dvb_event_init);
module_exit(dvb_event_exit);

MODULE_AUTHOR("wenxu");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("dvb event");

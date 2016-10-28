#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/timer.h>

#define INPUTFIFO_SIZE	8	
#define FIFO_CLEAR 0x1  
#define INPUT_MAJOR 233    
#define INPUT_NAME  "input_dev"
#define TIME_OUT (jiffies + 5* HZ)

static int input_major = INPUT_MAJOR;
static struct timer_list test_timer;



static struct inputfifo_dev                                    
{                                                                              
  	unsigned int current_len;    
  	unsigned char mem[INPUTFIFO_SIZE];   
  	unsigned int head;
  	unsigned int end;
  	struct semaphore sem; 
 	// struct semaphore open_sem;  
	//wait_queue_head_t w_wait;         
  	wait_queue_head_t r_wait;     	    
};

static  struct inputfifo_dev *inputfifo_devp; 


void inputfifo_write(unsigned char message)
{
	down(&inputfifo_devp->sem);
    	printk("before in write the dev->current_len is %d\n",inputfifo_devp->current_len);
	printk("before in write the dev->head is %d\n",inputfifo_devp->head);
	printk("before in write the dev->end is %d\n",inputfifo_devp->end);
	
	memcpy(inputfifo_devp->mem+inputfifo_devp->end, &message, 1);
	//inputfifo_dev->mem[end]=message;
	inputfifo_devp->end = (inputfifo_devp->end+1)%INPUTFIFO_SIZE;
	if(inputfifo_devp->current_len == INPUTFIFO_SIZE)
	{
    		printk("the FIFO is full........\n");
    		printk("the FIFO is full........\n");
    		printk("the FIFO is full........\n");
		inputfifo_devp->head = (inputfifo_devp->head+1)%INPUTFIFO_SIZE;
	}
	else
		(inputfifo_devp->current_len)++;
	printk("after in write the dev->current_len is %d\n",inputfifo_devp->current_len);
	printk("after in write the dev->head is %d\n",inputfifo_devp->head);
	printk("after in write the dev->end is %d\n",inputfifo_devp->end);

	wake_up_interruptible(&inputfifo_devp->r_wait);
        up(&inputfifo_devp->sem);
}





unsigned char message=0;
static void test_timer_func(unsigned long __data)
{
	printk("we are in the timer........\n");
	inputfifo_write(message);
	if(message == 255)
		message=0;
	else
		message++;
	
	mod_timer(&test_timer, TIME_OUT);
}



int inputfifo_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inputfifo_devp;
	setup_timer(&test_timer, test_timer_func, 0);
	test_timer.expires = TIME_OUT;
	add_timer(&test_timer);	
	//down(&inputfifo_devp->open_sem);
	return 0;
}



static int inputfifo_ioctl(struct inode *inodep, struct file *filp, unsigned
  int cmd, unsigned long arg)
{
  	struct inputfifo_dev *dev = filp->private_data;

  	switch (cmd)
  	{
  		case FIFO_CLEAR:
      		down(&dev->sem);    	
      		dev->current_len = 0;
      		memset(dev->mem,0,INPUTFIFO_SIZE);
      		up(&dev->sem); 
         
      		printk(KERN_INFO "inputfifo is set to zero\n");      
      		break;

    		default:
      		return  - EINVAL;
  	}

  	return 0;
}

static unsigned int inputfifo_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;
	struct inputfifo_dev *dev = filp->private_data; 
  	
	printk("we are in the inputfifo_poll...\n");
 	down(&dev->sem);
  
 	poll_wait(filp, &dev->r_wait, wait);
  	//poll_wait(filp, &dev->w_wait, wait);  
  
  	if (dev->current_len != 0)
  	{
		printk("now the dev->current_len is not 0\n");
    		mask |= POLLIN | POLLRDNORM; 
  	}
  
  	/*if (dev->current_len != INPUTFIFO_SIZE)
  	{
    		mask |= POLLOUT | POLLWRNORM; 
  	}*/
     
  	up(&dev->sem);
  	return mask;
}




int inputfifo_release(struct inode *inode, struct file *filp)
{
	//inputfifo_fasync( -1, filp, 0);
  	//up(&inputfifo_devp->open_sem);
	del_timer(&test_timer);
  	return 0;
}


static ssize_t inputfifo_read(struct file *filp, char __user *buf, size_t count,
  loff_t *ppos)
{
	int ret;
	unsigned int count1,count2;
	struct inputfifo_dev *dev = filp->private_data; 
	DECLARE_WAITQUEUE(wait, current); 
	down(&dev->sem); 
	printk("in the inputfifo_read...........\n");
	printk("before in read the dev->current_len is %d\n",inputfifo_devp->current_len);
	printk("before in read the dev->head is %d\n",inputfifo_devp->head);
	printk("before in read the dev->end is %d\n",inputfifo_devp->end);

	
	add_wait_queue(&dev->r_wait, &wait);
  
  
	if (dev->current_len == 0)
	{
		if (filp->f_flags &O_NONBLOCK)
		{
			ret =  - EAGAIN;
			goto out;
		} 
		__set_current_state(TASK_INTERRUPTIBLE); /*wait_*/
		up(&dev->sem);
		printk("it is will be sleep ,until there is some data in it\n");
		schedule(); 
		if (signal_pending(current))
		{
			ret =  - ERESTARTSYS;
			goto out2;
		}

    		down(&dev->sem);
  	}


	if (count > dev->current_len)
		count = dev->current_len;

		printk("count is %d\n", count);

  	if(count > INPUTFIFO_SIZE-dev->head)
  	{
		count1 = INPUTFIFO_SIZE - dev->head;
		count2 = count - count1;
	
		printk("count1 is %d\n", count1);
		printk("count2 is %d\n", count2);
  		if (copy_to_user(buf, dev->mem+dev->head, count1))
 		{
    			ret =  - EFAULT;
    			goto out;
  		}
		if (copy_to_user(buf+count1, dev->mem, count2))
 		{
    			ret =  - EFAULT;
    			goto out;
  		}
  		else
  		{
    			//memcpy(dev->mem, dev->mem + count, dev->current_len - count); 
    			dev->current_len -= count; 
   	 		printk("read %d bytes(s),current_len:%d\n", count, dev->current_len);
     			dev->head = (dev->head+count)%INPUTFIFO_SIZE;
    			//wake_up_interruptible(&dev->w_wait); 
    
    			ret = count;
  		}
  	}
  	else 
  	{
		if (copy_to_user(buf, dev->mem+dev->head, count))
 		{
    			ret =  - EFAULT;
    			goto out;
  		}
  		else
  		{
    			//memcpy(dev->mem, dev->mem + count, dev->current_len - count); 
    			dev->current_len -= count; 
   		 	printk("read %d bytes(s),current_len:%d\n", count, dev->current_len);
     			dev->head = (dev->head+count)%INPUTFIFO_SIZE;
    			//wake_up_interruptible(&dev->w_wait); 
    	
    			ret = count;
  		}	
  	}
  	out: up(&dev->sem); 
	printk("after in read the dev->current_len is %d\n",inputfifo_devp->current_len);
	printk("after in read the dev->head is %d\n",inputfifo_devp->head);
	printk("after in read the dev->end is %d\n",inputfifo_devp->end);
  	out2:remove_wait_queue(&dev->r_wait, &wait);
  	set_current_state(TASK_RUNNING);
  	return ret;
}




static const struct file_operations input_dev_fops =
{
	.owner = THIS_MODULE,
	.read = inputfifo_read,
	//.write = inputfifo_write,
	.ioctl = inputfifo_ioctl,
	.poll = inputfifo_poll,
  	.open = inputfifo_open,
  	.release = inputfifo_release,
  	//.fasync = inputfifo_fasync,
};






static int input_init( void )
{
	int ret = 0;

	printk("we are in the input init...........\n");
	
	ret = register_chrdev( input_major, INPUT_NAME, &input_dev_fops );
	if (ret < 0)
	{
		printk("Device registration error - %s(%d): (%d)\r\n", INPUT_NAME, input_major, ret);
		return ret;
	}
	else
	{
		printk("rgister input driver ok  ...%s(%d): %d\r\n", INPUT_NAME, input_major, ret);
		//if(input_major == 0)
		//	input_major = ret;
		 inputfifo_devp = kmalloc(sizeof(struct inputfifo_dev), GFP_KERNEL);
 		 if (!inputfifo_devp)    
 	 	 {
    			ret =  - ENOMEM;
    			goto fail_malloc;
  		 }

  		memset(inputfifo_devp, 0, sizeof(struct inputfifo_dev));



  		init_MUTEX(&inputfifo_devp->sem); 
 		 //init_MUTEX(&inputfifo_devp->open_sem);
  		init_waitqueue_head(&inputfifo_devp->r_wait); 
  		//init_waitqueue_head(&inputfifo_devp->w_wait); 
	}
	
	
	printk("we are in the input driver  end  haha...........\n");
	return 0;
	fail_malloc:
		unregister_chrdev( input_major, INPUT_NAME );
	return ret;
}

/****************************************************************/
static void input_exit(void)
{
	
	unregister_chrdev( input_major, INPUT_NAME );
	kfree(inputfifo_devp);
	printk("%s(%d) exit ok\n", INPUT_NAME, input_major );
    return ;
}

MODULE_AUTHOR("wenxu");
MODULE_LICENSE("Dual BSD/GPL");


module_init(input_init);
module_exit(input_exit);

//#include <linux/moduleloader.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/kmod.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/slab.h>

#include <asm/system.h>
#include <asm/page.h>
#include <asm/pgtable.h>

static inline u8 *emit_code(u8 *ptr, u32 bytes, unsigned int len)
{
	if (len == 1)
		*ptr = bytes;
	else if (len == 2)
		*(u16 *)ptr = bytes;
	else {
		*(u32 *)ptr = bytes;
		barrier();
	}
	return ptr + len;
}

#define EMIT(bytes, len)	do { prog = emit_code(prog, bytes, len); } while (0)

#define EMIT1(b1)		EMIT(b1, 1)
#define EMIT2(b1, b2)		EMIT((b1) + ((b2) << 8), 2)
#define EMIT3(b1, b2, b3)	EMIT((b1) + ((b2) << 8) + ((b3) << 16), 3)
#define EMIT4(b1, b2, b3, b4)   EMIT((b1) + ((b2) << 8) + ((b3) << 16) + ((b4) << 24), 4)
#define EMIT1_off32(b1, off)	do { EMIT1(b1); EMIT(off, 4);} while (0)

#define TIME_OUT (jiffies + 3 * HZ)
static struct timer_list test_timer; // just for debug
struct _handle
{
 unsigned int  (*bpf_func)();
};
static struct _handle handle;


static void test_timer_func(unsigned long __data)
{
	int ret = 0;
	printk("in the test_timer_func\n");
	//ret = handle.bpf_func();
	printk("handle.bpf_func() ret:%d\n",ret);

	mod_timer(&test_timer, TIME_OUT);
}


static int get_image()
{
	u8* image = NULL;
	u8 temp[64];
	u8* prog;
	unsigned char proglen = 6;
	prog = temp;
	EMIT1(0xb8);
	EMIT4(0x15,00,00,00);
	EMIT1(0xc3);
	//movl $20, %eax
	//ret 
	
	//image = module_alloc(max_t(unsigned int,proglen,sizeof(struct work_struct)));
	image = kmalloc(proglen,GFP_KERNEL);		  
	if (!image)
		return -1;
	memcpy(image, temp, proglen);
	handle.bpf_func = (void *)image;
	return   0;
}

static void jit_free_defer(struct work_struct *arg)
{
	//module_free(NULL, arg);
}

static int __init main_init(void)
{
	int ret = 0;	
	printk("<0>""in the test_m ...........\n");
	setup_timer(&test_timer, test_timer_func, 0);
	test_timer.expires = TIME_OUT;
	//ret = get_image();
	printk("<0>""get_image() ret:%d\n", ret);
	if(!ret)
		add_timer(&test_timer);
    return 0;
}

static void __exit main_exit(void)
{
	//struct work_struct *work = (struct work_struct *)handle.bpf_func;
	printk("<0>""out the test_m.........\n");
	del_timer_sync(&test_timer);	
	
	//INIT_WORK(work, jit_free_defer);
		
	//schedule_work(work);
	
}

module_init(main_init);
module_exit(main_exit);
MODULE_LICENSE("GPL");



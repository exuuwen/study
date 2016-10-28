#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>

#define MY_FILE "/home/dragon/test"

char buf[128] = "hshhsshshyeyy hello world!";
char buf_read[18];
struct file *file = NULL;



static int __init init(void)
{
        mm_segment_t old_fs;
        printk("Hello, I'm the module that intends to write messages to file.\n");


        if(file == NULL)
                file = filp_open(MY_FILE, O_RDWR, 0644);
        if (IS_ERR(file)) {
                printk("error occured while opening file %s, exiting...\n", MY_FILE);
                return 0;
        }

        //sprintf(buf,"%s", "The Messages.");

        /*old_fs = get_fs();
        set_fs(KERNEL_DS);
        file->f_op->write(file, (char *)buf, sizeof(buf), &file->f_pos);
        set_fs(old_fs);*/

	old_fs = get_fs();
        set_fs(KERNEL_DS);
        file->f_op->read(file, (char *)buf_read, sizeof(buf), &file->f_pos);//you can set file->f_pos to read again
        set_fs(old_fs);
	
	printk("read the file %s\n", buf_read);


        return 0;
}

static void __exit fini(void)
{
        if(!IS_ERR(file))
                filp_close(file, NULL);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");

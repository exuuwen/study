#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/gfp.h>


typedef struct manutd
{
	unsigned int use;
	char * name;
	spinlock_t  lock;
}manutd_t;

static struct kmem_cache  *manutd_cache;
static manutd_t *manutd;


void manutd_constructor(void *buf) //in kmalloc_manutd be used many times
{
	manutd_t *manutd = buf;
	//printk("%s\n", __func__);
	manutd->use = 0;
	manutd->name = NULL;
	spin_lock_init(&manutd->lock);

	return ;
}

manutd_t * kmalloc_manutd(unsigned char buf_size)
{
	manutd_t *manutd;
	printk("1%s\n", __func__);
	manutd = kmem_cache_alloc(manutd_cache, GFP_KERNEL);
	printk("2%s\n", __func__);
	/*manutd->use = 0;
    spin_lock_init(&manutd->lock);*/
	manutd->name = kmalloc(buf_size, GFP_KERNEL);
	return manutd;
}

void kfree_manutd(manutd_t * manutd)
{
	printk("%s\n", __func__);
	kfree(manutd->name);
	kmem_cache_free(manutd_cache, manutd);
}



static int __init init(void)
{
	
	char name[7] ="manutd";
		
	printk("%s\n", __func__);
	manutd_cache = kmem_cache_create("manutd_cache", sizeof(manutd_t), 0, 0, manutd_constructor);//, manutd_destructor, NULL, NULL, 0);
	
	manutd = kmalloc_manutd(10);
	manutd->use = 1;
	memcpy(manutd->name, name, sizeof(name));
	manutd->name[sizeof(name)] = 0;

	printk("manutd->use is %d\n", manutd->use);
	printk("manutd->name is %s\n", manutd->name);
	//manutd->use = 0;
    return 0;
}

static void __exit fini(void)
{
	printk("%s\n", __func__);
	printk("manutd->use is %d\n", manutd->use);
	printk("manutd->name is %s\n", manutd->name);
	manutd->use = 1;
	
	kfree_manutd(manutd);
	
	kmem_cache_destroy(manutd_cache);
	return ;
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");

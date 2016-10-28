#include <linux/version.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
/*must be open CONFIG_DEVKMEM*/
/*must be check CONFIG_STRICT_DEVMEM both in drivers/char/mem.c
arch/x86/mm/pat.c  can not use range_is_allowed return 0*/
/* CONFIG_STRICT_DEVMEM use function range_is_allowed to allow ram<1M and no ram I/O  return 1*/
#define PROC_MEMSHARE_DIR                                "memshare"
#define PROC_MEMSHARE_VIRTADDR                        "virtual_addr"
#define PROC_MEMSHARE_SIZE                                "virtual_size"

/*alloc one page. 4096 bytes*/
#define PAGE_ORDER                                0
/*this value can get from PAGE_ORDER*/
#define PAGES_NUMBER                                1

struct proc_dir_entry *proc_memshare_dir ;
struct proc_dir_entry *proc_entry_addr ;
struct proc_dir_entry *proc_entry_size ;
unsigned long kernel_memaddr = 0;
unsigned long kernel_memsize= 0;

char* test_string = "hello world!";

static ssize_t proc_read_virtmem_addr(char *page, char **start, off_t off, int count, int*eof, void *data)
{
        return sprintf(page, "%08lx\n", kernel_memaddr);
}
static ssize_t proc_read_virtmem_size(char *page, char **start, off_t off, int count, int*eof, void *data)
{
        return sprintf(page, "%lu\n", kernel_memsize);
}

static int __init init(void)
{
        /*build proc dir "memshare"and two proc files: phymem_addr, phymem_size in the dir*/
        proc_memshare_dir = proc_mkdir(PROC_MEMSHARE_DIR, NULL);
        proc_entry_addr = create_proc_entry(PROC_MEMSHARE_VIRTADDR, 0666, proc_memshare_dir);
        proc_entry_size = create_proc_entry(PROC_MEMSHARE_SIZE, 0666, proc_memshare_dir);
	
	proc_entry_addr->read_proc = proc_read_virtmem_addr;
	proc_entry_size->read_proc = proc_read_virtmem_size;

        /*alloc one page*/
        kernel_memaddr =__get_free_page(GFP_KERNEL);
        if(!kernel_memaddr)
        {
                printk("Allocate memory failure!\n");
        }
        else
        {
                SetPageReserved(virt_to_page(kernel_memaddr));
                kernel_memsize = PAGES_NUMBER * PAGE_SIZE;
                printk("Allocate memory success!. The virt mem addr=0x%08lx, size=0x%lx\n", kernel_memaddr, kernel_memsize);
		strcpy((char*)kernel_memaddr, test_string);
		printk("The content defeaut is: %s\n", (unsigned char *)kernel_memaddr);
        }
        return 0;
}

static void __exit fini(void)
{
        printk("The content written by user is: %s\n", (unsigned char *)kernel_memaddr);
        ClearPageReserved(virt_to_page(kernel_memaddr));
        free_pages(kernel_memaddr, PAGE_ORDER);
        remove_proc_entry(PROC_MEMSHARE_VIRTADDR, proc_memshare_dir);
        remove_proc_entry(PROC_MEMSHARE_SIZE, proc_memshare_dir);
        remove_proc_entry(PROC_MEMSHARE_DIR, NULL);

        return;
}
module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Godbach (nylzhaowei@163.com)");
MODULE_DESCRIPTION("Kernel memory share module.");

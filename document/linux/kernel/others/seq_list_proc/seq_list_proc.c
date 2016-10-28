#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#define SIZE 20
static struct mutex lock;
static struct list_head head;
struct my_data {
        struct list_head list;
        char info[SIZE];
};

static void add_one(char* info)
{
        struct my_data *data;

        mutex_lock(&lock);
        data = kzalloc(sizeof(*data), GFP_KERNEL);
        if (data != NULL)
        {
                strncpy(data->info, info, strlen(info)); 
                list_add(&data->list, &head);
        }
        mutex_unlock(&lock);
}

static ssize_t list_seq_write(struct file *file, const char __user * buffer,
                       size_t count, loff_t *ppos)
{
        char info[SIZE];
     
        memset(info, 0, SIZE);
        if (count > SIZE)
                return -EFBIG;
        copy_from_user(info, buffer, count - 1);
        
        add_one(info);

        return count;
}

static int list_seq_show(struct seq_file *m, void *p)
{
        struct my_data *data = list_entry(p, struct my_data, list);

        seq_printf(m, "info: %s\n", data->info);
        return 0;
}

static void *list_seq_start(struct seq_file *m, loff_t *pos)
{
        mutex_lock(&lock);
        return seq_list_start(&head, *pos);
}

static void *list_seq_next(struct seq_file *m, void *p, loff_t *pos)
{
        return seq_list_next(p, &head, pos);
}

static void list_seq_stop(struct seq_file *m, void *p)
{
        mutex_unlock(&lock);
}

static struct seq_operations _seq_ops = {
        .start = list_seq_start,
        .next = list_seq_next,
        .stop = list_seq_stop,
        .show = list_seq_show
};

static int list_seq_open(struct inode *inode, struct file *file)
{
        return seq_open(file, &_seq_ops);
}

static struct file_operations list_seq_fops = {
        .open = list_seq_open,
        .read = seq_read,
        .write = list_seq_write,
        .llseek = seq_lseek,
        .release = seq_release
};

static void clean_all(struct list_head *head)
{
        struct my_data *data;

        while (!list_empty(head)) {
                data = list_entry(head->next, struct my_data, list);
                list_del(&data->list);
                kfree(data);
        }
}

static int __init seq_init(void)
{
        struct proc_dir_entry *entry;

        mutex_init(&lock);
        INIT_LIST_HEAD(&head);

        add_one("aaaa");
        add_one("bbbb");

        entry = create_proc_entry("my_data", S_IWUGO | S_IRUGO, NULL);
        if (entry == NULL) {
                clean_all(&head);
                return -ENOMEM;
        }
        entry->proc_fops = &list_seq_fops;

        return 0;
}

static void __exit seq_exit(void)
{
        remove_proc_entry("my_data", NULL);
        clean_all(&head);
}

MODULE_AUTHOR("xu.wen@ericsson.com");
MODULE_LICENSE("Dual BSD/GPL");


module_init(seq_init);
module_exit(seq_exit);

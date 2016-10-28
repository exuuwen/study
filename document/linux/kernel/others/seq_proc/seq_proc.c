/**
 *  procfs4.c -  create a "file" in /proc
 * 	This program uses the seq_file library to manage the /proc file.
 *
 */

#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/proc_fs.h>	/* Necessary because we use proc fs */
#include <linux/seq_file.h>	/* for seq_file */

#define PROC_NAME	"exuuwen"

MODULE_AUTHOR("exuuwen");
MODULE_LICENSE("GPL");

/**
 * This function is called at the beginning of a sequence.
 * ie, when:
 *	- the /proc file is read (first time)
 *	- after the function stop (end of sequence)
 *
 */
#define N 10
static unsigned int counter[N];

static void *my_seq_start(struct seq_file *s, loff_t *pos)
{

    /* beginning a new sequence ? */	
    if ( *pos == 0 )
    {	
	/* yes => return a non null value to begin the sequence */
	return counter;
    }
    else if (*pos < N)
    {
	/* no => it's the end of the sequence, return end to stop reading */
	return counter + *pos;
    }
    else
    {
        return NULL;
    }
}

/**
 * This function is called after the beginning of a sequence.
 * It's called untill the return is NULL (this ends the sequence).
 *
 */
static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    unsigned int *tmp_v = (unsigned int *)v;
    
    tmp_v++;
    if (tmp_v == counter + N)
    {
        tmp_v = NULL;
    }

    (*pos)++;

    return tmp_v;
}

/**
 * This function is called at the end of a sequence
 * 
 */
static void my_seq_stop(struct seq_file *s, void *v)
{
    /* nothing to do, we use a static value in start() */
}

/**
 * This function is called for each "step" of a sequence
 *
 */
static int my_seq_show(struct seq_file *s, void *v)
{
    int *spos = (int *) v;

    seq_printf(s, "%d\n", *spos);
    return 0;
}

/**
 * This structure gather "function" to manage the sequence
 *
 */
static struct seq_operations my_seq_ops = {
     .start = my_seq_start,
     .next  = my_seq_next,
     .stop  = my_seq_stop,
     .show  = my_seq_show
};

/**
 * This function is called when the /proc file is open.
 *
 */
static int my_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &my_seq_ops);
};

/**
 * This structure gather "function" that manage the /proc file
 *
 */
static struct file_operations my_file_ops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = seq_release
};
	
	
/**
 * This function is called when the module is loaded
 *
 */
static int __init seq_init(void)
{
    struct proc_dir_entry *entry;
    int i;

    for(i=0; i<N; i++)
    {
       counter[i] = i; 
    }

    entry = create_proc_entry(PROC_NAME, 0444, NULL);
    if (entry) 
    {
        entry->proc_fops = &my_file_ops;
    }

    printk("init seq module ok \n");
	
    return 0;
}

/**
 * This function is called when the module is unloaded.
 *
 */
static void __exit seq_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
}

module_init(seq_init);
module_exit(seq_exit);

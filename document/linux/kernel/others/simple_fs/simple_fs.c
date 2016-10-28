#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mount.h>
#include <linux/pagemap.h> 	/* PAGE_CACHE_SIZE */
#include <linux/fs.h>     	/* This is where libfs stuff is declared */
#include <asm/atomic.h>
#include <asm/uaccess.h>	/* copy_to_user */
#include <linux/slab.h>
#include <linux/namei.h>
#include <linux/backing-dev.h>

#define SIMPLE_SUPER_MAGIC	0x20140108

int simplefs_drop_inode(struct inode *inode)
{
    if (inode->i_private)
        kfree(inode->i_private);

    printk("in the drop inode\n");
    return 1;
}

static struct super_operations simplefs_s_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= simplefs_drop_inode,
};

static struct backing_dev_info simplefs_backing_dev_info = {
	.name		= "simplefs",
	.capabilities	= BDI_CAP_NO_ACCT_AND_WRITEBACK,
};

static struct inode *simplefs_make_inode(struct super_block *sb, umode_t mode)
{
    struct inode *inode = new_inode(sb);

    if (inode) 
    {
        inode->i_ino = get_next_ino();
        inode->i_mode = mode;
        //inode->i_uid = 0;//current_fsuid();
        //inode->i_gid = 0;//current_fsgid();
        inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
	inode->i_mapping->backing_dev_info = &simplefs_backing_dev_info;
    }

    return inode;
}

static int simplefs_open(struct inode *inode, struct file *filp)
{
    filp->private_data = inode->i_private;
    //printk("inode %p\n", inode);  
    return 0;
}
 
#define TMPSIZE 20

static ssize_t simplefs_read_file(struct file *filp, char *buf,
		size_t count, loff_t *offset)
{
    atomic_t *counter = (atomic_t *) filp->private_data;
    int v, len;
    char tmp[TMPSIZE];
/*
 * Encode the value, and figure out how much of it we can pass back.
 */
    v = atomic_read(counter);
    if (*offset > 0)
        v -= 1;  /* the value returned when offset was zero */
    else
        atomic_inc(counter);
    len = snprintf(tmp, TMPSIZE, "%d\n", v);
    if (*offset > len)
        return 0;
    if (count > len - *offset)
        count = len - *offset;
/*
 * Copy it back, increment the offset, and we're done.
 */
    if (copy_to_user(buf, tmp + *offset, count))
        return -EFAULT;
    *offset += count;
     return count;
}
 
/*
 * Write a file.
 */
static ssize_t simplefs_write_file(struct file *filp, const char *buf,
		size_t count, loff_t *offset)
{
    atomic_t *counter = (atomic_t *) filp->private_data;
    char tmp[TMPSIZE];

    if (*offset != 0)
        return -EINVAL;
/*
 * Read the value from the user.
 */
    if (count >= TMPSIZE)
        return -EINVAL;
    memset(tmp, 0, TMPSIZE);
    if (copy_from_user(tmp, buf, count))
    return -EFAULT;
/*
 * Store it in the counter and we are done.
 */
    atomic_set(counter, simple_strtol(tmp, NULL, 10));
    return count;
}

static struct file_operations simplefs_file_ops = {
    .open   = simplefs_open,
    .read   = simplefs_read_file,
    .write  = simplefs_write_file,
};

static int common_create_file_or_dir(struct super_block *sb, struct dentry *dentry, umode_t mode, void *data);

static int simplefs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
    int stuff;
    printk("mkdir dir %p, destry %p\n", dir, dentry);
    return common_create_file_or_dir(dir->i_sb, dentry, mode | S_IFDIR, (void*)(&stuff));
}

static int simplefs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool b)
{
    atomic_t *counter = kmalloc(sizeof(*counter), GFP_KERNEL);
    atomic_set(counter, 0);
    printk("create dir %p, destry %p, name %s\n", dir, dentry, dentry->d_iname);
    return common_create_file_or_dir(dir->i_sb, dentry, mode | S_IFREG, (void*)counter);
}

static int simplefs_rmdir(struct inode *dir, struct dentry *dentry)
{
    printk("rmdir dir %p, destry %p\n", dir, dentry);
    if (dir->i_sb->s_root->d_inode == dir && strcmp(dentry->d_iname, "root-subdir") == 0)
        return -EPERM;
    return simple_rmdir(dir, dentry);
}

static int simplefs_unlink(struct inode *dir, struct dentry *dentry)
{
    printk("unlink dir %p, destry %p\n", dir, dentry);
    if (dir->i_sb->s_root->d_inode == dir && strcmp(dentry->d_iname, "root-file") == 0)
        return -EPERM;
    return simple_unlink(dir, dentry);
}

static int simplefs_link(struct dentry *old_dentry, struct inode *dir, struct dentry *dentry)
{
    printk("link dir %p, old dentry %p, destry %p\n", dir, old_dentry, dentry);
    if (dir->i_sb->s_root->d_inode == dir && strcmp(old_dentry->d_iname, "root-file") == 0)
        return -EPERM;
    return simple_link(old_dentry, dir, dentry);
}

static int simplefs_rename(struct inode *old_dir, struct dentry *old_dentry,
		struct inode *new_dir, struct dentry *new_dentry)
{
    printk("rename old dir %p, new dir, %p, old dentry %p, new destry %p\n", old_dir, new_dir, old_dentry, new_dentry);
    if (old_dir->i_sb->s_root->d_inode == old_dir && (strcmp(old_dentry->d_iname, "root-file") == 0 || strcmp(old_dentry->d_iname, "root-subdir") == 0))
        return -EPERM;
    return simple_rename(old_dir, old_dentry, new_dir, new_dentry);
}

static struct dentry *simplefs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
{
    //printk("in the simple lookup\n");
    return simple_lookup(dir, dentry, flags);
}

static int simplefs_symlink(struct inode * dir, struct dentry *dentry, const char * symname)
{
    printk("symlink dir %p, dentry %p, dname %s, symname %s\n", dir, dentry, dentry->d_iname, symname);
    if (dir->i_sb->s_root->d_inode == dir && (strcmp(symname, "root-file") == 0 || strncmp(symname, "root-subdir", strlen("root-subdir")) == 0))
        return -EPERM;

    return common_create_file_or_dir(dir->i_sb, dentry, S_IFLNK | S_IRUGO | S_IWUSR, (void*)symname);
}


static const struct inode_operations simplefs_dir_inode_operations = {
	.lookup = simplefs_lookup,
        .create = simplefs_create,
	.mkdir = simplefs_mkdir,
	.rmdir = simplefs_rmdir,
        .unlink = simplefs_unlink,
	.link = simplefs_link,
	.symlink = simplefs_symlink,
	.rename = simplefs_rename,
	/*
	.setxattr = simplefs_setxattr,
	.getxattr = simplefs_getxattr,
	.listxattr = simplefs_listxattr,
	.removexattr = simplefs_removexattr,
	*/
};

static int simplefs_set_page_dirty_no_writeback(struct page *page)
{
	if (!PageDirty(page))
		return !TestSetPageDirty(page);
	return 0;
}

const struct address_space_operations simplefs_aops = {
	.readpage	= simple_readpage,
	.write_begin	= simple_write_begin,
	.write_end	= simple_write_end,
	.set_page_dirty = simplefs_set_page_dirty_no_writeback,
};

static int common_create_file_or_dir(struct super_block *sb, struct dentry *dentry, umode_t mode, void *data)
{
    struct inode *inode;
    if (!dentry)
        return -ENOENT;
    if (dentry->d_inode)
        return -EEXIST;
    
    inode = simplefs_make_inode(sb, mode);
    if (!inode)
        return -ENOMEM;

    if (S_ISDIR(mode)) 
    {
        if (data)
            inode->i_op = &simplefs_dir_inode_operations;
        else
            inode->i_op = &simple_dir_inode_operations;
        inode->i_fop = &simple_dir_operations;
        inc_nlink(inode);
        inc_nlink(dentry->d_parent->d_inode);
    }
    else if (S_ISLNK(mode))
    {
        char *symname = (char*)data;
        int l = strlen(symname) + 1;
        int error;
        inode->i_op = &page_symlink_inode_operations;
        inode->i_mapping->a_ops = &simplefs_aops;
        error = page_symlink(inode, symname, l);
        if (error) 
        {
            iput(inode);
            return error;
        }
    }
    else if (S_ISREG(mode))
    {
        if (data)
        {
            inode->i_private = data;
            inode->i_fop = &simplefs_file_ops;
        }
    }
    else
    {
        iput(inode);
        printk("unsupported File type\n");
        return -EPERM;
    }

/*
 * Put it all into the dentry cache and we're done.
 */
    d_instantiate(dentry, inode);
    dget(dentry);

    return 0;

}
/*
 * Create a file mapping a name to a counter.
 */
static struct dentry *simplefs_kernel_create_file (struct super_block *sb,
		struct dentry *dir, const char *name)
{
    struct dentry *dentry;
    int ret;
    
    mutex_lock(&dir->d_inode->i_mutex);

    dentry = lookup_one_len(name, dir, strlen(name));
    if (!dentry)
        goto out;

    ret = common_create_file_or_dir(sb, dentry, S_IFREG | S_IRUGO | S_IWUSR, NULL);
    if (ret)
        goto out_dput;

    dput(dentry);
    mutex_unlock(&dir->d_inode->i_mutex);
    return dentry;
/*
 * Then again, maybe it didn't work.
 */
out_dput:
    dput(dentry);
out:
    mutex_unlock(&dir->d_inode->i_mutex);
    return NULL;
}

 
/*
 * Create a directory which can be used to hold files.  This code is
 * almost identical to the "create file" logic, except that we create
 * the inode with a different mode, and use the libfs "simple" operations.
 */
static struct dentry *simplefs_kernel_create_dir (struct super_block *sb,
		struct dentry *dir, const char *name)
{
    struct dentry *dentry;
    int ret;

    mutex_lock(&dir->d_inode->i_mutex);

    dentry = lookup_one_len(name, dir, strlen(name));
    if (!dentry)
        goto out;

    ret = common_create_file_or_dir(sb, dentry, S_IFDIR | S_IRUGO | S_IXUGO | S_IWUSR, NULL);
    if (ret)
        goto out_dput;
   
    dput(dentry);
    mutex_unlock(&dir->d_inode->i_mutex);
    return dentry;
 
out_dput:
    dput(dentry);
out:
    mutex_unlock(&dir->d_inode->i_mutex);
    return NULL;
}

static int simplefs_get_rootdir(struct super_block *sb)
{
    /*static const struct dentry_operations cgroup_dops = {
    .d_iput = cgroup_diput,
    .d_delete = cgroup_delete,
    };*/
    struct dentry *root_dentry;
    struct inode *inode = simplefs_make_inode(sb, S_IFDIR | S_IRUGO | S_IXUGO | S_IWUSR);

    if (!inode)
        goto out;

    inode->i_fop = &simple_dir_operations;
    inode->i_op = &simplefs_dir_inode_operations;
    /* directories start off with i_nlink == 2 (for "." entry) */
    inc_nlink(inode);
    root_dentry = d_make_root(inode);
    if (!root_dentry)
        goto out_iput;
    sb->s_root = root_dentry;
    /* for everything else we want ->d_op set */
    //sb->s_d_op = &cgroup_dops;
    return 0;

out_iput:
    iput(inode);
out:
    return -ENOMEM;
}

struct simplefs
{
    atomic_t ref;
    char name[256];   
};

static int simplefs_set_super(struct super_block *sb, void *data)
{
    int ret;

    ret = set_anon_super(sb, NULL);
    if (ret)
        return ret;

    sb->s_blocksize = PAGE_CACHE_SIZE;
    sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
    sb->s_magic = SIMPLE_SUPER_MAGIC;
    sb->s_op = &simplefs_s_ops;

    return 0;
}

static int simplefs_test_super(struct super_block *sb, void *data)
{
    struct simplefs *fs = (struct simplefs*)(sb->s_fs_info);

    if (strcmp(fs->name, (char*)data) == 0)
    {
        printk("test match\n");
        return 1;
    }

    return 0;
}

static struct dentry *simplefs_mount(struct file_system_type *fs_type,
			 int flags, const char *dev_name, void *data)
{
    struct super_block *sb;
    struct simplefs *fs;
    struct dentry *subdir;
    int ret;

    sb = sget(fs_type, simplefs_test_super, simplefs_set_super, 0, (void*)dev_name); 

    if (IS_ERR(sb))
    {
        ret = PTR_ERR(sb);
        return ERR_PTR(ret);
    }

    fs = sb->s_fs_info;
    if (!fs)
    {
        ret = simplefs_get_rootdir(sb);
        if (ret)
            goto drop_new_super;
        /*
        * One counter in the top-level directory.
        */
        fs = (struct simplefs*)kmalloc(sizeof(*fs), GFP_KERNEL);
        if (!fs)
        {
            ret = -ENOMEM;
            goto drop_new_super;
        }

        memset(fs->name, 0, sizeof(fs->name));
        //atomic_set(&fs->ref, 1);
        strcpy(fs->name, (char*)dev_name);
    
        //atomic_set(&fs->counter, 0);
        simplefs_kernel_create_file(sb, sb->s_root, "root-file");
        /*
        * And one in a subdirectory.
        */
        //atomic_set(&fs->subcounter, 0);
        subdir = simplefs_kernel_create_dir(sb, sb->s_root, "root-subdir");
        //if (subdir)
            //simplefs_kernel_create_file(sb, subdir, "subcounter", &fs->subcounter);

        sb->s_fs_info = fs;
    }
    else
    {
        atomic_inc(&fs->ref);
    }
    
    return dget(sb->s_root);

drop_new_super:
    deactivate_locked_super(sb);
    return ERR_PTR(ret);
    
}

static void simplefs_kill_sb(struct super_block *sb) 
{
    struct simplefs *fs = sb->s_fs_info;
    if (atomic_dec_and_test(&fs->ref))
        kfree(fs);
    kill_litter_super(sb);
}

static struct file_system_type simplefs_type = {
    .owner 	= THIS_MODULE,
    .name	= "simplefs",
    .mount	= simplefs_mount,
    .kill_sb	= simplefs_kill_sb,
};


/*
 * Get things set up.
 */
static int __init simplefs_init(void)
{
    printk("simplefs register\n");
    return register_filesystem(&simplefs_type);
}

static void __exit simplefs_exit(void)
{
    printk("simplefs unregister\n");
    unregister_filesystem(&simplefs_type);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("exuuwen");

module_init(simplefs_init);
module_exit(simplefs_exit);

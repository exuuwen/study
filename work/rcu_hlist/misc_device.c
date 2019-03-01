#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h> 
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>

#include "uake.h"

#define UAKE_HASHBITS 8
#define UAKE_HASHENTRIES (1 << UAKE_HASHBITS)

struct uake_hlist_entry {
	struct uake_entry entry;
	struct hlist_node addr4_hlist;
	struct hlist_node addr6_hlist;
};

struct uake_hlist {
	struct hlist_head  *uake6_head;
	struct hlist_head  *uake4_head;
	struct uake_hlist_entry *entries;
	struct rcu_head         rcu;
};

static struct uake_hlist __rcu *g_uake;
static unsigned int uake_h_initval;
static spinlock_t lock;

static inline struct hlist_head *uake_hash(struct hlist_head *uake_head, struct uake_entry *entry, bool is_v4)
{
	unsigned hash = 0;
	
	if (is_v4)
		hash = jhash_1word(entry->ip_addr, uake_h_initval);
	else
		hash = jhash_3words(entry->ip6_addr.s6_addr32[0], entry->ip6_addr.s6_addr32[1], 
						  entry->ip6_addr.s6_addr32[3], uake_h_initval);

	return &uake_head[hash & ((1 << UAKE_HASHBITS) - 1)];
}

static void uake_hlist_init(struct hlist_head *uake_head) 
{
	int i;

	for (i = 0; i < UAKE_HASHENTRIES; i++)
		INIT_HLIST_HEAD(&uake_head[i]);
}

static int uake_hlist_lookup(struct uake_entry *entry) 
{
	struct hlist_node *p;
	struct uake_hlist *uake;
	int ret = -ENOENT;

	rcu_read_lock();
	uake = rcu_dereference(g_uake);
	if (entry->ip_addr) {
		if (uake == NULL || uake->uake4_head == NULL)  
			goto out;
		hlist_for_each(p, uake_hash(uake->uake4_head, entry, true)) {
			struct uake_hlist_entry *uake_entry
					= hlist_entry(p, struct uake_hlist_entry, addr4_hlist);
			if (uake_entry->entry.ip_addr == entry->ip_addr) {
				memcpy(&entry->ip6_addr, &uake_entry->entry.ip6_addr, sizeof(struct in6_addr));
      				ret = 0;
			}
		}
	}
	else {
		if (uake == NULL || uake->uake6_head == NULL)  
			goto out;
		hlist_for_each(p, uake_hash(uake->uake6_head, entry, false)) {
			struct uake_hlist_entry *uake_entry
					= hlist_entry(p, struct uake_hlist_entry, addr6_hlist);
			if (!memcmp(&entry->ip6_addr, &uake_entry->entry.ip6_addr, sizeof(struct in6_addr))) {
				entry->ip_addr = uake_entry->entry.ip_addr;
      				ret = 0;
			}
		}
	}

out:
	rcu_read_unlock();
	return ret;
}

static void uake_reclaim(struct rcu_head *rp)
{
	struct uake_hlist *uh = container_of(rp, struct uake_hlist, rcu);

	if (uh) {
		printk("uake in the reclaim\n");
		if (uh->entries)
			vfree(uh->entries);
		if (uh->uake4_head)
			kfree(uh->uake4_head);
		if (uh->uake6_head)
			kfree(uh->uake6_head);
		kfree(uh);
	}
}

static int uake_hlist_update(struct uake_map *data)
{
	int i;
	unsigned short num = data->count;
	struct uake_hlist *new_uake = NULL;
	struct uake_hlist *old_uake = NULL;

	new_uake = kmalloc(sizeof(struct uake_hlist), GFP_KERNEL);
	if (!new_uake) {
		printk("uake hlist kmalloc fail\n");
		return -ENOMEM;
	}

	new_uake->uake6_head = kmalloc(sizeof(struct hlist_head) * UAKE_HASHENTRIES, GFP_KERNEL);
	if (!new_uake->uake6_head) {
		printk("uake hlist uake6_head kmalloc fail\n");
		kfree(new_uake);
		return -ENOMEM;
	}
	uake_hlist_init(new_uake->uake6_head);

	new_uake->uake4_head = kmalloc(sizeof(struct hlist_head) * UAKE_HASHENTRIES, GFP_KERNEL);
	if (!new_uake->uake4_head) {
		printk("uake hlist uake4_head kmalloc fail\n");
		kfree(new_uake->uake6_head);
		kfree(new_uake);
		return -ENOMEM;
	}
	uake_hlist_init(new_uake->uake4_head);

	new_uake->entries = vmalloc(sizeof(struct uake_hlist_entry) * num);
	if (!new_uake->entries) {
		printk("uake hlist entries vmalloc fail\n");
		kfree(new_uake->uake6_head);
		kfree(new_uake->uake4_head);
		kfree(new_uake);
		return -ENOMEM;
	}

	for (i = 0; i < num; ++i) {
		memcpy(&new_uake->entries[i].entry, &(data->maps[i]), sizeof(struct uake_entry));
		hlist_add_head(&new_uake->entries[i].addr4_hlist, uake_hash(new_uake->uake4_head, &new_uake->entries[i].entry, true));
		hlist_add_head(&new_uake->entries[i].addr6_hlist, uake_hash(new_uake->uake6_head, &new_uake->entries[i].entry, false));
	}

	spin_lock(&lock);
	old_uake = rcu_dereference_protected(g_uake, lockdep_is_held(&lock));
	rcu_assign_pointer(g_uake, new_uake);
	spin_unlock(&lock);
	if (old_uake)
		call_rcu(&old_uake->rcu, uake_reclaim);

	return 0;
}

static long ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret;
	struct uake_entry entry;
	struct uake_map *data;

	switch (cmd)
	{   
		case UAKE_MAP_UPDATE:
			data = vmalloc(sizeof(struct uake_map));
			if (data) {
				ret = copy_from_user(data , (struct uake_map*)arg , sizeof(struct uake_map));
				if (!ret) {
					if (data->count > 0 && data->count <= MAX_UAKE_SIZE) {
						ret = uake_hlist_update(data);
					}
					else {
						printk("uake data count not correct %d\n", data->count);
						ret = -EINVAL;
					}
				} 
				else {
					printk("uake UAKE_MAP_UPDATE copy_from_user fail %d\n", ret);
					ret = -EINVAL;
				}
				vfree(data);
			}
			else {
				printk("uake UAKE_MAP_UPDATE valloc data fail");
				ret = -EINVAL;
			}
			break;
		case UAKE_MAP_GET:
			ret = copy_from_user(&entry , (struct uake_entry*)arg , sizeof(entry));
			if (!ret) {
				ret = uake_hlist_lookup(&entry);
				if (!ret) {
					ret = copy_to_user((void __user*)arg, &entry, sizeof(entry));
					if (ret) {
						printk("uake UAKE_MAP_GET copy_to_user fail %d\n", ret);
						ret = -ENOMEM;
					}
				}
			}
			else {
				printk("uake UAKE_MAP_GET copy_from_user fail %d\n", ret);
				ret = -ENOMEM;
			}
			break;

		default:
			ret = -EINVAL;
	}   

	return ret;
}

static struct file_operations uake_fops = { 
	.owner  = THIS_MODULE, 
	.unlocked_ioctl = ioctl,
}; 

static struct miscdevice uake_dev = { 
	.minor = MISC_DYNAMIC_MINOR, 
	.name = "uake", 
	.fops = &uake_fops,
};
 
int __init uake_device_init(void)
{ 
	int ret;
	ret = misc_register(&uake_dev);
	if (ret) {
		printk("uake misc_register fail ,ret is %d..\n", ret);
		goto out;
	}

	get_random_bytes(&uake_h_initval, sizeof(uake_h_initval));
	spin_lock_init(&lock);
	
	printk("uake misc_register ok\n");

out:
	return ret;
} 

void __exit uake_device_remove(void)
{ 
	struct uake_hlist *old_uake = NULL;

	spin_lock(&lock);
	old_uake = rcu_dereference_protected(g_uake, lockdep_is_held(&lock));
	spin_unlock(&lock);
	if (old_uake)
		call_rcu(&old_uake->rcu, uake_reclaim);

	misc_deregister(&uake_dev);

	printk("uake misc_deregister ok\n");
}
 
module_init(uake_device_init);
module_exit(uake_device_remove); 
MODULE_LICENSE("GPL v2");

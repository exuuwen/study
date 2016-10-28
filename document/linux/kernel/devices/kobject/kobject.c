#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/stat.h>
 
MODULE_AUTHOR("David Xie");
MODULE_LICENSE("Dual BSD/GPL");

#define KOBJ_NAME  "test_kobj" 
#define KOBJ_NAME_CHILD  "test_kobj_child" 
void obj_test_release(struct kobject *kobject);
ssize_t kobj_test_show(struct kobject *kobject, struct attribute *attr,char *buf);
ssize_t kobj_test_store(struct kobject *kobject,struct attribute *attr,const char *buf, size_t count);
 
struct attribute test_attr = {
        .name = "kobj_config",
        .mode = S_IRWXUGO,
};

struct attribute new_attr = {
        .name = "kobj_new",
        .mode = S_IRWXUGO,
};

static struct attribute *def_attrs_child[] = {
        &new_attr,
        NULL,
};
 
static struct attribute *def_attrs[] = {
        &test_attr,
        NULL,
};
 
 
struct sysfs_ops obj_test_sysops =
{
        .show = kobj_test_show,
        .store = kobj_test_store,
};
 
struct kobj_type ktype_child = 
{
        .release = obj_test_release,
        .sysfs_ops=&obj_test_sysops,
        .default_attrs=def_attrs_child,
};

struct kobj_type ktype = 
{
        .release = obj_test_release,
        .sysfs_ops=&obj_test_sysops,
        .default_attrs=def_attrs,
};
 
void obj_test_release(struct kobject *kobject)
{
        printk("eric_test: release .\n");
}
 
ssize_t kobj_test_show(struct kobject *kobject, struct attribute *attr,char *buf)
{
        printk("have show.\n");
        printk("attrname:%s.\n", attr->name);
        sprintf(buf,"%s\n",attr->name);
        return strlen(attr->name)+2;
}
 
ssize_t kobj_test_store(struct kobject *kobject,struct attribute *attr,const char *buf, size_t count)
{
        printk("havestore\n");
        printk("write: %s\n",buf);
        return count;
}
 
struct kobject kobj;
struct kobject kobj1;
static int kobj_test_init(void)
{
        printk("kboject test init.\n");
		//kobject_register(&kobj);
        kobject_init_and_add(&kobj,&ktype,NULL,KOBJ_NAME);
		kobject_init_and_add(&kobj1,&ktype,&kobj,KOBJ_NAME_CHILD);
		sysfs_create_file(&kobj, &new_attr);
		/*
		kobject_set_name(&kobj,KOBJ_NAME);
		kobject_register(&kobj);
		sysfs_create_file(&kobj, &test_attr);
		*/
        return 0;
}
 
static void kobj_test_exit(void)
{
        printk("kobject test exit.\n");
		sysfs_remove_file(&kobj, &new_attr);
		kobject_del(&kobj1);
        kobject_del(&kobj);
		
		
}
 
module_init(kobj_test_init);
module_exit(kobj_test_exit);

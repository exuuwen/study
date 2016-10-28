#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/types.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/if.h>

static char *ifname = "eth0";

int __init ip_list_init(void)
{
    struct net_device *dev;
    struct in_device *in_dev;

    dev = __dev_get_by_name(&init_net, ifname);
    if (dev == NULL)
    {
        printk(KERN_ERR "dev_get_by_name failed.\n");
        goto out;
    }

    in_dev = __in_dev_get_rcu(dev);
    if (!in_dev)
        goto out;
    
    for_ifa(in_dev)
    {
        __be32 addr = ifa->ifa_local;
        unsigned char *p = (unsigned char *)&addr;
    
        printk(KERN_INFO "%d.%d.%d.%d\n", *p, *(p+1), *(p+2), *(p+3));
    } endfor_ifa(in_dev);

    
    return 0;
out:
    return -ENODEV;
}

void __exit ip_list_exit(void)
{
}

module_param(ifname, charp, S_IRUGO);
module_init(ip_list_init);
module_exit(ip_list_exit);
MODULE_LICENSE("GPL");

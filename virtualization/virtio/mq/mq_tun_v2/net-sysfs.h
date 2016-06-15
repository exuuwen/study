#ifndef __NET_SYSFS_H__
#define __NET_SYSFS_H__

int netdev_kobject_init(void);
int netdev_register_kobject(struct net_device *);
void netdev_unregister_kobject(struct net_device *);
void netdev_initialize_kobject(struct net_device *);
int netdev_queue_update_kobjects(struct net_device *net,
				 int old_num, int new_num);
int rx_queue_add_kobject(struct net_device *net, int index);
#endif

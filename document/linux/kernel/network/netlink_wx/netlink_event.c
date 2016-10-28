#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <net/net_namespace.h>
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <linux/connector.h>
#include <linux/version.h>

#define MAX_CA_PAYLOAD 64 
#define NETLINK_TEST 20	

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhan");
MODULE_DESCRIPTION("ARIB test module");

/*enum {
	MSG_TYPE_CA_INSERTED = 1,
	MSG_TYPE_CA_REMOVED,
};*/
typedef enum
{
	MSG_TYPE_EVENT_ONE = 1,
	MSG_TYPE_EVENT_TWO = 2,
	MSG_TYPE_EVENT_THREE = 3,
	MSG_TYPE_EVENT_FOUR = 4

}EVENT;

typedef enum
{
	MSG = 1,
	DATA = 2
}MSG_TYPE;

typedef struct data_to_kernel
{
	EVENT event;
	char  info[20];
}data_to_kernel_t;

typedef struct data_from_kernel
{
	EVENT event;
	char  info[20];
}data_from_kernel_t;

static struct sock *nl_sk = NULL;
static void test_send(int pid, const void* data, int size);
static struct timer_list test_timer;


static unsigned char event[] ="123456";

#define TIME_OUT (jiffies + 3 * HZ)


static int port = 1234;

static void test_timer_func(unsigned long __data)
{
	if (port) {
		printk(" %s\n", __func__);
		printk("port is %d\n", port);
		test_send(port, event, sizeof(event));
	}

	mod_timer(&test_timer, TIME_OUT);
}


static void test_send(int pid, const void *data, int size)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	char *datab;
	int err;
	int len = 0;
	int skblen = 0;
	if (!pid) {
		return;
	}

	len = NLMSG_SPACE(size);
	skblen = NLMSG_SPACE(len);

	skb = alloc_skb(skblen, GFP_KERNEL);
	if (!skb) {
		printk("alloc_skb failed\n");
		return;
	}

	nlh = nlmsg_put(skb, pid, 0, 0, size, 0);
	if (!nlh) {
		printk("nlmsg_put failed\n");
		kfree_skb(skb);
		return;
	}

	datab = NLMSG_DATA(nlh);
	memcpy(datab, data, size);

	err = nlmsg_unicast(nl_sk, skb, pid);
	printk("nlsmsg_unicast(%d)\n", err);
}
data_from_kernel_t events = { MSG_TYPE_EVENT_TWO, "data_from_kernel"};

#if 1
static void data_ready (struct sk_buff *skb)  /*one way ,it has another way as below*/
{
        struct nlmsghdr *nlh = NULL;

        printk("in the data ready\n");

        if(skb == NULL)
        {
                printk("skb is NULL \n");
                return ;
        }

        nlh = (struct nlmsghdr *)skb->data;

        switch (nlh->nlmsg_type) {
        case MSG:
				printk("it is MSG \n");
                port = nlh->nlmsg_pid;
                //add_timer(&test_timer);
                break;
        case DATA:
		printk("it is DATA \n");
		unsigned short size = (nlh->nlmsg_len - sizeof(*nlh));
		data_to_kernel_t *datab;
		datab = NLMSG_DATA(nlh);
		printk("received payload len: %d data type is %d ,data is %s\n", size,datab->event, datab->info);
		test_send(port, &events, sizeof(events));
		break;
        default:
                break;
        }

}
#else


static void data_ready (struct sk_buff *_skb)//two way
{
        struct nlmsghdr *nlh = NULL;
	struct sk_buff *skb;
        printk("in the data ready\n");

	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)		
	skb = skb_get(_skb);
	#else
	skb = skb_dequeue(&_skb->sk_receive_queue)
	#endif

        if(skb == NULL)
	{
        	printk("skb is NULL \n");
        	return ;
	}
	printk("in the nlhbefore\n");
	nlh = nlmsg_hdr(skb);
	//nlh = (struct nlmsghdr *)skb->data;
	printk("in the nlhafter\n");
        switch (nlh->nlmsg_type) {
        case MSG:
		printk("it is MSG \n");
                port = nlh->nlmsg_pid;
               // add_timer(&test_timer);
                break;
        case DATA:
		printk("it is DATA \n");
		unsigned short size = (nlh->nlmsg_len - sizeof(*nlh));
		data_to_kernel_t *datab;
		datab = NLMSG_DATA(nlh);
		printk("received payload len: %d data type is %d ,data is %s\n", size,datab->event, datab->info);
		test_send(port, &events, sizeof(events));
		break;
        default:
                break;
        }
	kfree_skb(skb);

}
#endif
static int __init my_module_init(void)
{
	printk(KERN_INFO "Initializing Netlink Socket");
	printk(KERN_INFO "Initializing Netlink Socket");
	printk(KERN_INFO "Initializing Netlink Socket");
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
	nl_sk = netlink_kernel_create(&init_net,NETLINK_TEST, 0, data_ready, NULL, THIS_MODULE);
	#else
 	nl_sk = netlink_kernel_create(NETLINK_TEST, 0, data_ready, NULL, THIS_MODULE);
	#endif
	setup_timer(&test_timer, test_timer_func, 0);
	setup_timer(&test_timer, test_timer_func, 0);
	test_timer.expires = TIME_OUT;
	return 0;
}

static void __exit my_module_exit(void)
{
	printk(KERN_INFO "Goodbye");
	sock_release(nl_sk->sk_socket);
	del_timer_sync(&test_timer);
}

module_init(my_module_init);
module_exit(my_module_exit);

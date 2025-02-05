#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/inet.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/slab_def.h>
#include <linux/jhash.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <net/secure_seq.h>

static struct proc_dir_entry *example_base_proc = NULL;
static struct proc_dir_entry *example_ip_proc = NULL;
static struct proc_dir_entry *example_mask_proc = NULL;
static struct proc_dir_entry *example_port_proc = NULL;

static __be32 filter_ip = 0;
static int filter_mask = 32;
static __be32 filter_mask_long = htonl(GENMASK(31, 0));
static int filter_port = 0; //小端

// 判断ip地址是否与filter_ip同网段，如果mask为32 则相等
int filter_net_match(__be32 ip)
{
	if ((ip & filter_mask_long) == (filter_ip & filter_mask_long))
		return 1;

	return 0;
}

//判断port是否和filter_port一致
int filter_port_match(__be16 port)
{
	if (port == htons(filter_port))
		return 1;

	return 0;
}

unsigned int example_pre_routing_fn(void *priv,
		struct sk_buff *skb,
		const struct nf_hook_state *state)
{
	struct iphdr	*iph;
	struct tcphdr	*tph;
	struct udphdr	*uph;
	__u8 proto;
	__u32 tot_len;
	__u32 iph_len;
	__be32 saddr;
	__be32 daddr;
	__be16 sport;
	__be16 dport;

	// 获取ip头
	iph = ip_hdr(skb);

	//判断是否为ipv4, 理论上ip_rcv函数已经判断过可以不需要
	if (iph->version != 4)
		return NF_ACCEPT;

	//解析ip头信息
	proto = iph->protocol;
	tot_len = ntohs(iph->tot_len);
	iph_len = iph->ihl*4;
	saddr = iph->saddr;
	daddr = iph->daddr;

	//解析4层头信息
	if (proto == IPPROTO_TCP) {
		//确定数据空间足够大，可以读到对应数据
		if (!pskb_may_pull(skb, iph_len + sizeof(struct tcphdr))) {
			return NF_ACCEPT;
		}

		//获取tcp头
		tph = (struct tcphdr *)((char *)iph + iph_len);
		//ip_rcv()中已经设置skb->transport_header指针，可以直接获取
		//tph = tcp_hdr(skb);

		sport = tph->source;
		dport = tph->dest;
	} else if (proto == IPPROTO_UDP) {
		//确定数据空间足够大，可以读到对应数据
		if (!pskb_may_pull(skb, iph_len + sizeof(struct udphdr))) {
			return NF_ACCEPT;
		}

		//获取tcp头
		uph = (struct udphdr *)((char *)iph + iph_len);
		//ip_rcv()中已经设置skb->transport_header指针，可以直接获取
		//uph = udp_hdr(skb);

		sport = uph->source;
		dport = uph->dest;
	}

	if (filter_net_match(daddr)) {
		if (net_ratelimit())
			printk("filter_net_match\n");
	}

	if (filter_port_match(dport)) {
		if (net_ratelimit())
			printk("filter_port_match");
	}



	return NF_ACCEPT;
}

unsigned int example_local_in_fn(void *priv,
		struct sk_buff *skb,
		const struct nf_hook_state *state)
{
	return NF_ACCEPT;
}				   


unsigned int example_forward_fn(void *priv,
		struct sk_buff *skb,
		const struct nf_hook_state *state)
{

	return NF_ACCEPT;
}

unsigned int example_local_out_fn(void *priv,
		struct sk_buff *skb,
		const struct nf_hook_state *state)
{
	return NF_ACCEPT;
}

unsigned int example_post_routing_fn(void *priv,
		struct sk_buff *skb,
		const struct nf_hook_state *state)
{   
	return NF_ACCEPT;
}


static const struct nf_hook_ops example_hook_ops[] = {
	{
		.hook			= example_pre_routing_fn,
		.pf 			= NFPROTO_IPV4,
		.hooknum		= NF_INET_PRE_ROUTING,
		.priority		= NF_IP_PRI_FIRST + 1,
	},
	{
		.hook           = example_local_in_fn,
		.pf             = NFPROTO_IPV4,
		.hooknum        = NF_INET_LOCAL_IN,
		.priority       = NF_IP_PRI_FIRST,
	},
	{
		.hook           = example_forward_fn,
		.pf             = NFPROTO_IPV4,
		.hooknum        = NF_INET_FORWARD,
		.priority       = NF_IP_PRI_FIRST,
	},
	{
		.hook           = example_local_out_fn,
		.pf             = NFPROTO_IPV4,
		.hooknum        = NF_INET_LOCAL_OUT,
		.priority       = NF_IP_PRI_FIRST,
	},
	{
		.hook           = example_post_routing_fn,
		.pf             = NFPROTO_IPV4,
		.hooknum        = NF_INET_POST_ROUTING,
		.priority       = NF_IP_PRI_FIRST,
	},
};

static void unregister_net_hooks(void)
{
	nf_unregister_net_hooks(&init_net, example_hook_ops, ARRAY_SIZE(example_hook_ops));
	smp_wmb();

	return;
}

static int register_net_hooks(void)
{
	int ret = 0;

	ret = nf_register_net_hooks(&init_net, example_hook_ops, ARRAY_SIZE(example_hook_ops));
	if (ret < 0)
		printk("nf_register_net_hooks failed\n");

	return ret;
}

/////////////////////////////////////////

// 读取操作函数
static ssize_t example_filter_ip_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos)
{
	char output[20];
	int len;

	// 将当前的 IP 地址转为点分十进制格式
	len = snprintf(output, sizeof(output), "%pI4\n", &filter_ip);

	// 检查文件是否已经读完
	if (*pos >= len)
		return 0;

	// 将数据从内核空间复制到用户空间
	if (copy_to_user(buf, output + *pos, len - *pos))
		return -EFAULT;

	*pos = len;  // 更新文件指针
	return len;
}

// 写入操作函数
static ssize_t example_filter_ip_write(struct file *file, const char __user *buf,
		size_t count, loff_t *pos)
{
	char input[20];
	unsigned int a, b, c, d;
	unsigned int little_addr;
	int ret = 0;

	// 确保输入数据不超长
	if (count > sizeof(input) - 1)
		return -EINVAL;

	// 从用户空间读取数据
	if (copy_from_user(input, buf, count))
		return -EFAULT;

	input[count] = '\0';  // 确保字符串结束

	// 解析输入的 IP 地址

	ret = sscanf(input, "%d.%d.%d.%d", &a, &b, &c, &d);
	if (ret <= 0) {
		printk("sscanf error %d\n", ret);
		return -EINVAL;
	}

	//转换为小端地址
	little_addr = (a<<24)|(b<<16)|(c<<8)|(d<<0);

	//转换成大端并赋值
	filter_ip = htonl(little_addr);

	printk("ip 0x%x", filter_ip);

	return count;  // 返回写入字节数
}

// 文件操作结构体
static const struct file_operations example_filter_ip_ops = {
	.owner = THIS_MODULE,
	.read = example_filter_ip_read,
	.write = example_filter_ip_write,
};

static ssize_t example_filter_mask_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos)
{
	char output[10];
	int len = snprintf(output, sizeof(output), "%d\n", filter_mask);

	// 检查文件是否已经读完
	if (*pos >= len)
		return 0;

	// 将数据从内核空间复制到用户空间
	if (copy_to_user(buf, output + *pos, len - *pos))
		return -EFAULT;

	*pos = len;  // 更新文件指针
	return len;
}

static ssize_t example_filter_mask_write(struct file *file, const char __user *buf,
		size_t count, loff_t *pos)
{
	char input[10];
	int new_value;

	// 确保输入数据不超长
	if (count > sizeof(input) - 1)
		return -EINVAL;

	// 从用户空间读取数据
	if (copy_from_user(input, buf, count))
		return -EFAULT;

	input[count] = '\0';  // 确保字符串结束

	// 尝试将输入的字符串转换为整数
	if (kstrtoint(input, 10, &new_value))
		return -EINVAL;

	// 更新整数值
	if (new_value > 0 && new_value < 32) {
		filter_mask = new_value;
		filter_mask_long = htonl(GENMASK(31, 32-filter_mask));
	}

	printk("mask %d, long mask 0x%x\n", filter_mask, filter_mask_long);

	return count;  // 返回写入字节数

}



static struct file_operations example_filter_mask_ops = {
	.owner  	= THIS_MODULE,
	.read   	= example_filter_mask_read,
	.write  	= example_filter_mask_write,
};

static ssize_t example_filter_port_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos)
{
	char output[10];
	int len = snprintf(output, sizeof(output), "%d\n", filter_port);

	// 检查文件是否已经读完
	if (*pos >= len)
		return 0;

	// 将数据从内核空间复制到用户空间
	if (copy_to_user(buf, output + *pos, len - *pos))
		return -EFAULT;

	*pos = len;  // 更新文件指针
	return len;
}

static ssize_t example_filter_port_write(struct file *file, const char __user *buf,
		size_t count, loff_t *pos)
{
	char input[10];
	int new_value;

	// 确保输入数据不超长
	if (count > sizeof(input) - 1)
		return -EINVAL;

	// 从用户空间读取数据
	if (copy_from_user(input, buf, count))
		return -EFAULT;

	input[count] = '\0';  // 确保字符串结束

	// 尝试将输入的字符串转换为整数
	if (kstrtoint(input, 10, &new_value))
		return -EINVAL;

	// 更新整数值
	if (new_value >= 0 && new_value < 65536)
		filter_port = new_value;

	return count;  // 返回写入字节数

}


static struct file_operations example_filter_port_ops = {
	.owner  	= THIS_MODULE,
	.read   	= example_filter_port_read,
	.write  	= example_filter_port_write,
};

static int example_proc_init(void)
{
	int ret =0;
	example_base_proc = proc_mkdir("example", NULL);
	if (example_base_proc == NULL) {
		printk("example_base_proc error\n");
		ret = -1;
		goto exit;
	}   

	example_ip_proc = proc_create("filter_ip", 0644, example_base_proc, &example_filter_ip_ops);
	if (example_ip_proc == NULL)
	{
		printk("example_ip_proc error\n");
		ret = -1;
		goto filter_ip;
	}

	example_mask_proc = proc_create("mask", 0644, example_base_proc, &example_filter_mask_ops);
	if (example_mask_proc == NULL)
	{
		printk("example_mask_proc error\n");
		ret = -1;
		goto mask;
	}

	example_port_proc = proc_create("port", 0644, example_base_proc, &example_filter_port_ops);
	if (example_port_proc == NULL)
	{
		printk("example_port_proc error\n");
		ret = -1;
		goto port;
	}

	return ret;

port:
	remove_proc_entry("mask", example_base_proc);
mask:
	remove_proc_entry("filter_ip", example_base_proc);
filter_ip:
	remove_proc_entry("example", NULL);
exit:
	return ret;
}

static void example_proc_exit(void)
{
	remove_proc_entry("port", example_base_proc);
	remove_proc_entry("mask", example_base_proc);
	remove_proc_entry("filter_ip", example_base_proc);
	remove_proc_entry("example", NULL);

	return;
}

static int __init filter_example_init(void)
{
	int ret = 0;
	ret = example_proc_init();
	if (ret < 0)
		goto ret;

	ret = register_net_hooks();
	if (ret < 0)
		goto hook;

	return ret;

hook:
	example_proc_exit();
ret:
	return ret;
}

static void __exit filter_example_exit(void)
{
	unregister_net_hooks();
	example_proc_exit();

	return;
}

module_init(filter_example_init);
module_exit(filter_example_exit);
MODULE_LICENSE("GPL");

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/string.h>

#define MIRROR_VERSION		"0.1"
#define MIRROR_DESCRIPTION	"Ethernet NIC mirror module"

#define MIRROR_RX	0
#define MIRROR_TX	1
#define MIRROR_BOTH	2

#define MAX_NICS	9

extern int (*mirror_launch)(struct sk_buff *, int);

static char *src = NULL;
static char *dst = NULL;
static char *mode = "both";
static int debug = 0;

module_param(mode, charp, 0);
MODULE_PARM_DESC(mode, "Mirror Mode: tx/rx/both");
module_param(src, charp, 0);
MODULE_PARM_DESC(src, "Source NICs");
module_param(dst, charp, 0);
MODULE_PARM_DESC(dst, "Destination NICs");
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug: 1 enable, 0 disable");

struct mirror_tbl {
	int mode;
	struct net_device *src[MAX_NICS], *dst[MAX_NICS];
	int srcnu, dstnu;
} mirror;


static int do_mirror(struct sk_buff *skb, int func)
{
if(func == mirror.mode || mirror.mode == 2) {
int i = 0, j = 0;
for(i = 0; i < mirror.srcnu; i++){
	if(skb->dev == mirror.src[i]) {
		for(j = 0; j < mirror.dstnu; j++){
			struct sk_buff *tmp = skb_clone(skb, GFP_ATOMIC);
			tmp->dev = mirror.dst[j];
			if(func == 0) {
			tmp->data = skb_mac_header(tmp);
			tmp->len += tmp->mac_len;
			}
			dev_queue_xmit(tmp);
		}
	break;
	}
}
}
return 0;
}

static bool parse(void)
{
int i = 0, j = 0;
char *q;
char *tmp;
char p[16];
if(memcmp(mode, "both", 4) == 0)
	mirror.mode = MIRROR_BOTH;
else if(memcmp(mode, "tx", 2) == 0)
	mirror.mode = MIRROR_TX;
else if(memcmp(mode, "rx", 2) == 0)
	mirror.mode = MIRROR_RX;
else
	{
	printk("Unknown mode %s\n",mode);
	return false;
	}
if(src == NULL || dst == NULL || mode == NULL)
	return false;
if(!strchr(src, ','))
	{
	if(!(mirror.src[0] = dev_get_by_name(src)))
		{
		printk("Not found netdevice src %s\n",src);
		return false;
		}
	mirror.srcnu = 1;
	}
else
	{
	tmp = src;
	mirror.srcnu = 0;
	while((q = strchr(tmp, ',')))
		{
		memcpy(p, tmp, (q - tmp));
		p[(q - tmp)] = '\0';
		if(!(mirror.src[mirror.srcnu] = dev_get_by_name(p)))
			{
			printk("Not found netdevice src %s\n",p);
			return false;
			}
		tmp = q + 1;
		mirror.srcnu++;
		}
	if(!(mirror.src[mirror.srcnu] = dev_get_by_name(tmp)))
		{
		printk("Not found netdevice src %s\n",tmp);
		return false;
		}
	else
		mirror.srcnu++;
	}

if(!strchr(dst, ','))
	{
	if(!(mirror.dst[0] = dev_get_by_name(dst)))
		{
		printk("Not found netdevice dst %s\n",dst);
		return false;
		}
	mirror.dstnu = 1;
	}
else
	{
	tmp = dst;
	mirror.dstnu = 0;
	while((q = strchr(tmp, ',')))
		{
		memcpy(p, tmp, (q - tmp));
		p[(q - tmp)] = '\0';
		if(!(mirror.dst[mirror.dstnu] = dev_get_by_name(p)))
			{
			printk("Not found netdevice dst %s\n",p);
			return false;
			}
		tmp = q + 1;
		mirror.dstnu++;
		}
	if(!(mirror.dst[mirror.dstnu] = dev_get_by_name(tmp)))
		{
		printk("Not found netdevice dst %s\n",tmp);
		return false;
		}
	else
		mirror.dstnu++;
	}

for(i = 0; i < mirror.srcnu; i++)
	{
	for(j = 0; j < mirror.dstnu; j++)
		{
		if(mirror.src[i] == mirror.dst[j])
			{
			printk("Same src %s and dst %s\n",mirror.src[i]->name,mirror.dst[j]->name);
			return false;
			}
		}
	}

if(debug)
	{
	printk("debug src: ");
	for(i = 0; i < mirror.srcnu; i++)
		{
		printk("%s ",mirror.src[i]->name);
		}
	printk("dst: ");
	for(i = 0; i < mirror.dstnu; i++)
		{
		printk("%s ",mirror.dst[i]->name);
		}
	printk("\n");
	}
return true;
}

static void help(void)
{
printk("Usage: modprobe nicmirror "
	"mode=both src=eth0,eth2 dst=eth1,eth3,eth4 debug=1\n");
}

static int __init mirror_init(void)
{
if(parse()) {
	mirror_launch = do_mirror;
	printk("Load portmirror v%s with mode: %s src: %s dst: %s\n",MIRROR_VERSION, mode, src, dst);
	}
else
	help();
return 0;
}

static void __exit mirror_exit(void)
{
mirror_launch = 0;
printk("Unload nicmirror.\n");
}

module_init(mirror_init);
module_exit(mirror_exit);
MODULE_LICENSE("GPL");
MODULE_VERSION(MIRROR_VERSION);
MODULE_DESCRIPTION(MIRROR_DESCRIPTION ", v" MIRROR_VERSION);
MODULE_AUTHOR("Lei Liu, orphen_leiliu@msn.com");

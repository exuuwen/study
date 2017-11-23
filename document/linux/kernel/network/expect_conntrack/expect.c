#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/tcp.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("expect helper test");

struct test_proto {
    int type;
    int port;
    union nf_inet_addr addr;
};

static int test_help(struct sk_buff *skb,
        unsigned int protoff,
        struct nf_conn *ct,
        enum ip_conntrack_info ctinfo)
{
    unsigned int dataoff, datalen;
    const struct tcphdr *th;
    struct tcphdr _tcph;
    int ret;
    char *dt_ptr;
    struct nf_conntrack_expect *exp;
    int dir = CTINFO2DIR(ctinfo);
    struct test_proto prot = {0};
    uint16_t port = ntohs((uint16_t)prot.port);
    char test_buffer[512];

    printk("hahaha in teh help 1\n");

    if (ctinfo != IP_CT_ESTABLISHED
        && ctinfo != IP_CT_ESTABLISHED+IP_CT_IS_REPLY) {
        return NF_ACCEPT;
    }

    th = skb_header_pointer(skb, protoff, sizeof(_tcph), &_tcph);
    dataoff = protoff + th->doff * 4;
    datalen = skb->len - dataoff;
    dt_ptr = skb_header_pointer(skb, dataoff, datalen, test_buffer);
    //get proto header
    memcpy(&prot, dt_ptr, sizeof(struct test_proto));
    if (prot.type != 100) { 
        ret = NF_ACCEPT;
        goto out;
    }
    printk("hahaha in teh help 2\n");

    exp = nf_ct_expect_alloc(ct);
    port = ntohs((uint16_t)prot.port);
    nf_ct_expect_init(exp, NF_CT_EXPECT_CLASS_DEFAULT, AF_INET,
              &ct->tuplehash[dir].tuple.src.u3, &prot.addr,
              IPPROTO_TCP, NULL, &port);
    if (nf_ct_expect_related(exp) != 0)
        ret = NF_DROP;
    else
        ret = NF_ACCEPT;
out:
    return ret;
}
static const struct nf_conntrack_expect_policy test_policy = {
    .max_expected    = 10,
    .timeout    = 50 * 60,
};

#define PORT 12345

static struct nf_conntrack_helper test = {
    .name = "test",
    .me = THIS_MODULE,
    .tuple.src.l3num = AF_INET,
    //because helper = __nf_ct_helper_find(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);
    .tuple.src.u.tcp.port = cpu_to_be16(PORT),
    .tuple.dst.protonum = IPPROTO_TCP,
    .help = test_help,
    .expect_policy = &test_policy,
};

static void nf_conntrack_test_fini(void)
{
    nf_conntrack_helper_unregister(&test);
}

static int __init nf_conntrack_test_init(void)
{
    int ret = nf_conntrack_helper_register(&test);

    return ret;
}

module_init(nf_conntrack_test_init);
module_exit(nf_conntrack_test_fini);

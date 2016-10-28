/* ***************************************************************
 *
 * (C) 2004-10 - Luca Deri <deri@ntop.org>
 *
 * This code includes contributions courtesy of
 * - Amit D. Chaudhary <amit_ml@rajgad.com>
 * - Andrew Gallatin <gallatyn@myri.com>
 * - Brad Doctor <brad@stillsecure.com>
 * - Felipe Huici <felipe.huici@nw.neclab.eu>
 * - Francesco Fusco <fusco@ntop.org> (IP defrag)
 * - Helmut Manck <helmut.manck@secunet.com>
 * - Hitoshi Irino <irino@sfc.wide.ad.jp> (IPv6 support)
 * - Jakov Haron <jyh@cabel.net>
 * - Jeff Randall <jrandall@nexvu.com>
 * - Kevin Wormington <kworm@sofnet.com>
 * - Mahdi Dashtbozorgi <rdfm2000@gmail.com>
 * - Marketakis Yannis <marketak@ics.forth.gr>
 * - Matthew J. Roth <mroth@imminc.com>
 * - Michael Stiller <ms@2scale.net> (VM memory support)
 * - Noam Dev <noamdev@gmail.com>
 * - Siva Kollipara <siva@cs.arizona.edu>
 * - Vincent Carrier <vicarrier@wanadoo.fr>
 * - Eugene Bogush <b_eugene@ukr.net>
 * - Samir Chang <coobyhb@gmail.com>
 * - Ury Stankevich <urykhy@gmail.com>
 * - Raja Mukerji <raja@mukerji.com>
 * - Davide Viti <zinosat@tiscali.it>
 * - Will Metcalf <william.metcalf@gmail.com>
 * - Godbach <nylzhaowei@gmail.com>
 * - Nicola Bonelli <bonelli@antifork.org>
 * - Jan Alsenz
 * - valxdater@seznam.cz
 * - Vito Piserchia <vpiserchia@metatype.it>
 * - Guo Chen <johncg1983@gmail.com>
 * - Tom Zhang <tom.zhang@ericsson.com>
 * - Tyson Hua <tyson.hua@ericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */
#include <linux/version.h>
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18))
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif
#else
#include <linux/config.h>
#endif
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/socket.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/in6.h>
#include <linux/init.h>
#include <linux/filter.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/proc_fs.h>
#include <net/xfrm.h>
#include <net/sock.h>
#include <asm/io.h>      /* needed for virt_to_phys() */
#include <linux/sched.h> /* needed for schedule() */
#ifdef CONFIG_INET
#include <net/inet_common.h>
#endif
#include <net/ip.h>
#include <net/ipv6.h>

#include "pfring.h"
#include <linux/vmalloc.h>
#include <linux/netfilter.h>

#define PROC_INFO               "info"
#define PROC_DEV                "dev"
#define PROC_TRACE              "trace"

#define PROCFS_MAX_SIZE              12
static char procTraceBuffer_g[PROCFS_MAX_SIZE];


// Proc entry for ring module
struct proc_dir_entry* ring_proc_dir = NULL;
struct proc_dir_entry* ring_proc_dev_dir = NULL;
struct proc_dir_entry* ring_proc_info = NULL;
struct proc_dir_entry* ring_proc_trace = NULL;

static int ring_proc_get_info(char *, char **, off_t, int, int *, void *);
static void ring_proc_add(struct ring_info *pfr);
static void ring_proc_remove(struct ring_info *pfr);
static void ring_proc_init(void);
static void ring_proc_term(void);

// Forward
static struct proto_ops ring_ops;
static struct proto ring_proto;

/* *************************************************** */
// Kernel logger control
static PfTraceLevel pf_current_loglevel = PfDebug3;

#define PFRING_LOGGER_ERROR(fmt, ...) \
    do{ uprintk(PfError,   "[PF_RING] ERROR  :" fmt, ## __VA_ARGS__); }while(0)
#define PFRING_LOGGER_WARNING(fmt, ...) \
    do{ uprintk(PfWarning, "[PF_RING] WARNING:" fmt, ## __VA_ARGS__); }while(0)
#define PFRING_LOGGER_NOTICE(fmt, ...) \
    do{ uprintk(PfNotice,  "[PF_RING] NOTICE :" fmt, ## __VA_ARGS__); }while(0)
#define PFRING_LOGGER_INFO(fmt, ...) \
    do{ uprintk(PfInfo,    "[PF_RING] INFO   :" fmt, ## __VA_ARGS__); }while(0)
#define PFRING_LOGGER_DEBUG(fmt, ...) \
    do{ uprintk(PfDebug,   "[PF_RING] DEBUG  :" fmt, ## __VA_ARGS__); }while(0)
#define PFRING_LOGGER_DEBUG2(fmt, ...) \
    do{ uprintk(PfDebug2,  "[PF_RING] DEBUG2 :" fmt, ## __VA_ARGS__); }while(0)
#define PFRING_LOGGER_DEBUG3(fmt, ...) \
    do{ uprintk(PfDebug3,  "[PF_RING] DEBUG3 :" fmt, ## __VA_ARGS__); }while(0)

// Kernel logger wrapper
// Implemented a debug level based mechnism
static int uprintk(PfTraceLevel level, const char *fmt, ...)
{
    va_list args;
    int r;
    /* check the log level before do the printk */
    if (level > pf_current_loglevel)
    {
        return 0;
    }
    va_start(args, fmt);
    r = vprintk(fmt, args);
    va_end(args);
    return r;
}

static char *getloglevel_str(PfTraceLevel level)
{
    switch (level)
    {
        case PfNone:    return "NONE    ";
        case PfError:   return "ERROR   ";
        case PfWarning: return "WARNING ";
        case PfNotice:  return "NOTICE  ";
        case PfInfo:    return "INFO    ";
        case PfDebug:   return "DEBUG   ";
        case PfDebug2:  return "DEBUG2  ";
        case PfDebug3:  return "DEBUG3  ";
        default:        return "NO LEVEL";
    }
}

// This function is used to change the current PF_RING debug level, this
// function can be called from userspace applications to change debug level
// dynamically.
static void setPfloglevel(PfTraceLevel level)
{
    PfTraceLevel old;
    old = pf_current_loglevel;
    pf_current_loglevel = level ;
    printk("pfring log level is changed from %s to %s\n",
           getloglevel_str(old), getloglevel_str(level));
}

static struct ring_sock* pkt_sk(struct sock *sk)
{
    return (struct ring_sock *)sk;
}

/* *************************************************** */

/* ***** Code taken from other kernel modules ******** */
/**
 * rvmalloc copied from usbvideo.c
 */
static void *rvmalloc(unsigned long size)
{
    void *mem;
    unsigned long adr;
    unsigned long pages = 0;

    size = PAGE_ALIGN(size);
    mem = vmalloc_32(size);
    if (!mem)
    {
        return NULL;
    }

    memset(mem, 0, size); /* Clear the ram out, no junk to the user */
    adr = (unsigned long) mem;
    while (size > 0)
    {
        SetPageReserved(vmalloc_to_page((void *)adr));
        pages++;
        adr += PAGE_SIZE;
        size -= PAGE_SIZE;
    }
    return mem;
}

static void rvfree(void *mem, unsigned long size)
{
    unsigned long adr;
    unsigned long pages = 0;

    if (!mem)
    {
        return;
    }

    adr = (unsigned long) mem;
    while ((long) size > 0)
    {
        ClearPageReserved(vmalloc_to_page((void *)adr));
        pages++;
        adr += PAGE_SIZE;
        size -= PAGE_SIZE;
    }
    vfree(mem);
}

// This function will be automatically called when network socket is released.
static void ring_sock_destruct(struct sock *net_sock)
{
    struct ring_info * pfring_info;

    PFRING_LOGGER_NOTICE("Destroy socket begin\n");
    skb_queue_purge(&net_sock->sk_write_queue);
    skb_queue_purge(&net_sock->sk_receive_queue);

    pfring_info = (struct ring_info*) net_sock->sk_protinfo;

    net_sock->sk_protinfo = NULL;
    if (pfring_info)
    {
        if (pfring_info->memory_start)
            rvfree(pfring_info->memory_start, pfring_info->slots_info->tot_mem);
    }
    kfree(pfring_info);
    pfring_info = NULL;

    PFRING_LOGGER_NOTICE("Destroy socket done\n");
}

static unsigned int num_queued_pkts(const struct ring_info *pfr)
{
    if (pfr->slot_start != NULL)
    {
        // Though tot_insert and tot_read are bit64, however the diff between
        // them is under 32bit integer. so use 32bit to caculate them.
        u_int32_t tot_insert = pfr->slots_info->tot_insert;
        u_int32_t tot_read = pfr->slots_info->tot_read;

        if(tot_insert >= tot_read)
        {
            return(tot_insert - tot_read);
        }
        else
        {
            return(((unsigned int) -1) + tot_insert - tot_read);
        }
    }
    else
    {
        return 0;
    }
}

inline unsigned int get_free_memory(struct ring_info * pfr)
{
    FlowSlotInfo* slots_info = pfr->slots_info;
    u_int free_memory = 0;
    if (slots_info->insert_off > slots_info->recycle_off)
    {
        free_memory = slots_info->end_off - slots_info->insert_off + slots_info->recycle_off;
    }
    else if (slots_info->insert_off < slots_info->recycle_off)
    {
        free_memory =  slots_info->recycle_off - slots_info->insert_off;
    }
    else if (slots_info->insert_off == slots_info->read_off &&
             slots_info->tot_read == slots_info->tot_insert)
    {
        free_memory = slots_info->end_off;
    }

    return free_memory;
}

/*
 *
 * file system /proc handing
 *
 */
static void ring_proc_add(struct ring_info *pfr)
{
    if (ring_proc_dir != NULL)
    {
        char name[64];
        snprintf(name, sizeof(name), "%d.%d", pfr->pid, pfr->index);
        create_proc_read_entry(name, 0, ring_proc_dir, ring_proc_get_info, pfr);
        PFRING_LOGGER_NOTICE("proc create /proc/net/pf_ring/%s\n", name);
    }
}

static void ring_proc_remove(struct ring_info *pfr)
{
    if (ring_proc_dir != NULL)
    {
        char name[64];
        snprintf(name, sizeof(name), "%d.%d", pfr->pid, pfr->index);
        remove_proc_entry(name, ring_proc_dir);
        PFRING_LOGGER_NOTICE("proc removed /proc/net/pf_ring/%s\n", name);
    }
}

static int ring_proc_get_info(char *buf, char **start, off_t offset,
                  int len, int *unused, void *data)
{
    int rlen = 0;
    struct ring_info *pfr;
    FlowSlotInfo *fsi;

    if (data == NULL)
    {
        /* /proc/net/pf_ring/info */
        rlen  = sprintf(buf,         "PF_RING Version     : %d\n", RING_VERSION);
        rlen += sprintf(buf + rlen,  "Slot header size    : %zu\n\n", sizeof(FlowSlot));
    }
    else
    {
        /* detailed statistics about a PF_RING */
        pfr = (struct ring_info *)data;
        if(data)
        {
            fsi = pfr->slots_info;
            if(fsi) {
                rlen  = sprintf(buf,        "Bound Device     : %s\n", pfr->dev_name);
                rlen += sprintf(buf + rlen, "Device if index  : %d\n", pfr->sock->sk->sk_bound_dev_if);
                rlen += sprintf(buf + rlen, "Slot Version     : %d [%d]\n", fsi->version, RING_VERSION);
                rlen += sprintf(buf + rlen, "Active           : %d\n", pfr->active);
                rlen += sprintf(buf + rlen, "Snap length      : %u\n", pfr->snap_length);
                rlen += sprintf(buf + rlen, "Tot Memory       : %d\n", fsi->tot_mem);
                rlen += sprintf(buf + rlen, "Free Memory      : %u\n", get_free_memory(pfr));
                rlen += sprintf(buf + rlen, "Queued packets   : %u\n", num_queued_pkts(pfr));
                rlen += sprintf(buf + rlen, "Tot Pkt Lost     : %lu\n", (unsigned long)fsi->tot_lost);
                rlen += sprintf(buf + rlen, "Tot Insert       : %lu\n", (unsigned long)fsi->tot_insert);
                rlen += sprintf(buf + rlen, "Tot Read         : %lu\n", (unsigned long)fsi->tot_read);
                rlen += sprintf(buf + rlen, "Tot Fwd Ok       : %d\n", atomic_read(&pfr->tot_fwd_ok));
                rlen += sprintf(buf + rlen, "Tot Fwd Errors   : %d\n", atomic_read(&pfr->tot_fwd_notok));
                rlen += sprintf(buf + rlen, "Tot Fwd > MTU    : %d\n", atomic_read(&pfr->tot_fwd_large));
                rlen += sprintf(buf + rlen, "Insert offset    : %u\n", fsi->insert_off );
                rlen += sprintf(buf + rlen, "Read offset      : %u\n", fsi->read_off );
                rlen += sprintf(buf + rlen, "Forward offset   : %u\n", fsi->forward_off );
                rlen += sprintf(buf + rlen, "Recycle offset   : %u\n", fsi->recycle_off );
                rlen += sprintf(buf + rlen, "End offset       : %u\n", fsi->end_off );
           }
           else
           {
               rlen = sprintf(buf, "WARNING fsi == NULL\n");
           }
      }
      else
      {
          rlen = sprintf(buf, "WARNING data == NULL\n");
      }
    }
    // keep rlen no bigger than PAGE_SIZE is important
    return rlen;
}

static int ring_proc_trace_read(char *buf, char **start, off_t offset,
                  int len, int *unused, void *data)
{
    int rlen = 0;
    int i;

    rlen += sprintf(buf + rlen, "Current log level : %s\n",
                    getloglevel_str(pf_current_loglevel));

    rlen += sprintf(buf + rlen, "\n------ Echo Corresponding Number to Change Log Level ------\n");
    for (i = PfError; i < PfDebug3; ++i )
        rlen += sprintf(buf + rlen, "%d %s\n", i, getloglevel_str(i));
    rlen += sprintf(buf + rlen, "------  Example: `echo 2 > /proc/net/pf_ring/trace`  ------\n");

    return rlen;
}

static int ring_proc_trace_write(struct file *file, const char *buffer,
                                 unsigned long count, void *data)
{
    char * endp;
    unsigned long procfsBufferSize;
    unsigned long newLevel;

    /* get buffer size */
    procfsBufferSize = count;
    if (procfsBufferSize > PROCFS_MAX_SIZE )
    {
        procfsBufferSize = PROCFS_MAX_SIZE;
    }
    /* write data to the buffer */
    if ( copy_from_user(procTraceBuffer_g, buffer, procfsBufferSize) )
    {
        return -EFAULT;
    }

    newLevel = simple_strtoul(procTraceBuffer_g, &endp, 0);

    if (newLevel <= PfNone || newLevel >= PfDebug3)
        PFRING_LOGGER_NOTICE("New trace level is wrong:%s\n");
    else
        setPfloglevel( (PfTraceLevel)newLevel );

    return procfsBufferSize;
}

static void ring_proc_init(void)
{
    ring_proc_dir = proc_mkdir("pf_ring", init_net.proc_net);

    if (ring_proc_dir)
    {
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
        ring_proc_dir->owner = THIS_MODULE;
#endif

        ring_proc_dev_dir = proc_mkdir(PROC_DEV, ring_proc_dir);

        // create /proc/net/pf_ring/info
        ring_proc_info = create_proc_read_entry(PROC_INFO, 0,
                                           ring_proc_dir,
                                           ring_proc_get_info, NULL);

        if (!ring_proc_info)
        {
            PFRING_LOGGER_WARNING("proc_init, unable to register proc file\n");
        }
        else
        {
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
            ring_proc_info->owner = THIS_MODULE;
#endif
            PFRING_LOGGER_WARNING("proc_init, registered /proc/net/pf_ring/\n");
        }

        // create /proc/net/pf_ring/trace
        ring_proc_trace = create_proc_entry(PROC_TRACE, 0644, ring_proc_dir);
        if (!ring_proc_trace)
        {
            PFRING_LOGGER_WARNING("Unable to register proc trace control file\n");
        }
        else
        {
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
            ring_proc_info->owner = THIS_MODULE;
#endif
            ring_proc_trace->write_proc  = ring_proc_trace_write;
            ring_proc_trace->read_proc = ring_proc_trace_read;

            PFRING_LOGGER_NOTICE("Registered /proc/net/pf_ring/info\n");
        }

    }
    else
    {
        PFRING_LOGGER_WARNING("Unable to create /proc/net/pf_ring\n");
    }
}

static void ring_proc_term(void)
{
    if (ring_proc_info != NULL)
    {
        remove_proc_entry(PROC_INFO, ring_proc_dir);
        PFRING_LOGGER_INFO("proc_term, removed /proc/net/pf_ring/%s\n", PROC_INFO);
    }

    if (ring_proc_dev_dir != NULL)
    {
        remove_proc_entry(PROC_DEV, ring_proc_dir);
        PFRING_LOGGER_INFO("proc_term, removed /proc/net/pf_ring/%s\n", PROC_DEV);
    }

    if (ring_proc_trace != NULL)
    {
        remove_proc_entry(PROC_TRACE, ring_proc_dir);
        PFRING_LOGGER_INFO("proc_term, remove /proc/net/pf_ring/%s\n", PROC_TRACE);
    }

    if (ring_proc_dir != NULL)
    {
        remove_proc_entry("pf_ring", init_net.proc_net);
        PFRING_LOGGER_INFO("proc_term, remove /proc/net/pf_ring\n");
    }
}
/* fake rtable used for ip_fragment */
static struct rtable fake_rtable;

static unsigned int fake_mtu(const struct dst_entry *dst)
{
    return dst->dev->mtu;
}

static struct dst_ops fake_dst_ops = {
    .family = AF_INET,
    .protocol = cpu_to_be16(ETH_P_IP),
    .mtu = fake_mtu,
};

static void fake_rtable_init(void)
{
    struct rtable *rt = &fake_rtable;

    atomic_set(&rt->dst.__refcnt, 1);
    rt->dst.path = &rt->dst;
    rt->dst.ops = &fake_dst_ops;
}

static void fake_rtable_init_dev(struct net_device *dev)
{
    fake_rtable.dst.dev = dev;
}

static void skb_rtable_init(struct sk_buff *skb)
{
    skb_dst_set_noref(skb, &(fake_rtable.dst));
}

static int pf_queue_xmit(struct sk_buff *skb)
{
    struct ethhdr *ethhdr;
    int ret;
	unsigned char tmp_mac[ETH_HLEN];

    skb_push(skb, ETH_HLEN);
    PFRING_LOGGER_DEBUG3("pf_queue_xmit: skb->len %d\n", skb->len);

    ethhdr = (struct ethhdr*)skb->data;
	memcpy (tmp_mac, ethhdr->h_dest, ETH_ALEN);
	memcpy (ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
	memcpy (ethhdr->h_source, tmp_mac, ETH_ALEN);
	
    ethhdr->h_proto = htons(0x0800);
    ret = dev_queue_xmit(skb);
    if(ret > 0)
    {	
        PFRING_LOGGER_DEBUG3("dev_queue_xmit failed\n");
        return -1;
    }

    return 0;	
}

static int forward_slot_packet(struct ring_info *pfr, FlowSlot *slot)
{
    struct sk_buff *skb = slot->skb;
    int ret = 0;
    struct net_device *dev;

	if (unlikely(skb == NULL))
	{
		ret = -ENXIO;
	    goto out;
    }

	dev = dev_get_by_index(&init_net, pfr->sock->sk->sk_bound_dev_if);
    if (unlikely(dev == NULL))
    {
        ret = -ENXIO;
        goto out_free;
    }
	else
	    dev_put(dev);

    /* Prepare to forward skb  */
    skb->dev = dev;
	/*for some ip_fragment reason:sock->sk->sk_wmem_alloc*/
    //skb->sk = pfr->sock->sk;
    skb->pkt_type = PACKET_OUTGOING;
    /* copy changed part back to skb */
    if (unlikely(skb_is_nonlinear(skb))) 
    {
        skb_store_bits(skb, 0, slot->bucket, slot->snap_len);
    } 
    else 
    {
        skb_copy_to_linear_data_offset(skb, 0, slot->bucket, slot->snap_len);
    }
    
	//TODO?
    //iph = ip_hdr(skb);
    //ip_send_check(iph);
    slot->skb = NULL;
    smp_mb();

    // It is very strange the lock is necessary here, if no lock, kernel
    // will always crash
    // dev_queue_xmit must be called with interrupts enabled, which means
    // it can't be called with spinlocks held.
    
    PFRING_LOGGER_DEBUG3("forwarding packet\n");
    if (skb->len > dev->mtu)
    {
        PFRING_LOGGER_DEBUG3("in the fragment\n");
        fake_rtable_init_dev(dev);
        skb_rtable_init(skb);
        ret = ip_fragment(skb, pf_queue_xmit);
        atomic_inc(&pfr->tot_fwd_large);
	}
    else
    {
        ret = pf_queue_xmit(skb);
		PFRING_LOGGER_DEBUG3("pf_queue_xmit ret:%d\n", ret);
    }

    goto out;

out_free:
    kfree_skb(skb);
out:
	slot->slot_state = RECYCLABLE;
    if (ret)
        atomic_inc(&pfr->tot_fwd_notok);
    else
        atomic_inc(&pfr->tot_fwd_ok);

    return ret;
}

static void send_queued_packets(struct ring_info* pfr, u_int32_t tx_start_off, u_int32_t tx_end_off)
{
    u_int32_t forward_off;
    u_int32_t next_forward_off;
    FlowSlot *theSlot = NULL;
    
    forward_off = tx_start_off;
    
    while (forward_off != tx_end_off)
    {
        theSlot = (FlowSlot*)(pfr->slot_start + forward_off);

        /* Find the next slot before doing anything on the current slot to avoid
         * recycle_off going beyond forward_off
         */
        next_forward_off = forward_off + theSlot->slot_len;
        smp_mb();
        switch (theSlot->slot_state)
        {
        case EMPTY:
            theSlot->slot_state = RECYCLABLE;
            
            if (theSlot->skb)
                kfree_skb(theSlot->skb);
            break;

        case FULL_TX:
            forward_slot_packet(pfr, theSlot);
            
            break;
        default:
            PFRING_LOGGER_ERROR("Send packet, fatal error, something wrong in ring handler,"
                                "tx_start_off=%u, tx_end_off=%u, forward_off=%u,"
                                "pfr recycle_off=%u, forward_off=%u,read_off=%u,insert_off=%u,end_off=%u, slot_state:%d, slot length: %u\n",
                                tx_start_off,
                                tx_end_off,
                                forward_off,
                                pfr->slots_info->recycle_off,                                
                                pfr->slots_info->forward_off,
                                pfr->slots_info->read_off,
                                pfr->slots_info->insert_off,
                                pfr->slots_info->end_off,
                                theSlot->slot_state,
                                theSlot->slot_len);
            BUG_ON(1);
            
            break;
        }

        forward_off = next_forward_off;
    }
}

static FlowSlot* allocate_slot(struct ring_info *pfr, int slot_len)
{
    u_int32_t slot_off = 0xFFFFFFFF;
    FlowSlotInfo* slots_info = pfr->slots_info;
    u_int32_t insert_off = slots_info->insert_off;
    u_int32_t read_off = slots_info->read_off;
    u_int32_t recycle_off = slots_info->recycle_off;
    FlowSlot * entry = NULL;

    if ((insert_off == read_off && insert_off == recycle_off && slots_info->tot_insert == slots_info->tot_read && (pfr->was_empty = true)) || insert_off > recycle_off)
    { /* Empty or not wrapped */
        if (slots_info->end_off - insert_off >= slot_len)
        {
            slot_off = insert_off;
        }
        else if (recycle_off > slot_len)
        {   /* wrap to beginning */
            slots_info->end_off = insert_off;
            slot_off = 0;
        }
    }
    else if (insert_off < recycle_off)
    {
        if (recycle_off - insert_off >= slot_len)
        {
            slot_off = insert_off;
        }
    }

    if (slot_off != 0xFFFFFFFF) /* Found a slot */
    {
        entry = (FlowSlot*)(pfr->slot_start + slot_off);
        entry->slot_len = slot_len;
        entry->slot_state = EMPTY;
        slots_info->insert_off = slot_off + slot_len;
    }

    return entry;
}

static inline void recycle_slots(struct ring_info *pfr)
{
    u_int32_t recycle_off = pfr->slots_info->recycle_off;
    FlowSlot* slot;

    while (recycle_off != pfr->slots_info->forward_off)
    {
        if (recycle_off == pfr->slots_info->end_off)
        {
            recycle_off = 0;
            pfr->slots_info->end_off = pfr->slots_info->tot_mem -  sizeof(FlowSlotInfo);
            continue;
        }

        slot = (FlowSlot*)(pfr->slot_start + recycle_off);

        switch (slot->slot_state)
        {
        case EMPTY:
        case FULL_TX:
            goto out;
            
            break;
        case RECYCLABLE:
            slot->slot_state = EMPTY;
            recycle_off += slot->slot_len;
            break;
            
        default:
            PFRING_LOGGER_ERROR("Send packet, fatal error, something wrong in ring handler,"
                                "recycle_off=%u, pfr recycle_off=%u, forward_off=%u, read_off=%u, insert_off=%u, end_off=%u, slot_state:%d, slot length: %u\n",
                                recycle_off,
                                pfr->slots_info->recycle_off,
                                pfr->slots_info->forward_off,
                                pfr->slots_info->read_off,
                                pfr->slots_info->insert_off,
                                pfr->slots_info->end_off,
                                slot->slot_state,
                                slot->slot_len);
            BUG_ON(1);
            break;
        }
    }
out:
    pfr->slots_info->recycle_off =  recycle_off;
}

/* skb->data point to IP layer
 * returns
 * 1 - sucessful add to slot or buffer is full
 * 0 - failed due to some reasons
 */
static int store_skb_to_slot(struct sk_buff *skb, struct ring_info * pfr)
{
    FlowSlot *empty_slot;
    u_int32_t tx_start_off = 0;
    u_int32_t tx_end_off = 0;
    unsigned char *pkt;
    unsigned int snap_len = min(skb->len, pfr->snap_length);
    unsigned int slot_len = ALIGN(snap_len + sizeof(FlowSlot), 64);

    write_lock(&pfr->buffer_lock);
    /* Do all the offset related moving the this critical area. */
    recycle_slots(pfr);  

    /* Get the range of slots that are ready for transmission. */
    tx_start_off = pfr->slots_info->forward_off;
    tx_end_off = pfr->slots_info->read_off;

    if (tx_end_off < tx_start_off)
    {
        /* Prevent crossing the end of ring buffer. */
        tx_end_off = pfr->slots_info->end_off;
        pfr->slots_info->forward_off = 0;
    }
    else
    {
        pfr->slots_info->forward_off = tx_end_off;
    }
    /* Get a free slot for storing the skb */
    empty_slot = allocate_slot(pfr, slot_len);
    
    if (empty_slot != NULL)
    {
        /* Ensure the insert_off change is before increasing tot_insert. Then
         * applcation could always get the correct slot.
         */
        smp_mb();           
        pfr->slots_info->tot_insert++;
    }
    
    write_unlock(&pfr->buffer_lock);

    if (unlikely(empty_slot == NULL))
    {
        PFRING_LOGGER_DEBUG("Copy skb, ring is full\n");
        pfr->slots_info->tot_lost++;
        kfree_skb(skb); /* Discarded */
    }
    else
    {
        pkt = &empty_slot->bucket[0];
        if (unlikely(skb_is_nonlinear(skb)))
        {
            skb_copy_bits(skb, 0, pkt, snap_len);
        }
        else
        {
            skb_copy_from_linear_data_offset(skb, 0, pkt, snap_len);
        }
        empty_slot->pkt_len = skb->len;
        empty_slot->snap_len = snap_len;
        empty_slot->pkt_off = sizeof(FlowSlot);
        empty_slot->skb = skb;
        /* This barrier is to make sure slot state update must be the last thing to
         * be done with the slot */
        smp_mb();

        empty_slot->slot_state = FULL_RX;
		
        /* Only try to wake up application when the ring was empty, because the
         * application is supposed to run in poll mode */
        if (pfr->was_empty)
        {
            pfr->was_empty = false;
            wake_up_interruptible(&pfr->waitqueue);
        }
    }

    if (tx_start_off != tx_end_off)
    {
		//printk("in the sends\n");
        send_queued_packets(pfr, tx_start_off, tx_end_off);
    }

    return 1;
}


static unsigned int run_filter(struct sk_buff *skb, const struct sock *sk)
{
    struct sk_filter *filter;
    unsigned int res = 1;

    skb_push(skb, ETH_HLEN);

    rcu_read_lock();
    filter = rcu_dereference(sk->sk_filter);
    if (filter != NULL)
	res = SK_RUN_FILTER(filter, skb);
    rcu_read_unlock();

    skb_pull(skb, ETH_HLEN);
    return res;
}


static struct sk_buff *pf_ip_defrag(struct sk_buff *skb)
{
    
    const struct iphdr *iph;
    unsigned int len;
    struct sk_buff *cloned;

    if (skb->protocol != htons(ETH_P_IP))    //check whether IP packet or not
    {
        printk("skb->protocol != htons(ETH_P_IP)\n");
        return skb;
    }

    if (!pskb_may_pull(skb, sizeof(struct iphdr)))   //check whether linger space enough for iphdr
    {

        printk("!pskb_may_pull(skb, sizeof(struct iphdr)\n");
        return skb;
    }

    iph = ip_hdr(skb);

    if (iph->ihl < 5 || iph->version != 4)    //check the version and header len
    {
        printk("iph->ihl < 5 || iph->version != 4\n");
        return skb;
    }

    if (!pskb_may_pull(skb, iph->ihl*4)) //check whether linger space enough for ip header
    {
        printk("!pskb_may_pull(skb, iph->ihl*4\n");
        return skb;
    }

    iph = ip_hdr(skb);
    len = ntohs(iph->tot_len);

    if (skb->len < len || len < (iph->ihl * 4)) //check whether len is correct
    {
        printk("!skb->len < len || len < (iph->ihl * 4)\n");
        return skb;
    }

    if (ip_is_fragment(iph)) // check the packet whether a fragment packet
    {
        printk("it is a fragment, share:%u, num:%u\n", skb_shared(skb), atomic_read(&skb->users));
        cloned = skb_clone(skb, GFP_ATOMIC);// check flag
        if (cloned)
        {
            if (ip_defrag(cloned, PF_DEFRAG_RING))
            {
                printk("yeah it is a fragment but not the last\n");
                return NULL;
            }
            else
            {
                skb = cloned;
                printk("it is the last fragment skb->len:%d, skb->data_len:%d\n", skb->len, skb->data_len);
            }
        }
    }

    return skb;   
}

// 1 : handled by PF_RING
// 0 : return to legacy IP stack.
static int skb_ring_handler(struct sk_buff *skb, struct sock *sk)
{
    int ret = 0;
    struct ring_info *ring = NULL;
    struct iphdr *iph = NULL;
    int len = 0;

    if (!skb || !sk)
    {
        return 0;
    }
    
	if (skb->len < sizeof(struct iphdr))
    {
        // IP packet length error, to linux IP stack
        return 0;
    }
    iph = ip_hdr(skb);
    len = ntohs(iph->tot_len);
    if (len > skb->len || len < sizeof(struct iphdr))
    {
        // Ip packet total length error
        return 0;
    }

    ring = (struct ring_info *)sk->sk_protinfo;
    __sync_synchronize();
	
    if(ring && !strcmp(ring->dev_name, skb->dev->name))
    {
        PFRING_LOGGER_DEBUG3("match the name: %s\n", ring->dev_name);	
			
        if (!ring->active)
        {
            PFRING_LOGGER_DEBUG3("Ring handler, ring socket is not in active status\n");
            return 0;
        }

        skb = pf_ip_defrag(skb);
        if(skb == NULL)
            return 0;

        if (run_filter(skb, sk) == 0)
        {
            PFRING_LOGGER_DEBUG3("Ring handler, bpf filter fail\n");
            return 0;
        }
		
		// if one is sucessful, then return handled
        PFRING_LOGGER_DEBUG3("skb->len:%d, skb->data_len:%d, ip->tot_len:%d\n", skb->len, skb->data_len, len);

        ret = store_skb_to_slot(skb, ring);
    }
       
    /*  0 = packet not handled, 1 handled by PF_RING */
    return ret;
}

static int packet_rcv(struct sk_buff *skb, struct net_device *dev,
		      struct packet_type *pt, struct net_device *orig_dev)
{
    int ret = 0;
    struct sock *sk = (struct sock*)pt->af_packet_priv;
 
    if (skb->pkt_type == PACKET_HOST || skb->pkt_type == PACKET_OTHERHOST) 
	{
        ret = skb_ring_handler(skb, sk);
	}
    return ret;
}

/*
 *
 * system operations handing
 *
 */

/* struct socket is a general BSD socket, struct sock is a network layer
 representation of sockets. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
static int ring_create(struct net * net, struct socket *sock, int protocol)
#else
static int ring_create(struct net * net, struct socket *sock, int protocol, int kern)
#endif
{
    struct sock *net_sock;
    struct ring_info *pfring_info;
    struct ring_sock *po;

    PFRING_LOGGER_NOTICE("Create socket\n");

    /* Supervisor admin? */
    if(!capable(CAP_NET_ADMIN))
    {
        PFRING_LOGGER_WARNING("Create socket permisstion deny");
        return -EPERM;
    }

    /* raw socket type must be specified */
    if(sock->type != SOCK_RAW)
    {
        PFRING_LOGGER_WARNING("Create socket, raw socket type must be specified for pfring\n");
        return -ESOCKTNOSUPPORT;
    }

    /* protocl must be specified */
    if(protocol != htons(ETH_P_ALL))
    {
        PFRING_LOGGER_WARNING("Create socket, protocol must be specified to ETH_P_ALL for pfring\n");
        return -EPROTONOSUPPORT;
    }

    /* get a network socket memory, this API is changed after 2.24 kernel
       here we use the format 2.6.26. */
    net_sock = sk_alloc(net, PF_RING, GFP_KERNEL, &ring_proto);
    if(net_sock == NULL)
    {
        PFRING_LOGGER_ERROR("Create socket, network socket memory allocate fail\n");
        return -ENOMEM;
    }

     /* Init the network layer to BSD sock */
    sock_init_data(sock, net_sock);

    /* Now assign the operations for BSD socket */
    sock->ops = &ring_ops;
    
    po = (struct ring_sock *)pkt_sk(net_sock);
    po->prot_hook.func = packet_rcv;
    po->prot_hook.af_packet_priv = net_sock;
    po->prot_hook.type = protocol;
    spin_lock_init(&po->bind_lock);
    dev_add_pack(&po->prot_hook);

    /* Private socket info for pfring */
    pfring_info = (struct ring_info *)kmalloc(sizeof(struct ring_info), GFP_KERNEL);
    if (pfring_info == NULL)
    {
        PFRING_LOGGER_ERROR("Create socket, allocate private memory for pfring socket fail\n");
        sk_free(net_sock);
        return -ENOMEM;
    }
    net_sock->sk_protinfo = (void *) pfring_info;
    /* Init the private info for pfring socket */
    memset(pfring_info, 0, sizeof(struct ring_info));
    pfring_info->active      = 0;
    pfring_info->pid         = current->tgid;
    pfring_info->dev_name[0] = '\0';
    pfring_info->sock        = sock; /* record the BSD socket pfring belongs to */
    pfring_info->buffer_size = PF_RING_BUFFER_SIZE;
    pfring_info->snap_length= PFRING_SNAP_LENGTH_DEFAULT;
    init_waitqueue_head(&pfring_info->waitqueue);

    net_sock->sk_family = PF_RING;
    net_sock->sk_destruct = ring_sock_destruct;
	net_sock->sk_bound_dev_if = 0;

    rwlock_init(&pfring_info->buffer_lock);
    pfring_info->index = 0;
    ring_proc_add(pfring_info);
    
    PFRING_LOGGER_NOTICE("Create socket, pfring socket creation done\n");
    return 0;
}

/* *********************************************** */
/* when the socket is close, this function will be called firstly.
   1, pfring_release begin
   2, sock_put will destroy the network layer sock, call the pfring_sock_destruct
   3, pfring_release done */
static int ring_release(struct socket *sock)
{
    struct sock *net_sock = sock->sk;
    struct ring_sock *po;
    struct ring_info *pfr = net_sock->sk_protinfo;

    PFRING_LOGGER_NOTICE("Release socket, socket release begin\n");
    if (net_sock == NULL)
    {
        PFRING_LOGGER_ERROR("Release socket, can't find net_sock\n");
        return -1;
    }

    po = (struct ring_sock *)pkt_sk(net_sock);
    spin_lock(&po->bind_lock);
    dev_remove_pack(&po->prot_hook);
    spin_unlock(&po->bind_lock);
	
    /* mark the BSD socket's net sock to null. */
    sock->sk = NULL;
    
	if (pfr)
        ring_proc_remove(pfr);

	//TODO:move all packet in pfring /*forward_off ----> insert_off*/

    /* if it is the last reference, destroy it. It will call
       net_sock->sk_destruct = pfring_sock_destruct automatically. */
    //printk("net sock ref:%d\n", atomic_read(&net_sock->sk_refcnt));
    //printk("sk_wmem_alloc ref:%d\n", atomic_read(&net_sock->sk_wmem_alloc));
    
    sock_put(net_sock);
    
    PFRING_LOGGER_NOTICE("Release socket, done\n");
    return 0;
}

/* Bind to a device */
static int ring_bind(struct socket *sock, struct sockaddr *sa, int addr_len)
{
    struct sock * net_sock = sock->sk;
    struct net_device *dev = NULL;
    struct ring_info * pfring_info;
    unsigned int total_mem;
    unsigned int buffer_size;

    PFRING_LOGGER_NOTICE("Bind socket, begin\n");
    if (addr_len != sizeof(struct sockaddr) || sa->sa_data == NULL)
    {
        PFRING_LOGGER_WARNING("Bind socket, addr length error or name null\n");
        return -EINVAL;
    }

    if (sa->sa_family != PF_RING)
    {
        PFRING_LOGGER_WARNING("Bind socket, family error\n");
        return -EINVAL;
    }

    /* __dev_get_by_name will not increase the reference number to the dev,
       while dev_get_by_name will increase the reference number.
       if dev_get_by_name or dev_get_by_index is used, then after that,
       dev_put must be called in order to decrease the reference number to that
       device. */
    sa->sa_data[sizeof(sa->sa_data)-1] = '\0';
    if ((dev = dev_get_by_name(&init_net, sa->sa_data)) == NULL)
    {
        PFRING_LOGGER_ERROR("Bind socket, device %s search failed\n", sa->sa_data);
    }
    else
    {
		net_sock->sk_bound_dev_if = dev->ifindex;
		dev_put(dev);
        PFRING_LOGGER_NOTICE("Bind socket, dev %s is found for pfring\n", sa->sa_data);
    }

    /*
     * |<-----slot_header----->|+--slot---+--slot---+--slot---+...|
     */
    pfring_info = (struct ring_info *) net_sock->sk_protinfo;

    memset(pfring_info->dev_name, 0x00, sizeof(pfring_info->dev_name));
    memcpy(pfring_info->dev_name, dev->name, sizeof(pfring_info->dev_name));
    pfring_info->dev_name[sizeof(pfring_info->dev_name) - 1] = '\0';

    /* do a safe check now */
    if (pfring_info->sock != sock)
    {
        PFRING_LOGGER_ERROR("Bind socket, fatal error, sock create and bind different pfring\n");
        return -EINVAL;
    }

    buffer_size = pfring_info->buffer_size;
    total_mem = PAGE_ALIGN(buffer_size + sizeof(FlowSlotInfo));

    pfring_info->memory_start = rvmalloc(total_mem);
    if (pfring_info->memory_start == NULL)
    {
        PFRING_LOGGER_WARNING("Bind socket, fatal error, not enough memory for the pfring socket\n");
        return -1;
    }

    PFRING_LOGGER_NOTICE("Bind socket, pfring total memory: %u\n", total_mem);
    
	pfring_info->slots_info = (FlowSlotInfo *) pfring_info->memory_start;
    pfring_info->slot_start = (unsigned char *) (pfring_info->memory_start + sizeof(FlowSlotInfo));
    atomic_set(&pfring_info->tot_fwd_ok, 0);
    atomic_set(&pfring_info->tot_fwd_notok, 0);
    atomic_set(&pfring_info->tot_fwd_large, 0);
    pfring_info->was_empty = true;
    /* memory is already reset in rvmalloc, make sure some fields are populated. */
    pfring_info->slots_info->version = RING_VERSION;
    pfring_info->slots_info->tot_mem = total_mem;
    pfring_info->slots_info->end_off = total_mem - sizeof(FlowSlotInfo);
    pfring_info->slots_info->insert_off = 0;
    pfring_info->slots_info->read_off = 0;
    pfring_info->slots_info->forward_off = 0;
    pfring_info->slots_info->recycle_off = 0;
    pfring_info->slots_info->tot_insert = 0;
    pfring_info->slots_info->tot_read = 0;
    pfring_info->slots_info->tot_lost = 0;

    PFRING_LOGGER_NOTICE("Bind socket, done\n");
    return 0;
}

static int ring_mmap(struct file *file, struct socket *sock, struct vm_area_struct *vma)
{
    struct sock *net_sock = sock->sk;
    /* get pfring information from the network socket private info */
    struct ring_info *pfring_info = (struct ring_info *) net_sock->sk_protinfo;
    int rc;
    unsigned long size = (unsigned long)(vma->vm_end - vma->vm_start);
    unsigned long start;
    unsigned long page;
    unsigned char * p;

    PFRING_LOGGER_NOTICE("Ring mmap, begin\n");
    if(size % PAGE_SIZE)
    {
        PFRING_LOGGER_WARNING("Ring mmap, memory is not align with page size\n");
        return(-EINVAL);
    }

    if (pfring_info->memory_start == NULL)
    {
        PFRING_LOGGER_WARNING("Ring mmap,  memory start from NULL\n");
        return -EINVAL;
    }

    /* do a safe check */
    if(size > pfring_info->slots_info->tot_mem)
    {
        PFRING_LOGGER_WARNING("Ring mmap, mmap error due to mmap size error\n");
        return(-EINVAL);
    }

    /* not swap out, lock it */
    vma->vm_flags |= VM_LOCKED;
    start = vma->vm_start;

    p = (unsigned char *)pfring_info->memory_start;
    while(size > 0)
    {
        /* the the physical page number */
        page = vmalloc_to_pfn(p);
        rc = remap_pfn_range(vma, start, page, PAGE_SIZE, PAGE_SHARED);
        if (rc != 0)
        {
            PFRING_LOGGER_WARNING("Ring mmap, fatal error, mmap failed for pfring socket\n");
            return(-EAGAIN);
        }

        start += PAGE_SIZE;
        p += PAGE_SIZE;
        if (size > PAGE_SIZE)
        {
            size -= PAGE_SIZE;
        }
        else
        {
            size = 0;
        }
    }
    PFRING_LOGGER_NOTICE("Ring mmap, done\n");
    return 0;
}

static int ring_recvmsg(struct kiocb *iocb, struct socket *sock,
            struct msghdr *msg, size_t size, int flags)
{
    struct sk_buff *skb;
	int err = -EINVAL;
    struct ring_info *ring_info =(struct ring_info *) sock->sk->sk_protinfo;
    FlowSlot * entry = (FlowSlot *)(ring_info->slot_start + ring_info->slots_info->read_off);
    int offset = 0;

	skb = entry->skb;
	if (!skb)
		return err;

    /* Get the offset from the first iovec */
    if (msg->msg_iovlen > 1 && msg->msg_iov[0].iov_base == NULL)
    {
        offset = msg->msg_iov[0].iov_len;
        msg->msg_iov[0].iov_len = 0; /* Skip the first iovec */
    }

    err = skb_copy_datagram_iovec(skb, offset, msg->msg_iov, size - offset);
	if (err < 0)
    {
		kfree_skb(skb);
        entry->skb = NULL;
		return err;
	}

    if (!(flags & MSG_PEEK))
    {
        kfree_skb(skb);
        entry->skb = NULL;
        entry->slot_state = EMPTY;
    }

    if (size < skb->len) msg->msg_flags |= MSG_TRUNC;
    
	return (flags & MSG_TRUNC) ? skb->len : (size - offset);
}

// manual close or abnormal process quit will triger ring_release
// 1, mannual close down, no race condition since one payload inst, one thread. out of ring_poll
// 2, abnormal quit like process crash or kill always occur out of ring_poll
// above all, no need to protect the shared resource here
unsigned int ring_poll(struct file *file, struct socket *sock, poll_table * wait)
{
    struct ring_info *pfring_info;

    pfring_info =(struct ring_info *) sock->sk->sk_protinfo;
    if (!pfring_info->active)
    {
        PFRING_LOGGER_DEBUG3("Ring poll, pf_ring not active\n");
        return 0;
    }
    
    if (pfring_info->slot_start != NULL)
    {
        PFRING_LOGGER_DEBUG3("Ring poll, pfring socket havn't been init sucessfully\n");
        return 0;
    }

    /* turn on the pfring automatically when poll it.  */
    /* we have chance now to do the transmit in process context before going to poll  */

    if (pfring_info->slots_info->tot_insert == pfring_info->slots_info->tot_read)
    {
         poll_wait(file, &pfring_info->waitqueue, wait);
    }
    else
    {
        return (POLLIN | POLLRDNORM);
    }

    return 0;
}

/* Code taken/inspired from core/sock.c */
static int ring_setsockopt(struct socket *sock,
                           int level,
                           int optname,
                           char __user * optval,
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
                           int optlen
#else
                           unsigned int optlen
#endif
                          )

{
    int ret = 0;
    unsigned int b_size;
    struct ring_info *pfring_info;
    struct sock *net_sock = sock->sk;

    PFRING_LOGGER_DEBUG("Setsockopt, begin\n");
    if (net_sock)
    {
        pfring_info = net_sock->sk_protinfo;
        if (pfring_info == NULL)
        {
            PFRING_LOGGER_ERROR("Setsockopt, sk_protinfo NULL\n");
            return -EFAULT;
        }
    }
    else
    {
        PFRING_LOGGER_ERROR("Setsockopt, net_sock NULL\n");
        return -EFAULT;
    }

    switch (optname)
    {
    // ACTIVE and DEACTIVE could only be after ring create, bind, index configuration
    case SO_SET_PFRING_ACTI_SOCK:
        pfring_info->active = 1;
        PFRING_LOGGER_NOTICE("SO_SET_PFRING_ACTI_SOCK, activate socket!\n");
        break;

    // normal process quit, call DEACTIVE
    case SO_SET_PFRING_DEACTI_SOCK:
        pfring_info->active = 0;
        PFRING_LOGGER_NOTICE("SO_SET_PFRING_DEACTI_SOCK, deactive socket!\n");
        break;

    case SO_SET_PFRING_BUFFER_SIZE:
        if (optlen != sizeof(unsigned int))
        {
            PFRING_LOGGER_ERROR("SO_SET_PFRING_BUCKET_LEN, argument size error\n");
            return -EINVAL;
        }
        else
        {
            if (copy_from_user(&b_size, optval, optlen))
            {
                PFRING_LOGGER_ERROR("SO_SET_PFRING_BUCKET_LEN, argument copy error\n");
                return -EFAULT;
            }

            if (b_size > 0 || b_size <= PF_RING_BUFFER_SIZE)
            {
                PFRING_LOGGER_NOTICE("SO_SET_PFRING_BUFFER_SIZE, buffer size is set from %d to %d\n",  pfring_info->buffer_size, b_size);
                pfring_info->buffer_size = b_size; 
            }
            else
	    {
                PFRING_LOGGER_ERROR("SO_SET_PFRING_BUFFER_SIZE, invalid buffer size\n");
                return -EFAULT;
            }
        }
        break;

    case SO_SET_PFRING_SNAP_LENGTH:
        if (optlen != sizeof(unsigned int))
        {
            PFRING_LOGGER_ERROR("SO_SET_PFRING_SNAP_LENGTH, argument size error\n");
            return -EINVAL;
        }
        else
        {
            if (copy_from_user(&b_size, optval, optlen))
            {
                PFRING_LOGGER_ERROR("SO_SET_PFRING_SNAP_LENGTH, argument copy error\n");
                return -EFAULT;
            }
            if (b_size > 0 && b_size <= PFRING_SNAP_LENGTH_DEFAULT)
            {
                PFRING_LOGGER_NOTICE("SO_SET_PFRING_SNAP_LENGTH, buffer size is set from %d to %d\n", pfring_info->snap_length, b_size);
                pfring_info->snap_length = b_size;
            }
            else
            {
                PFRING_LOGGER_ERROR("SO_SET_PFRING_SNAP_LENGTH, invalid buffer size\n");
                return -EFAULT;
            }
        }
        break;
	
    default:
        ret = sock_setsockopt(sock, level, optname, optval, optlen);
        break;
    }

    return ret;
}

static int ring_getsockopt(struct socket *sock,
               int level, int optname,
               char __user * optval, int __user * optlen)
{
    int loglevel;
    unsigned int b_size;
    struct ring_info *pfring_info =(struct ring_info *) sock->sk->sk_protinfo;

    PFRING_LOGGER_DEBUG("Getsockopt, pfring getsockopt begin\n");
    if (!pfring_info)
    {
        return -EINVAL;
    }

    switch (optname)
    {
        case SO_GET_PFRING_BUFFER_SIZE:
            b_size = pfring_info->buffer_size;
            if (copy_to_user(optval, &b_size, sizeof(b_size)))
            {
                PFRING_LOGGER_ERROR("SO_GET_PFRING_BUFFER_SIZE, argument copy error\n");
                return -EFAULT;
            }
            break;

        case SO_GET_PFRING_SNAP_LENGTH:
            b_size = pfring_info->snap_length;
            if (copy_to_user(optval, &b_size, sizeof(b_size)))
            {
                PFRING_LOGGER_ERROR("SO_GET_PFRING_SNAP_LENGTH, argument copy error\n");
                return -EFAULT;
            }
            break;

        case SO_GET_PFRING_LOG_LEVEL:
            loglevel = pf_current_loglevel;
            if (copy_to_user(optval, &loglevel, sizeof(int)))
            {
                PFRING_LOGGER_ERROR("SO_GET_PFRING_LOG_LEVEL, argument copy error\n");
                return -EFAULT;
            }
            break;

        default:
            PFRING_LOGGER_ERROR("Getsockopt, wrong optname %d\n", optname);
            return -EPROTOTYPE;
    }

    PFRING_LOGGER_DEBUG("Getsockopt, pfring getsockopt done\n");
    return 0;
}

static int ring_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
    switch (cmd) 
    {
    #ifdef CONFIG_INET
    case SIOCADDRT:
    case SIOCDELRT:
    case SIOCDARP:
    case SIOCGARP:
    case SIOCSARP:
    case SIOCGIFADDR:
    case SIOCSIFADDR:
    case SIOCGIFBRDADDR:
    case SIOCSIFBRDADDR:
    case SIOCGIFNETMASK:
    case SIOCSIFNETMASK:
    case SIOCGIFDSTADDR:
    case SIOCSIFDSTADDR:
    case SIOCSIFFLAGS:
        return inet_dgram_ops.ioctl(sock, cmd, arg);
#endif
    default:
        return -ENOIOCTLCMD;
    }

    return 0;
}

static struct proto_ops ring_ops = {
    .family = PF_RING,
    .owner = THIS_MODULE,

    /* Operations that make no sense on ring sockets. */
    .connect = sock_no_connect,
    .socketpair = sock_no_socketpair,
    .accept = sock_no_accept,
    .getname = sock_no_getname,
    .listen = sock_no_listen,
    .shutdown = sock_no_shutdown,
    .sendpage = sock_no_sendpage,
    .sendmsg = sock_no_sendmsg,

    /* Now the operations that really occur. */
    .release = ring_release,
    .bind = ring_bind,
    .mmap = ring_mmap,
    .poll = ring_poll,
    .setsockopt = ring_setsockopt,
    .getsockopt = ring_getsockopt,
    .ioctl = ring_ioctl,
    .recvmsg = ring_recvmsg,
};

static struct net_proto_family ring_family_ops = {
    .family = PF_RING,
    .create = ring_create,
    .owner = THIS_MODULE,
};

static struct proto ring_proto = {
      .name = "PF_RING",
      .owner = THIS_MODULE,
      .obj_size = sizeof(struct ring_sock),
};

/*
 * A device event has occurred
 */
static int pfring_netdev_event(struct notifier_block *this,
                               unsigned long          state,
                               void                  *ptr)
{
    struct net_device *dev = ptr;

    PFRING_LOGGER_NOTICE("Netdev_event, %s: dev->name = %s; state = %04lx\n",
                         __func__, (dev) ? dev->name : "NULL", state);


    return NOTIFY_DONE;
}

static struct notifier_block pfring_notifier = {
    .notifier_call = pfring_netdev_event,
};

static void __exit ring_exit(void)
{
    PFRING_LOGGER_NOTICE("Ring exit, pfring module exit\n");

    unregister_netdevice_notifier(&pfring_notifier);

    sock_unregister(PF_RING);
    ring_proc_term();

    PFRING_LOGGER_NOTICE("Ring exit, pfring module exit done\n");
}

static int __init ring_init(void)
{
    int err;

    PFRING_LOGGER_NOTICE("Init, pfring module init\n");
	
    ring_proc_init();
    sock_register(&ring_family_ops);
    
    err = register_netdevice_notifier(&pfring_notifier);
    if (err)
    {
        printk(KERN_ERR "%s: register_netdevice_notifier failed: %d\n", __func__, err);
    }

    fake_rtable_init();

    PFRING_LOGGER_NOTICE("Init, pfring module init done\n");
    return 0;
}

module_init(ring_init);
module_exit(ring_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("exuuwen <vincent7.wen@gmail.com>");
MODULE_DESCRIPTION("Packet capture acceleration by means of a ring buffer");

MODULE_ALIAS_NETPROTO(PF_RING);

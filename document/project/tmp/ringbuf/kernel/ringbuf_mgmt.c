#include <linux/time.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/hardirq.h>
#include <linux/version.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/atomic.h>
#include <linux/list.h>
#include <asm/io.h>

#include "ringbuf_cache.h"
#include "ringbuf_mgmt.h"
#include "ringbuf_ioctl.h"
#include "ringbuf_utils.h"
#include "ringbuf_log.h"
#include "ringbuf_time.h"

#define DEV_NAME "ringbuf"

extern RbTraceLevel rb_current_loglevel;

/*lock*/
struct semaphore rb_mutex = __SEMAPHORE_INITIALIZER(rb_mutex, 1);
#define LOCK_MUTEX if (down_interruptible(&rb_mutex)) return -ERESTARTSYS
#define LOCK_MUTEX_RES down_interruptible(&rb_mutex)
#define UNLOCK_MUTEX up(&rb_mutex)
#define UNLOCK_MUTEX_AND_RETURN(e) {UNLOCK_MUTEX; return e;}

static int allowRbAccess = 0;
spinlock_t rb_kernel_lock = __SPIN_LOCK_UNLOCKED(rb_kernel_lock);

DECLARE_WAIT_QUEUE_HEAD(rb_updated_q);

/*ringbuf device*/
static const char* rb_device_types[RB_DEV_PER_TAG] = {"raw", "simple", "full", "raw_cont", "simple_cont", "full_cont"};
/* Max length of a device name with types defined above */
static const int RB_MAX_DEV_SIZE = RB_MAX_TAG_SIZE + 20;

typedef struct {
  dev_t dev;
  struct class* class; 
} Ringbuf_dev;

static Ringbuf_dev rb_dev;


static ssize_t ringbuf_show(struct device *cd, struct device_attribute *attr, char *buf) 
{
	return sprintf(buf, "%s\n", (char*)cd->platform_data);
}

//NOTE: No " around tag below 
static DEVICE_ATTR(tag, S_IRUGO, ringbuf_show, NULL);


/* This structure defines data specific for every file using the driver */
typedef struct {
  Cache*   p_cache;              /* Cache for copying from kernal to user space,
				    and to read ahead a slice, to get away from
				    the write head (next_write_pos) to avoid
				    driver writes overwrites what is currently
				    read.*/
  Cache*   p_intercept_buf;      /* Buffer used for messages that are inserted
				    before the rest of the normal reading from
				    cache continues. This is used to insert
				    warning/errors, and to translate headers
				    to human readable headers. */
  int     tag;                   /* The tag used for the current file */
  int     last_tag;              /* The current tag for the last raw header
				    read from ringbuf.*/
  Device_type   device_type;     /* The device_type used for the current file */
} Rb_file_specific;


#define ALIGN4(x) (((x)+3)&~3)
#define TRUNC4(x) ((x)&~3)

/*buffer*/
static unsigned long ringbuf_max_size = RB_DEFAULT_RINGBUF_SIZE + sizeof(Ringbuf);

/* Global common buffer used to copy bytes to/from user to kernel space.*/
static char* g_kern_tmp_buf = 0;

/* Here comes the real ringbuf memory. Note that ALL driver accesses to calls
   in this file manipulating this memory must be under lock protection! */
static Ringbuf* g_p_ringbuf = 0;


/*proc file info*/
static struct proc_dir_entry* rb_proc_dir = NULL;
static struct proc_dir_entry* rb_proc_info = NULL;
static struct proc_dir_entry* rb_proc_trace = NULL;

#if(LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
static int rb_proc_get_info(char *buf, char **start, off_t offset,
                  int len, int *unused, void *data)
#else
static ssize_t rb_proc_get_info(struct file *file, char *buf, size_t count, loff_t *offset)
#endif
{
    int rlen = 0;
    int i;

    if (g_p_ringbuf)
    {
        /* /proc/ringbuf/info */
		LOCK_MUTEX;
        rlen  = sprintf(buf,         "RINGBUF Version         : %s\n", RB_VERSION);
        rlen += sprintf(buf + rlen,  "RINGBUF Ident           : %s\n", g_p_ringbuf->ident);
        rlen += sprintf(buf + rlen,  "RINGBUF Bufsize         : %u\n", g_p_ringbuf->bufsize);
		rlen += sprintf(buf + rlen,  "RINGBUF next write pos  : %u\n", g_p_ringbuf->next_write_pos);
		rlen += sprintf(buf + rlen,  "RINGBUF has wrapped     : %d\n", g_p_ringbuf->has_wrapped);
		rlen += sprintf(buf + rlen,  "RINGBUF wrap cnt        : %u\n", g_p_ringbuf->wrap_cnt);
		rlen += sprintf(buf + rlen,  "RINGBUF tag num         : %u\n", g_p_ringbuf->next_free_tag_no);
		rlen += sprintf(buf + rlen,  "RINGBUF cache size      : %u\n", RB_CACHE_SIZE);		
		rlen += sprintf(buf + rlen,  "%s start ------------------------\n", "tag info");
		for(i=0; i<g_p_ringbuf->next_free_tag_no; i++)
			rlen += sprintf(buf + rlen,  "tag %d : %s \n", i, g_p_ringbuf->tag_list[i]);
		rlen += sprintf(buf + rlen,  "%s end   ------------------------\n", "tag info");
		UNLOCK_MUTEX;
    }
    else
    {
        
        rlen  = sprintf(buf, "%s\n", "RINGBUF is uninit");
    }

	buf[rlen++] = 0;

    return rlen;
}

#if(LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
static int rb_proc_trace_read(char *buf, char **start, off_t offset,
                  int len, int *unused, void *data)
#else
static ssize_t rb_proc_trace_read(struct file *file, char *buf, size_t count, loff_t *offset)
#endif
{
    int rlen = 0;
    int i;

    rlen += sprintf(buf + rlen, "Current log level : %s\n", getloglevel_str(rb_current_loglevel));

    rlen += sprintf(buf + rlen, "\n------ Echo Corresponding Number to Change Log Level ------\n");
    for (i = RbError; i < RbDebug3; ++i )
        rlen += sprintf(buf + rlen, "%d %s\n", i, getloglevel_str(i));
    rlen += sprintf(buf + rlen, "------  Example: `echo 2 > /proc/ringbuf/trace`  ------\n");

    return rlen;
}

#if(LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
static int rb_proc_trace_write(struct file *file, const char *buf, unsigned long count, void *data)
#else
static ssize_t rb_proc_trace_write(struct file *file, const char *buf, size_t count, loff_t *offset)
#endif
{
    char * endp;
    unsigned long procfsBufferSize;
    unsigned long newLevel;
	char procTraceBuffer[PROCFS_MAX_SIZE];

    /* get buffer size */
    procfsBufferSize = count;
    if (procfsBufferSize > PROCFS_MAX_SIZE )
    {
        procfsBufferSize = PROCFS_MAX_SIZE;
    }	

    /* write data to the buffer */
    if (copy_from_user(procTraceBuffer, buf, procfsBufferSize))
    {
        return -EFAULT;
    }
	
    newLevel = simple_strtoul(procTraceBuffer, &endp, 0);

    if (newLevel <= RbNone || newLevel > RbDebug3)
        RINGBUF_LOGGER_NOTICE("Trace level unknown: ignored.\n");
    else
        setPfloglevel( (RbTraceLevel)newLevel );

    return procfsBufferSize;
}

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
struct file_operations proc_info = {
  .owner = THIS_MODULE,
  .read = rb_proc_get_info
};

struct file_operations proc_trace = {
  .owner = THIS_MODULE,
  .read = rb_proc_trace_read,
  .write = rb_proc_trace_write
};
#endif

void rb_proc_init(void)
{
	rb_proc_dir = proc_mkdir("ringbuf", NULL);

	if (rb_proc_dir)
	{
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
		rb_proc_dir->owner = THIS_MODULE;
#endif

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
		// create /proc/ringbuf/info
		rb_proc_info = proc_create_data(PROC_INFO, 0444, rb_proc_dir, &proc_info, NULL);
#else
		rb_proc_info = create_proc_read_entry(PROC_INFO, 0, rb_proc_dir, rb_proc_get_info, NULL);
#endif
		if (!rb_proc_info)
		{
			RINGBUF_LOGGER_ERR("proc_init, unable to register proc file\n");
		}
		else
		{
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
			rb_proc_info->owner = THIS_MODULE;
#endif
			RINGBUF_LOGGER_INFO("proc_init, registered /proc/ringbuf/info\n");
		}

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
		// create /proc/ringbuf/info
		rb_proc_trace = proc_create_data(PROC_TRACE, 0644, rb_proc_dir, &proc_trace, NULL);
#else
		// create /proc/ringbuf/trace
		rb_proc_trace = create_proc_entry(PROC_TRACE, 0644, rb_proc_dir);
#endif
		if (!rb_proc_trace)
		{
			RINGBUF_LOGGER_ERR("Unable to register proc trace control file\n");
		}
		else
		{
#if(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
			rb_proc_info->owner = THIS_MODULE;
#endif

#if(LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
			rb_proc_trace->write_proc  = rb_proc_trace_write;
			rb_proc_trace->read_proc = rb_proc_trace_read;
#endif

			RINGBUF_LOGGER_INFO("Registered /proc/ringbuf/trace\n");
		}
	}
	else
	{
		RINGBUF_LOGGER_ERR("Unable to create /proc/ringbuf\n");
	}
}

void rb_proc_exit(void)
{
	if (rb_proc_info != NULL)
	{
		remove_proc_entry(PROC_INFO, rb_proc_dir);
		RINGBUF_LOGGER_INFO("proc_exit, removed /proc/ringbuf/%s\n", PROC_INFO);
	}

	if (rb_proc_trace != NULL)
	{
		remove_proc_entry(PROC_TRACE, rb_proc_dir);
		RINGBUF_LOGGER_INFO("proc_exit, remove /proc/ringbuf/%s\n", PROC_TRACE);
	}

	if (rb_proc_dir != NULL)
	{
		remove_proc_entry("ringbuf", NULL);
		RINGBUF_LOGGER_INFO("proc_exit, remove /proc/ringbuf\n");
	}
}

dev_t rb_get_dev_t(void)
{
	return rb_dev.dev;
}

static void* kzalloc_rb(size_t size)
{
	void *ptr;

	ptr = vmalloc(size);
	if (NULL == ptr) 
	{
		RINGBUF_LOGGER_ERR("Error, cannot allocate %lu bytes in memory\n", (unsigned long)size);
		return NULL;
	}
	
	memset(ptr, 0, size);

  	return ptr;
}


static void allow_rb_access(void)
{
	spin_lock(&rb_kernel_lock);
	allowRbAccess=1;
	spin_unlock(&rb_kernel_lock);
}

static void deny_rb_access(void)
{
	spin_lock(&rb_kernel_lock);
	allowRbAccess=0;
	spin_unlock(&rb_kernel_lock);
}

static void init_rb(Ringbuf* p_rb, uint32_t bufsize)
{
	uint32_t i;

	p_rb->bufsize = bufsize;
	strncpy(p_rb->ident, RB_IDENT_STRING, RB_IDENT_MAX_STRING_LEN);
	p_rb->next_write_pos=0;
	p_rb->wrap_cnt=0;
	p_rb->has_wrapped=0;
	p_rb->last_header_pos=0;
	p_rb->next_free_tag_no=0;
	p_rb->buf = &(p_rb->buf_data_begin);
	/* "U" means never used chars */
	for (i=1; i<bufsize; i++) 
		p_rb->buf[i]='U';
}

/* Only printable ASCCI charecters are supporter as tag names */
static int is_valid_tag(const char* tag)
{
	int len = strlen(tag);
	int c;

	if (len > RB_MAX_TAG_SIZE) 
	{
		RINGBUF_LOGGER_DEBUG("Tag name %s is too long\n", tag);
		return -EINVAL;
	}

	for (c=0; c<len; c++) 
	{
		if (!isascii(tag[c])) 
		{
	  		RINGBUF_LOGGER_DEBUG("Tag name %s contains non ASCII char %c", tag, tag[c]);
	  		return -EINVAL;
		}
		if (!isprint(tag[c])) 
		{
	  		RINGBUF_LOGGER_DEBUG("Tag name %s contains non printable char %x", tag, tag[c]);
	  		return -EINVAL;
		}
	}

	return 0;
}


static int find_tag(const char * tag)
{
	int i;

	for (i=0; i<g_p_ringbuf->next_free_tag_no; ++i) 
	{
		if ((strncmp(tag, g_p_ringbuf->tag_list[i], RB_MAX_TAG_SIZE)) == 0)
	  		return i;
	}

	return -1;
}

static int set_tag(const char* tag)
{
	int free_tag_no;
	int tag_no;

	if (0 > is_valid_tag(tag)) return -EINVAL;

	/* Is tag already in list ?*/
	if ((tag_no = find_tag(tag)) != -1)
		return tag_no;

	free_tag_no = g_p_ringbuf->next_free_tag_no;

	if (free_tag_no < RB_MAX_NO_TAGS) 
	{
		strncpy(g_p_ringbuf->tag_list[free_tag_no], tag, RB_MAX_TAG_SIZE);
		g_p_ringbuf->next_free_tag_no++;
		RINGBUF_LOGGER_INFO("Got tag number %d for tag %s\n", free_tag_no, tag);
		return free_tag_no;
	}

	RINGBUF_LOGGER_ERR("Number of tags (%d) exceeded RB_MAX_NO_TAGS (%d)\n", free_tag_no, RB_MAX_NO_TAGS);

	return -EINVAL;
}


/* Small utillity function that creates devices in our class.
 * udev uses this for dynamicly create devices.
 */
static int create_devices(const char* tag_name) 
{
	int i, res, tag_no, minor_base;
	struct device *d;

	/* Get major device number if not already allocated to me */
	if (0 == MAJOR(rb_dev.dev)) 
	{
		res = alloc_chrdev_region(&(rb_dev.dev), 0, RB_MAX_NO_TAGS * RB_DEV_PER_TAG, DEV_NAME);
		if (0 != res) 
		{
			RINGBUF_LOGGER_ERR("create_devices(): Failed to alloc major device number\n");
			rb_dev.dev = MKDEV(0, 0);
			return res;
		}
	}

	/* Create a class for use by udev if not already created */
	if (NULL == rb_dev.class) 
	{
		rb_dev.class = class_create(THIS_MODULE, DEV_NAME);
	
		if (IS_ERR(rb_dev.class)) 
		{
			RINGBUF_LOGGER_ERR("Failed to create class %s\n", DEV_NAME);
			rb_dev.class = NULL;
			return PTR_ERR(rb_dev.class);
		}
	}

	tag_no = set_tag(tag_name);
	if (0 > tag_no) 
		return -EINVAL;

	minor_base = tag_no * RB_DEV_PER_TAG;
	for (i=0; i<RB_DEV_PER_TAG; i++) 
	{
		d = device_create(rb_dev.class, NULL, MKDEV(MAJOR(rb_dev.dev), minor_base + i), NULL, "rb_%d_%s", tag_no, rb_device_types[i]);
	
		if (IS_ERR(d)) 
		{
			RINGBUF_LOGGER_ERR("Failed to create class device\n");
			return PTR_ERR(d);
		}

		/* Create the basename of the device */
		d->platform_data = kzalloc_rb(RB_MAX_DEV_SIZE);
		if (NULL == d->platform_data) 
			return -ENOMEM;
		snprintf(d->platform_data, RB_MAX_DEV_SIZE, "%s_%s", tag_name, rb_device_types[i]);

		res = device_create_file(d, &dev_attr_tag);
	
		if (0 != res) 
		{
			printk(KERN_ERR "Failed to do class_device_create_file\n");
			return res;
		}
	}

	return 0;
}


static int __match_devt(struct device *dev, void *data)
{
	dev_t *devt = (dev_t *)data;

	return dev->devt == *devt;
}

static void rb_device_destroy(struct class *class, dev_t devt)
{
	struct device *dev;

	dev = class_find_device(class, NULL, &devt, __match_devt);
	if (dev) 
	{
		device_remove_file(dev, &dev_attr_tag);
		vfree(dev->platform_data);
		put_device(dev);
		device_unregister(dev);
	}
}


static void remove_devices(void)
{
	int tag_no, i, minor_base;

	for (tag_no=0; tag_no<g_p_ringbuf->next_free_tag_no; ++tag_no) 
	{
		minor_base = tag_no * RB_DEV_PER_TAG;
		for (i=0; i<RB_DEV_PER_TAG; i++) 
		{
			//device_destroy(rb_dev.class, MKDEV(MAJOR(rb_dev.dev), minor_base + i));
			rb_device_destroy(rb_dev.class, MKDEV(MAJOR(rb_dev.dev), minor_base + i));
		}
	}

	if (NULL != rb_dev.class) 
	{
		class_destroy(rb_dev.class);
		rb_dev.class = NULL;
	}


	if (0 != MAJOR(rb_dev.dev)) 
	{
		unregister_chrdev_region(rb_dev.dev, RB_MAX_NO_TAGS * RB_DEV_PER_TAG);
		rb_dev.dev = MKDEV(0, 0);
	}
}


int rb_init()
{
	int res;

	LOCK_MUTEX;
	init_waitqueue_head(&rb_updated_q);

	g_kern_tmp_buf = kzalloc_rb(RB_KERN_TMP_BUF_SIZE);
	if (NULL == g_kern_tmp_buf) 
		UNLOCK_MUTEX_AND_RETURN(-ENOMEM);

	g_p_ringbuf = kzalloc_rb(ringbuf_max_size);
	if (NULL == g_p_ringbuf) 
		UNLOCK_MUTEX_AND_RETURN(-ENOMEM);

	RINGBUF_LOGGER_INFO("Ringbuf at memory position %p\n",  g_p_ringbuf);

	init_rb(g_p_ringbuf, RB_DEFAULT_RINGBUF_SIZE);
	res = create_devices("all");
	if (0 != res)
		RINGBUF_LOGGER_ERR("Failed to call create_devices with tag all\n");
		
	res = rt_create_table();
	if (0 != res) 
	{
		RINGBUF_LOGGER_ERR("Failed to call rt_create_table\n");
		UNLOCK_MUTEX_AND_RETURN(res);
	}

	allow_rb_access();

	UNLOCK_MUTEX_AND_RETURN(0);
}

/* Undoing stuff from rb_init in reversed order */
void rb_stop(void)
{
	/* Wait for rb_mutex to be aquired */
	while(LOCK_MUTEX_RES == -EINTR);
	
	deny_rb_access();
	
	remove_devices();

	rt_remove_table();

	if (NULL != g_p_ringbuf) 
	{
		RINGBUF_LOGGER_INFO("free g_p_ringbuf %p\n", g_p_ringbuf);
		vfree(g_p_ringbuf);
		g_p_ringbuf = NULL;
	}

	if (NULL != g_kern_tmp_buf) 
	{
		RINGBUF_LOGGER_INFO("free g_kern_tmp_ringbuf %p\n", g_kern_tmp_buf);
		vfree(g_kern_tmp_buf);
		g_kern_tmp_buf = NULL;
	}

	UNLOCK_MUTEX;
}

/*file operations*/

char* rb_get_tag(int tag_no)
{
	int free_tag_no = g_p_ringbuf->next_free_tag_no;
	if (free_tag_no > RB_MAX_TAG_SIZE) 
		free_tag_no = RB_MAX_TAG_SIZE;

	if (tag_no >= 0 && tag_no < free_tag_no)
		return g_p_ringbuf->tag_list[tag_no];
	else
		return "undefined_tag";
}

static void put_header_and_tags_in_icept_buf(struct file *p_file)
{
	char str[RB_MAX_TAG_SIZE + 80];
	int i;

	Rb_file_specific* p_rb_file_spec = p_file->private_data;
	Cache* p_icept_buf = p_rb_file_spec->p_intercept_buf;
	int free_tag_no = g_p_ringbuf->next_free_tag_no;
	if (free_tag_no > RB_MAX_NO_TAGS) 
		free_tag_no = RB_MAX_NO_TAGS;

	snprintf(str,sizeof(str), "%s\n", RB_VERSION);
	add_to_cache(p_icept_buf, str, strlen(str));

	snprintf(str, sizeof(str), "Dump of %d tags\n", free_tag_no);
	add_to_cache(p_icept_buf, str, strlen(str));

	snprintf(str, sizeof(str), "HEX String\n");
	add_to_cache(p_icept_buf, str, strlen(str));

	snprintf(str, sizeof(str), "-----------------------------------------------------------------\n");
	add_to_cache(p_icept_buf, str, strlen(str));

	for (i=0; i<free_tag_no; i++) 
	{
		snprintf(str, sizeof(str), " %02X %s\n", i, rb_get_tag(i));
		add_to_cache(p_icept_buf, str, strlen(str));
	}
}

static uint32_t written_chars_in_rb(Ringbuf* p_rb)
{
	if (p_rb->has_wrapped)
		return p_rb->bufsize;
	else
		return p_rb->next_write_pos;
}

static uint32_t oldest_position_in_rb(Ringbuf* p_rb)
{
	if (p_rb->has_wrapped)
		return p_rb->next_write_pos;
	else
		return 0;
}

static uint32_t inc_rb_pos(Ringbuf* p_rb, uint32_t index)
{
	if (index == p_rb->bufsize - 1)
		return 0;
	else if (index >= p_rb->bufsize)
		return oldest_position_in_rb(p_rb);
	else
		return index + 1;
}

static uint32_t dec_rb_pos(Ringbuf* p_rb,uint32_t index)
{
	if (index >= p_rb->bufsize)
		return dec_rb_pos(p_rb, oldest_position_in_rb(p_rb));
	else if (index == 0)
		return p_rb->bufsize - 1;
	else
		return index - 1;
}

static uint32_t get_str_from_rb(Cache* p_cache, char str[], uint32_t num_chars, void* p_ringbuf)
{
	uint32_t total_copied = 0;
	uint32_t index;
	Ringbuf* p_rb = (Ringbuf*) p_ringbuf;
	
	spin_lock(&rb_kernel_lock);

	if(!allowRbAccess) 
	{
		spin_unlock(&rb_kernel_lock);
		return 0;
	}

	if (written_chars_in_rb(p_rb) == 0) 
	{
		spin_unlock(&rb_kernel_lock);
		return 0;
	}

	if (!p_cache->valid_cached_position) 
	{
		p_cache->last_cached_rb_wrap_cnt = p_rb->wrap_cnt;
		index = oldest_position_in_rb(p_rb);
	}
	else 
	{
		index = inc_rb_pos(p_rb, p_cache->last_cached_rb_pos);
	}

	if (p_cache->valid_cached_position && (p_cache->last_cached_rb_pos == dec_rb_pos(p_rb,oldest_position_in_rb(p_rb)))) 
	{
    		spin_unlock(&rb_kernel_lock);
    		return 0; /* Nothing more to read, we have already cached the
                 newest char in ringbuf */
	}

	if (num_chars > p_rb->bufsize) 
		num_chars = p_rb->bufsize;

	while (total_copied < num_chars) 
	{
		if (index >= written_chars_in_rb(p_rb)) 
			break;
		str[total_copied++] = p_rb->buf[index];
		p_cache->last_cached_rb_pos = index;
		p_cache->valid_cached_position = 1;
		index = inc_rb_pos(p_rb, index);
		/* Wrap around in rb during read? */
		if (p_cache->last_cached_rb_pos > index) 
			p_cache->last_cached_rb_wrap_cnt++;

		/* Check if we have read everything in ringbuf, if so, exit*/
		if (index == oldest_position_in_rb(p_rb)) 
		  break;
	}

	spin_unlock(&rb_kernel_lock);

	return total_copied;
}



int rb_open(struct file *p_file, int tag, Device_type device_type) 
{
	Rb_file_specific* p_rb_file_spec;

	p_rb_file_spec = kzalloc_rb(sizeof(Rb_file_specific));
	if (NULL == p_rb_file_spec) 
		return -ENOMEM;

	p_file->private_data = p_rb_file_spec;

	p_rb_file_spec->tag = tag;
	p_rb_file_spec->device_type = device_type;
	p_rb_file_spec->last_tag = -1; /* Illegal, not set yet */

	p_rb_file_spec->p_intercept_buf = kzalloc_rb(sizeof(Cache));
	if (NULL == p_rb_file_spec->p_intercept_buf) 
		return -ENOMEM;

	p_rb_file_spec->p_cache = kzalloc_rb(sizeof(Cache));
	if (NULL == p_rb_file_spec->p_cache) 
		return -ENOMEM;

	init_cache(p_rb_file_spec->p_intercept_buf);
	init_cache(p_rb_file_spec->p_cache);

	LOCK_MUTEX;
	/* If this is a raw "all" device, ensure the header and tag_dump
	 comes first out when reading by putting it in the intercept buffer */
	if (is_raw(device_type) && tag == 0) 
		put_header_and_tags_in_icept_buf(p_file);

	/* Initiate and fill the read cache, to prevent writing to
	 catch up on reading */
	fill_cache(p_rb_file_spec->p_cache, 0, g_p_ringbuf, get_str_from_rb);
	
	UNLOCK_MUTEX_AND_RETURN(0);
}

int rb_release(struct file* p_file) 
{
	Rb_file_specific* p_rb_file_spec = p_file->private_data;
	
	if (p_rb_file_spec) 
	{
		if (p_rb_file_spec->p_cache)
			vfree(p_rb_file_spec->p_cache);
		if (p_rb_file_spec->p_intercept_buf)
			vfree(p_rb_file_spec->p_intercept_buf);
		vfree(p_rb_file_spec);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

static int data_is_overwritten(Ringbuf* p_rb, Cache* p_cache)
{
	int overwritten;

	if (!p_cache->valid_cached_position) 
		return 0;

	if (p_rb->next_write_pos > p_cache->last_cached_rb_pos) 
	{
		if (p_rb->has_wrapped == 0)
			overwritten = 0;
		else if (p_rb->wrap_cnt > p_cache->last_cached_rb_wrap_cnt ||
		   (p_rb->wrap_cnt == 0 && p_cache->last_cached_rb_wrap_cnt == 0xffffffff))
			overwritten = 1;
		else
			overwritten = 0;
	}
	else 
	{
		if (p_rb->wrap_cnt > p_cache->last_cached_rb_wrap_cnt + 1 ||
		   (p_rb->wrap_cnt == 0 && p_cache->last_cached_rb_wrap_cnt == 0xffffffff)) 
			overwritten = 1;
		else
			overwritten = 0;
	}

	if (overwritten)
		RINGBUF_LOGGER_INFO("Read position overwritten by writer in ringbuf\n");

	return overwritten;
}

/* Dito but for the decremented position */
static uint32_t wrap_counter_for_prev_rb_pos(uint32_t rb_pos,
                                             uint32_t wrap_cnt)
{
	if (rb_pos == 0)
		return wrap_cnt - 1;
	else
		return wrap_cnt;
}

/* Both decrement position and wrap count correctly */
static void dec_rb_pos_and_wrap_cnt(uint32_t* rb_pos, uint32_t* wrap_cnt)
{
	*wrap_cnt = wrap_counter_for_prev_rb_pos(*rb_pos, *wrap_cnt);
	*rb_pos = dec_rb_pos(g_p_ringbuf, *rb_pos);
}

static void sync_cache_and_rb(Ringbuf* p_rb,Cache* p_cache)
{
	p_cache->last_cached_rb_pos = oldest_position_in_rb(p_rb);
	p_cache->last_cached_rb_wrap_cnt = p_rb->wrap_cnt;

	dec_rb_pos_and_wrap_cnt(&p_cache->last_cached_rb_pos, &p_cache->last_cached_rb_wrap_cnt);
}

static int check_overwritten_reader_and_sync(Ringbuf* p_rb, Cache* p_cache)
{
	if (data_is_overwritten(p_rb, p_cache)) 
	{
		RINGBUF_LOGGER_ERR( 
			   "Warning, writes to ringbuf has written over the data to read "
			   "try to read faster to avoid this\n");
		RINGBUF_LOGGER_INFO(
			   "Continuing at last known oldest data\n");
		sync_cache_and_rb(p_rb, p_cache);
		return 1;
	}
	else
		/* No lost data */
		return 0;
}

static void refill_cache_check_overwrite(Ringbuf* p_rb, Cache* p_cache, Cache* p_icept_buf)
{
	char overwrite_header[100];

	if (cached_chars(p_icept_buf) == 0) 
	{
		int overwritten = check_overwritten_reader_and_sync(p_rb,p_cache);
		fill_cache(p_cache, 0, p_rb, get_str_from_rb);
		if (overwritten) 
		{
			snprintf(overwrite_header, RB_CACHE_SIZE, "%s[%s]", RB_UNIQUE_RAW_HEADER_HEAD,OVERWRITE_MSG);
			add_to_cache(p_icept_buf, overwrite_header, strlen(overwrite_header));
		}
	}    
}

static int has_cont_dev(struct file* p_file)
{
	Rb_file_specific* p_rb_file_spec = p_file->private_data;
	return (p_rb_file_spec->device_type == raw_cont ||
		  p_rb_file_spec->device_type == simple_cont ||
		  p_rb_file_spec->device_type == full_cont);
}

static uint32_t wrap_counter_for_next_rb_pos(Ringbuf* p_rb, uint32_t rb_pos, uint32_t wrap_cnt)
{
	if ((rb_pos + 1) >= p_rb->bufsize)
		return wrap_cnt + 1;
	else
		return wrap_cnt;
}

static int data_is_unavaliable(Ringbuf* p_rb,struct file* p_file)
{
	Rb_file_specific* p_rb_file_spec = p_file->private_data;
	Cache* p_cache = p_rb_file_spec->p_cache;

	/* If we have never written anything to ringbuf and cache has never
	 been used data is unavailable.
	*/
	if (!p_cache->valid_cached_position) 
	{
		if (written_chars_in_rb(p_rb) == 0)
			return 1;
		else
			return 0;
	}
	else if (p_rb->next_write_pos == inc_rb_pos(p_rb, p_cache->last_cached_rb_pos) && 
	 (p_rb->wrap_cnt <= wrap_counter_for_next_rb_pos(p_rb, p_cache->last_cached_rb_pos, p_cache->last_cached_rb_wrap_cnt) 
	   ||
	   (
	 /* Wrap maxint will occur after RB_DEFAULT_RINGBUF_SIZE*4GByte bytes
		has been written. If RB_DEFAULT_RINGBUF_SIZE is 1 MByte, that will
		occur when 4 TByte has been written to ringbuf. If 100 lines/second
		is written, that will occur after 317 years... */
	 p_rb->wrap_cnt == 0xffffffff && 
	 0 == wrap_counter_for_next_rb_pos(p_rb, p_cache->last_cached_rb_pos,
					   p_cache->last_cached_rb_wrap_cnt))
	   )
	  )

	/* If the next position we would cache is the next position to write
	   in ringbuf, data is unavailable since we stop caching when there
	   is no more data in the ringbuf */
		return 1;
	else
		return 0;
}

static int wait_for_rb_update(Ringbuf* p_rb, struct file* p_file)
{
	UNLOCK_MUTEX;

	if ((p_file->f_flags & O_NONBLOCK)) 
		return -EAGAIN;
	
	RINGBUF_LOGGER_INFO("Ringbuf Reading going to sleep\n");
	
	if (wait_event_interruptible(rb_updated_q, !data_is_unavaliable(p_rb,p_file))) 
		return -ERESTARTSYS;

	LOCK_MUTEX;

	return 0;
}

static uint32_t caching_get_str_from_rb(Ringbuf* p_rb,struct file* p_file,
                                        char str[], uint32_t num_chars,
                                        int* err_code)
{
	Rb_file_specific* p_rb_file_spec = p_file->private_data;
	Cache* p_cache = p_rb_file_spec->p_cache;
	Cache* p_icept_buf = p_rb_file_spec->p_intercept_buf;
	uint32_t left_to_read = num_chars, total_read = 0, act_read = 0;
	char skip_str[1];
	int found_header = 0;
	*err_code = 0;
	

	while(left_to_read) 
	{
		act_read = 0;
		refill_cache_check_overwrite(p_rb, p_cache, p_icept_buf);
		if (cached_chars(p_icept_buf) > 0)
			act_read = get_from_cache(p_icept_buf, &(str[total_read]), left_to_read);
		else if (is_raw(p_rb_file_spec->device_type) && p_rb_file_spec->tag == 0) 
		{
			/*	
			The file is a raw device for the "all" tags. No translation at all
			of header is needed. Read as much as possible to get fast ringbuf
			raw dumps!
			*/
			act_read = get_from_cache(p_cache, &(str[total_read]), left_to_read);
		}
		else 
		{
			/* The file type says that we should translate the header */
			found_header = translate_if_raw_header_first(p_cache, p_icept_buf,
						  rt_get_time, rb_get_tag,
						  p_rb_file_spec->device_type,
						  &p_rb_file_spec->last_tag);

			if (p_rb_file_spec->tag != p_rb_file_spec->last_tag && p_rb_file_spec->tag != 0)
			{
				/* Another tag than the tag for the specific device file is ahead in
				ringbuf,and we are NOT looking at the an "all"-type device file (0).
				If a header was translated then first through away that header from
				the intercept cache. */
				if (found_header) 
					empty_cache(p_icept_buf);         
				/* If there is non-header character ahead skip the first next
				character, since it belongs to the previous header above that has a
				non-matching tag. */
				if (!examine_raw_header_if_first_in_cache(p_cache, 0, 0))
					(void) get_from_cache(p_cache,skip_str, 1);
			}
			else 
			{
				if (cached_chars(p_icept_buf) > 0)
					act_read = get_from_cache(p_icept_buf,  &(str[total_read]), left_to_read);
				else
				{
					/* Only read the characters one by one here, to be able to look for
					header first in cache above */
					act_read = get_from_cache(p_cache, &(str[total_read]), 1);
				}
			}
		}

		total_read += act_read;
		left_to_read -= act_read;

		refill_cache_check_overwrite(p_rb, p_cache, p_icept_buf);
		if (cached_chars(p_cache) == 0 && cached_chars(p_icept_buf) == 0) 
		{
			/* If this is a piping device, wait for new data to arrive. */
			if (has_cont_dev(p_file) && total_read == 0) 
			{
				*err_code = wait_for_rb_update(p_rb, p_file);
				if (*err_code) 
					break;
			} 
			else 
			{
				*err_code=0;
				break; /*End of file, no more data*/
			}
		}
	}

	return total_read;
}

int rb_read(struct file* p_file, char* buf, size_t count, rb_off_t* offset) 
{
	size_t num_read, num_to_read, total_read,i;
	int err_code;
	char* bufp;

	if (count == 0) 
		return 0;

	LOCK_MUTEX;

	bufp = buf;
	total_read = 0;
	num_to_read = (count < RB_KERN_TMP_BUF_SIZE) ? count : RB_KERN_TMP_BUF_SIZE;

	for (i=0; i<count/RB_KERN_TMP_BUF_SIZE; i++)
	{
		num_read = caching_get_str_from_rb(g_p_ringbuf, p_file, g_kern_tmp_buf, num_to_read, &err_code);
		*offset += num_read;
		total_read += num_read;

		if (err_code) 
			UNLOCK_MUTEX_AND_RETURN(err_code);
		if (copy_to_user(bufp, g_kern_tmp_buf, num_read))
			UNLOCK_MUTEX_AND_RETURN(-EFAULT);

		bufp += num_read;
		if (num_read != num_to_read) 
		{
			RINGBUF_LOGGER_INFO("rb_read returns %lu bytes, offset %lu\n", total_read, (unsigned long) *offset);
			UNLOCK_MUTEX_AND_RETURN(total_read);
		}
	}

	num_to_read = count % RB_KERN_TMP_BUF_SIZE;
	if (num_to_read > 0)
	{
		num_read = caching_get_str_from_rb(g_p_ringbuf, p_file, g_kern_tmp_buf, num_to_read, &err_code);
		*offset += num_read;
		total_read += num_read;

		if (err_code) 
			UNLOCK_MUTEX_AND_RETURN(err_code);
		if (copy_to_user(bufp, g_kern_tmp_buf, num_read))
			UNLOCK_MUTEX_AND_RETURN(-EFAULT);

		bufp += num_read;
	}

	RINGBUF_LOGGER_DEBUG3("rb_read returns %lu bytes, offset %lu\n", total_read, (unsigned long) *offset);

	UNLOCK_MUTEX_AND_RETURN(total_read);
}


///////////////////////////////////////////////////////////////////

static uint32_t add_char_to_rb(Ringbuf* p_rb, char c, int check_header_overwrite)
{
	/* Do not write behind the header for the string we are actally writing, 
	 i.e. do not overwrite our own header. */
	if (check_header_overwrite && p_rb->next_write_pos == p_rb->last_header_pos ) 
	{
		RINGBUF_LOGGER_INFO("About to overwrite the last written header, probably "
			   "because either the previous or the current message is very large. "
			   "Making a new header replacing the old header. \n");
		return 0;
	}

	p_rb->buf[p_rb->next_write_pos] = c;  
	p_rb->next_write_pos++;
	if (p_rb->next_write_pos >= p_rb->bufsize) 
	{
	/* To avoid be fooled to think the ringbuf is empty if computer is
	   reset just before setting next_write_pos to 0 below, it is very
	   important to first set has_wrapped, then increase wrap_cnt and
	   thereafter reset next_write_pos. */
		p_rb->has_wrapped = 1;
		p_rb->wrap_cnt++;
		p_rb->next_write_pos = 0;
	}

	return 1;
}

/* Write a string to ringbuf, just "as-is", not adding header etc */
static uint32_t write_str_to_rb(Ringbuf* p_rb,const char str[],
                                uint32_t num_chars,int check_header_overwrite)
{
	uint32_t i, act_chars = 0, str_pos = 0, total_written = 0;

	if (num_chars == 0) 
		return 0;

	for(i=0; i<num_chars; i++) 
	{
		act_chars = add_char_to_rb(p_rb, str[str_pos], check_header_overwrite);
		if (act_chars == 0) 
		{
			return total_written;
		}
		else 
		{
	  		total_written++;
	  		str_pos++;
		}
	}

	return total_written;
}

/* 
   Write the header to ringbuf. Returns number of bytes
   written to rb. If really_write_to_rb equals 0, nothing is written to rb
   only the size is calculated and returned.

   No header is written if the last header with the same tag was written
   for less than one second ago!
*/
static uint32_t write_header_to_rb(Ringbuf* p_rb, const int tag)
{
	static struct timespec time4last_header;
	static int last_tag = -1;
	static int time4last_header_valid = 0;
	uint32_t total_written = 0;
	char header_start[RB_RAW_HEADER_SIZE + 1]; /* Room for null char */
	struct timespec time_now;
	uint32_t header_pos;

	time_now = current_kernel_time();
	if (!time4last_header_valid) 
		last_tag = -1;
	/* If less than one second since last header with the same tag was written,
	 do not write any new header, unless we are just about to 
	 overwrite the last written header with a new one. Overwriting the last
	 written header with a new one occurs, when writing something greater than
	 the ringbuf, beause the writes to ringbuf stops/aborts just before the
	 wrap-around to avoid destoying the header for the write itself. 
	 In that case, the driver writes the last skipped bytes again. */
	if (time4last_header_valid) 
	{
		if (p_rb->last_header_pos!=p_rb->next_write_pos) 
		{
			if (last_tag == tag) 
			{
				if ((time_now.tv_sec == time4last_header.tv_sec) ||
					((time_now.tv_sec == (time4last_header.tv_sec + RB_HEADER_COLLECT_TIME)) &&
					 (time_now.tv_nsec <= time4last_header.tv_nsec))) 
				return 0;
			}
		}
	}
	snprintf(header_start, sizeof(header_start), "%s[%08X %02X]", 
		   RB_UNIQUE_RAW_HEADER_HEAD, (uint32_t)time_now.tv_sec, tag);
	
	header_pos = p_rb->next_write_pos;
	/* Note the last parameter to write_str_to_rb, that prevents checking
	   header overwrite here */
	total_written = write_str_to_rb(p_rb, header_start, strlen(header_start), 0);
	/* Remember the start of header above to protect from overwrites */
	p_rb->last_header_pos = header_pos; 
	time4last_header.tv_sec = time_now.tv_sec;
	time4last_header.tv_nsec = time_now.tv_nsec;
	last_tag = tag;
	time4last_header_valid = 1;

	return total_written;
	
}

static uint32_t add_item_to_rb(Ringbuf* p_rb,const char str[],
                               uint32_t num_chars, const int tag,
                               Rb_do_header_param do_header)
{
	uint32_t act_written = 0, header_size = 0;

	/* Now write the header to rb. Do NOT count the header size in the amount
	 returned! */
	if (do_header == Rb_write_header)
		header_size = write_header_to_rb(p_rb, tag);

	act_written = write_str_to_rb(p_rb, str, num_chars, 1);

	return act_written;
}

static void add_kspace_chunk_to_rb(Ringbuf* p_rb, const char* chunk,
                                   const int tag, size_t num_to_write,
                                   Rb_do_header_param* write_header, int panic_mode)
{
	size_t act_written, tot_chunk_written;
	int release_lock = 1;
	unsigned long flags = 0; /* Initialized just to keep compiler happy */

	if(unlikely(panic_mode)) 
	{
		/* If we are called during panic we should print even if we 
		 * do not get the lock. */
		if(!spin_trylock_irqsave(&rb_kernel_lock, flags)) 
		{
			release_lock = 0;
			local_irq_save(flags);
		}
	} 
	else 
	{
		spin_lock(&rb_kernel_lock);
		if(!allowRbAccess) 
		{
	  		spin_unlock(&rb_kernel_lock);
	  		return;
		}
	}
	/* Loop to handle if writing more than buffer space, to avoid
	 overwriting the header when wrapping around */
	act_written = 0;
	tot_chunk_written = 0;
	while (tot_chunk_written != num_to_write) 
	{
		act_written = add_item_to_rb(p_rb, &(chunk[tot_chunk_written]), num_to_write-tot_chunk_written, tag, *write_header);
		if (act_written != num_to_write)
		{
			/* Failed to write the original required size, that means that
			 we has reached our own header. Therefore ensure to write a 
			 new header the next call to add_item_to_rb*/
			*write_header = Rb_write_header;
		}
		tot_chunk_written += act_written;
	}

	if(unlikely(panic_mode)) 
	{
		if(release_lock)
			spin_unlock_irqrestore(&rb_kernel_lock, flags);
		else
			local_irq_restore(flags);
	} 
	else
		spin_unlock(&rb_kernel_lock);
}

static void signal_rb_updated(void)
{
	wake_up_interruptible(&rb_updated_q);
}

int rb_write( struct file* p_file, const char* buf, size_t count, 
              rb_off_t* offset)
{
	size_t num_to_write, total_written = 0, i;
	const char* bufp = buf;
	Rb_do_header_param write_header = Rb_write_header;
	Rb_file_specific* p_rb_file_spec = p_file->private_data;

	if (count == 0) 
		return 0;

	LOCK_MUTEX;

	num_to_write = (count < RB_KERN_TMP_BUF_SIZE) ? count : RB_KERN_TMP_BUF_SIZE;
	for (i=0; i<count/RB_KERN_TMP_BUF_SIZE; i++)
	{
		if (copy_from_user(g_kern_tmp_buf, bufp, num_to_write))
			UNLOCK_MUTEX_AND_RETURN(-EFAULT);
		add_kspace_chunk_to_rb(g_p_ringbuf, g_kern_tmp_buf, p_rb_file_spec->tag, num_to_write, &write_header, 0);
		write_header = Rb_no_header;
		bufp += num_to_write;
		*offset += num_to_write;
		total_written += num_to_write;
	}

	num_to_write = count % RB_KERN_TMP_BUF_SIZE;
	if (num_to_write > 0) 
	{
		if (copy_from_user(g_kern_tmp_buf,bufp, num_to_write))
			UNLOCK_MUTEX_AND_RETURN(-EFAULT);
		add_kspace_chunk_to_rb(g_p_ringbuf, g_kern_tmp_buf, p_rb_file_spec->tag, num_to_write, &write_header, 0);
		write_header = Rb_no_header;
		bufp += num_to_write;
		*offset += num_to_write;
		total_written += num_to_write;
	}

	signal_rb_updated();

	UNLOCK_MUTEX;

	return total_written;
}

////////////////////////////////////////////////////////////////////

static char  tag_list_copy[RB_MAX_NO_TAGS][RB_MAX_TAG_SIZE + 1];
static int next_free_tag_no_copy;

static void save_tags_list(void)
{
	int i,j;
	next_free_tag_no_copy = g_p_ringbuf->next_free_tag_no;
	/* Copy all tags */
	for (i=0; i<next_free_tag_no_copy; i++)
		for (j=0; j<RB_MAX_TAG_SIZE; j++)
	  		tag_list_copy[i][j] = g_p_ringbuf->tag_list[i][j];
}

static void restore_tag_list(void)
{
	int i,j;
	g_p_ringbuf->next_free_tag_no = next_free_tag_no_copy;
	/* Copy all tags back */
	for (i=0; i<next_free_tag_no_copy; i++)
		for (j=0; j<RB_MAX_TAG_SIZE; j++)
	  		g_p_ringbuf->tag_list[i][j] = tag_list_copy[i][j];
}

static int tag_exist(const char tag[])
{
	int i;

	for (i=0; i<g_p_ringbuf->next_free_tag_no; i++)
		if (strncmp(tag, g_p_ringbuf->tag_list[i], RB_MAX_TAG_SIZE) == 0)
	  		return i + 1;

	return 0;
}

static int set_ringbuf_size(uint32_t size)
{
	if (size == g_p_ringbuf->bufsize) 
	{
		RINGBUF_LOGGER_INFO("Ringbuf size is already %u bytes\n", size);
		return 0;
	}

	if ((RB_MIN_RINGBUF_SIZE <= size) && ((ringbuf_max_size - sizeof(Ringbuf)) >= size)) 
	{
		RINGBUF_LOGGER_INFO("Changing ringbuf size from %u to %u\n", g_p_ringbuf->bufsize, size);
		save_tags_list();
		init_rb(g_p_ringbuf, size);
		restore_tag_list();
		return 0;
	} 
	else
	{
		RINGBUF_LOGGER_ERR("Wrong size of argument to set_ringbuf_size, %u\n", size);
		return -EINVAL;
	}
}


static void set_ringbuf_pos_rel_end(struct file *p_file, uint32_t chars_from_end)
{
	Rb_file_specific* p_rb_file_spec = p_file->private_data;
	Cache* p_cache = p_rb_file_spec->p_cache;

	init_cache(p_cache); /* Discard data filled at open, and reset wrap cnt */

	if (chars_from_end >= written_chars_in_rb(g_p_ringbuf)) 
	{
	/* Requested a larger offset from end than what is available, i.e.
	   set position to oldest position. In this situation, do nothing
	   the init_cache() above already has prepared to read from the
	   beginning. */
	}
	else 
	{
		/* Fake to fool the system that we already has read and cached characters
		   but the last chars_from_end number of characters.
		   Lazy code to decrement from end, not very efficient */
		uint32_t i;
		p_cache->last_cached_rb_wrap_cnt = g_p_ringbuf->wrap_cnt;
		if (g_p_ringbuf->has_wrapped)
			p_cache->last_cached_rb_pos = oldest_position_in_rb(g_p_ringbuf);
		else
			p_cache->last_cached_rb_pos = g_p_ringbuf->next_write_pos;

		dec_rb_pos_and_wrap_cnt(&p_cache->last_cached_rb_pos, &p_cache->last_cached_rb_wrap_cnt);

		for (i=0; i<chars_from_end; i++) 
			dec_rb_pos_and_wrap_cnt(&p_cache->last_cached_rb_pos, &p_cache->last_cached_rb_wrap_cnt);
		p_cache->valid_cached_position=1;
	}

	/* Fill cache again from new position to keep the readahead in front
	 of writes to ringbuf */
	fill_cache(p_cache, 0, g_p_ringbuf, get_str_from_rb);
}


int rb_ioctl(struct file *p_file, unsigned int cmd, unsigned long arg)
{
	char tag_tmp_buf[RB_MAX_TAG_SIZE + 2];
	int res;
	switch (cmd) {
	case RB_IOCRESET:
		RINGBUF_LOGGER_INFO("ringbuf rb_ioctl cmd = RB_IOCRESET\n");
		rb_stop();
		return rb_init();
	case RB_IOCEMPTYDATA:
		RINGBUF_LOGGER_INFO("ringbuf rb_ioctl cmd = RB_IOCEMPTYDATA\n");
		LOCK_MUTEX;
		deny_rb_access();
		save_tags_list();
		init_rb(g_p_ringbuf, g_p_ringbuf->bufsize); /* Keep the old size */
		restore_tag_list();
		allow_rb_access();
		UNLOCK_MUTEX_AND_RETURN(0);
	case RB_IOCSETSIZE:
		RINGBUF_LOGGER_INFO("Ringbuf rb_ioctl cmd = RB_IOCSETSIZE\n");
		LOCK_MUTEX;
		deny_rb_access();
		res = set_ringbuf_size((uint32_t)arg);
		allow_rb_access();
		UNLOCK_MUTEX_AND_RETURN(res);		
	case RB_IOCSETTAG:
		/* ioctl to add a tag, that makes a new device specific to that tag.
		 * Should we really trust the pointer from user space?
		 */
		if (copy_from_user(tag_tmp_buf, (char*)arg, RB_MAX_TAG_SIZE+2)) 
			return -EFAULT;

		tag_tmp_buf[RB_MAX_TAG_SIZE+1] = 0;		
		RINGBUF_LOGGER_INFO("rb_ioctl cmd = RB_IOCSETTAG. tag = %s\n", tag_tmp_buf);
		LOCK_MUTEX;
		deny_rb_access();
		if (tag_exist(tag_tmp_buf))
			res = 0;
		else
			res = create_devices(tag_tmp_buf);
		allow_rb_access();
		UNLOCK_MUTEX_AND_RETURN(res);
	case RB_IOCOFFSET:
		RINGBUF_LOGGER_INFO("Ringbuf rb_ioctl: cmd = RB_IOCOFFSET, localtime offset = %d\n", (int)arg);
		// ioctl to get the offset from UTC to localtime.
		// This is only an integer less than a day off from UTC.
		// Checking is done in the function below.	
		LOCK_MUTEX;
		spin_lock(&rb_kernel_lock);
		res = rt_set_offset((int)arg);
		spin_unlock(&rb_kernel_lock);
		UNLOCK_MUTEX_AND_RETURN(res);
	case RB_IOCSETPOSFROMEND:
		RINGBUF_LOGGER_INFO("Ringbuf rb_ioctl: cmd = RB_IOCSETPOSFROMEND, arg = %d\n", (int)arg);
		LOCK_MUTEX;
		deny_rb_access();
		set_ringbuf_pos_rel_end(p_file, (uint32_t)arg);
		allow_rb_access();
		UNLOCK_MUTEX_AND_RETURN(0);
	default: /* return value according to POSIX */
	return -ENOTTY;
	}
}

///////////////////////////////////////////////////////////////////////////////
/*
  Write the str from kernel space directly to ringbuf
*/
int rb_kspace_write (const char* kstr, uint32_t count)
{
	if(in_interrupt())
	/* This function may not be called in interrupt context
	 * DO NOT USE IN INTERRUPT CONTEXT !!! It will crash on a BUG()
	 * If we allow it we might have a deadlock since we 
	 * use spin_lock() (without disabling irq) in 
	 * add_kspace_chunk_to_rb(), and we don't wan't to 
	 * disable irq every time some userspace process write 
	 * to ringbuf. And we must have one common lock in kernel
	 * space and userspace to protect ringbuf.... that's
	 * the core of this rather complicated problem.
	 */
		BUG();
	else 
	{
		const char* bufp=kstr;
		int tag_no;
		Rb_do_header_param write_header = Rb_write_header;

		tag_no = set_tag("kernel");
		add_kspace_chunk_to_rb(g_p_ringbuf, bufp, tag_no, count, &write_header, 0);

		return count;
	}
}
EXPORT_SYMBOL(rb_kspace_write);




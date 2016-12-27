#ifndef _RINGBUF_MGMT_H_
#define _RINGBUF_MGMT_H_

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/device.h>

typedef loff_t rb_off_t;

/* Number of devices per tag */
#define RB_DEV_PER_TAG 6

/* This string identifies a ringbuf area */
#define RB_IDENT_STRING "Ringbuf"
#define RB_IDENT_MAX_STRING_LEN 20

/* Location for the basic raw devices (all tags). */
#define RB_DEV_ALL_RAW "/dev/rb_0_raw"
#define RB_DEV_ALL_FULL "/dev/rb_0_full"
#define RB_DEV_ALL_SIMPLE "/dev/rb_0_simple"
#define RB_DEV_ALL_RAW_CONT "/dev/rb_0_raw_cont"
#define RB_DEV_ALL_FULL_CONT "/dev/rb_0_full_cont"
#define RB_DEV_ALL_SIMPLE_CONT "/dev/rb_0_simple_cont"

/* The size of a minimum raw file metadata in the beginning of 
   the file, specifying ringbuf version and the "all" tag.
   This size could be measured by "cat /dev/rb_0_raw|wc"
*/
#define RB_RAW_FILE_MIN_METADATA_SIZE 106

/* 
   Header stuff. A raw header looks like "~RBLn.m~[nnnnnnnn mm]" where
   "n" and "m" denotes single digits. 
   Do NOT change the number of n's and m's in the header between the brackets
   without carefully walkthrough the code!
*/
#define RB_VERSION "RB03"
#define RB_UNIQUE_RAW_HEADER_HEAD "~"RB_VERSION"~"

/* Note -1, the trailing null char is not a part of the header */
#define RB_RAW_HEADER_SIZE (sizeof(RB_UNIQUE_RAW_HEADER_HEAD"[nnnnnnnn mm]")-1)

#define RB_UNIQUE_RAW_HEADER_HEAD_SIZE sizeof(RB_UNIQUE_RAW_HEADER_HEAD)-1

/* Maximum number of characters in a textual date string in a header,
   i.e. the length of a date in format "YYYY-MM-DD HH:mm:SS".
   YYYY is the year, MM month, DD days, HH hours, mm minutes SS seconds */
#define RB_MAX_DATE_SIZE 19
/* Max length of a full ringbuf header with date and tag as readable text.
   A full header has format "[YYYY-MM-DD HH:MM:SS <tag_text>]"
   Note -1, the trailing null char is not a part of the header */
#define RB_MAX_FULL_HEADER_SIZE (sizeof("[")+RB_MAX_DATE_SIZE+sizeof(" ")+RB_MAX_TAG_SIZE+sizeof("]")-1)

/* Number of seconds, to collect printouts within the same header */
#define RB_HEADER_COLLECT_TIME 1

/* Maximum number of characters in a textual tag */
#define RB_MAX_TAG_SIZE 50

/* Maximum number of tags */
#define RB_MAX_NO_TAGS 100


/*define the Size of the ring buffer itself in terms of strings to
   store including header.*/
/* Minimum ringbuf size */
#define RB_MIN_RINGBUF_SIZE 700

#define RB_DEFAULT_RINGBUF_SIZE /*500*/15000000

/* Define size for temp area for copying in/out of userspace/kernelspace. 
   Keep this less than the ringbuf size !!!! */
#define RB_KERN_TMP_BUF_SIZE (RB_MIN_RINGBUF_SIZE - 1)


/* Here is the important structure for a ring buffer */
typedef struct {
  char      ident[RB_IDENT_MAX_STRING_LEN];
  /* Identity string for a valid ring buf */
  uint32_t  next_write_pos;    /* Position in ring buffer to write next
				  data. If the buffer is full, this is the
				  same as the oldest data in the ringbuf. */
  uint32_t  last_header_pos;   /* Position of the last written header */
  uint32_t  wrap_cnt;          /* Number of times the ring buffer has 
				  wrapped around */
  int       has_wrapped;       /* Zero initially, set to 1 if we has wrapped
				  at least once */

  int       next_free_tag_no;  /* Guess :-) */
  char      tag_list[RB_MAX_NO_TAGS][RB_MAX_TAG_SIZE + 1];   /* List of our tags */
  uint32_t  bufsize;           /* Allocated size of buf below */
  char*     buf;               /* The buffert to use for ringbuf data.
				  In this buffer each string copied to the
				  buffer is null terminated. Possibly null
				  characters within the buffer is 
				  removed before written to the buffer.*/
  char      buf_data_begin;    /* The first byte of buffered ringbuf data, i.e
				  buf points to address of buf_data_begin */
} Ringbuf;

/* Device types */
typedef enum {
  raw = 0,
  simple,
  full,
  raw_cont,
  simple_cont,
  full_cont
} Device_type;

typedef enum {Rb_no_header,Rb_write_header} Rb_do_header_param;

#define PROC_INFO               "info"
#define PROC_TRACE              "trace"

#define PROCFS_MAX_SIZE              12

void rb_proc_init(void);
void rb_proc_exit(void);

/* Functions to be called from the driver file operation functions.
 */
int rb_read(struct file* p_file, char* buf, size_t count, rb_off_t* offset);
int rb_write(struct file* p_file, const char* buf, size_t count, rb_off_t* offset);
int rb_ioctl(struct file *p_file, unsigned int cmd, unsigned long arg);
int rb_open(struct file* p_file, int tag, Device_type device_type);
int rb_release(struct file* p_file);

/* Init function to be called when driver specific tasks has been done in
   rbmod_init. If init_also_when_valid is true (non-zero) the ringbuf
   contents is cleared also when the previous memory contents is valid. 
*/
int rb_init(void);
  
/* Cleanup funtion to be called when driver specific tasks has been done in
   rbmod_stop
*/
void rb_stop(void);

/* Debug function to dump a string. Useful also outside this file in 
   the main test program */
void rb_dump_string(const char* str, const uint32_t size);

/* Print/dump all info of ringbuf */
void rb_dump(void);

/* Convert between tag number and tag string */
char* rb_get_tag(int tag_no);

/* Get device numbers */
dev_t rb_get_dev_t(void);

/*
  Write the str from kernel space to ringbuf
  DO NOT USE IN INTERRUPT CONTEXT !!! It will crash on a BUG()
*/
int rb_kspace_write (const char* kstr, uint32_t count);


#define OVERWRITE_MSG "UNKNOWN SIZE OF OVERWRITTEN DATA FOLLOWS"

#endif

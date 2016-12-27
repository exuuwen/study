/* Definitions of ioctl commands.*/
#ifndef _RINGBUF_IOCTL_H_
#define _RINGBUF_IOCTL_H_

#include <linux/ioctl.h>

#ifdef RB_MAX_TAG_SIZE
#define RB_MAX_TAG_SIZE_U RB_MAX_TAG_SIZE
#else
#define RB_MAX_TAG_SIZE_U 50
#endif


/* Free magic token according to Documentation/ioctl-number.txt.*/
#define RB_MAGIC '!'
#define RB_IOCRESET _IO(RB_MAGIC, 0)              /* Reset ringbuf */
#define RB_IOCSETTAG  _IOW(RB_MAGIC, 1, char)     /* Set tag */
#define RB_IOCOFFSET  _IOW(RB_MAGIC, 2, int)      /* Set offset from UTC to local time */
#define RB_IOCEMPTYDATA _IOW(RB_MAGIC, 3, int)    /* Yet another reset of
                                                     ringbuf */
#define RB_IOCSETSIZE _IOW(RB_MAGIC, 4, int)      /* Set ringbuf size */
#define RB_IOCSETPOSFROMEND _IOW(RB_MAGIC, 5, int) 
													/* Set ringbuf position
                                                     relative end of ringbuf
                                                     data. 
                                                     unit is characters. */

#endif /* _RINGBUF_IOCTL_H_ */


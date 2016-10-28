#ifndef _RINGBUF_UTILS_H_
#define _RINGBUF_UTILS_H_

#include "ringbuf_mgmt.h"

int is_raw(Device_type device_type);
int is_full(Device_type device_type);
int is_simple(Device_type device_type);


#endif /* _RINGBUF_UTILS_H_*/


#include <linux/vmalloc.h>

#include "ringbuf_mgmt.h"
#include "ringbuf_utils.h"

int is_raw(Device_type device_type)
{
  return (device_type == raw || device_type == raw_cont);
}

int is_full(Device_type device_type)
{
  return (device_type == full || device_type == full_cont);
}

int is_simple(Device_type device_type)
{
  return (device_type == simple || device_type == simple_cont);
}

obj-m   += ringbuf.o
ringbuf-objs := ringbuf_drv.o ringbuf_cache.o ringbuf_mgmt.o ringbuf_utils.o ringbuf_log.o ringbuf_time.o

KDIR    := /lib/modules/$(shell uname -r)/build
PWD     := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf Module.symvers *.ko *.o *.mod.c .*.cmd .tmp_versions modules.order

obj-m   += test_utc.o
test_utc-objs := kernel_utc.o utc.o
KDIR    := /lib/modules/$(shell uname -r)/build
PWD             := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf Module.symvers *.ko *.o *.mod.c .*.cmd .tmp_versions modules.order


ifneq ($(KERNELRELEASE),)
obj-m   := scatter_gather_io.o
else
KDIR    := /lib/modules/$(shell uname -r)/build
PWD             := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf Module.symvers *.ko *.o *.mod.c .*.cmd .tmp_versions modules.order

endif

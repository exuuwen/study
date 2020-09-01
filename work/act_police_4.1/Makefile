PWD = $(shell pwd)
KERNEL_SRC = /lib/modules/`uname -r`/build
obj-m = act_police.o

default:
	$(MAKE) -C $(KERNEL_SRC) SUBDIRS=$(PWD) modules

clean:
	rm -rf Module.symvers *.ko *.o *.mod.c .*.cmd .tmp_versions modules.order *.so

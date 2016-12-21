CC=gcc

PWD = $(shell pwd)
KERNEL_SRC = /lib/modules/`uname -r`/build
obj-m = expect.o

all: expect.ko
expect.ko:  expect.c
	$(MAKE) -C $(KERNEL_SRC) SUBDIRS=$(PWD) modules

clean:
	rm -rf Module.symvers *.ko *.o *.mod.c .*.cmd .tmp_versions modules.order *.so

install: all
	cp -rf expect.ko /lib/modules/`uname -r`/kernel/net/netfilter
	@depmod -a

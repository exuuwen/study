CC=gcc

PWD = $(shell pwd)
KERNEL_SRC = /lib/modules/`uname -r`/build
obj-m = ipt_SNATPOOL.o

all: ipt_SNATPOOL.ko libipt_SNATPOOL.so
ipt_SNATPOOL.ko:  ipt_SNATPOOL.c
	$(MAKE) -C $(KERNEL_SRC) SUBDIRS=$(PWD) modules
libipt_SNATPOOL.so: libipt_SNATPOOL.c
	$(CC)  -fPIC -c libipt_SNATPOOL.c
	ld -shared -o libipt_SNATPOOL.so libipt_SNATPOOL.o

clean:
	rm -rf Module.symvers *.ko *.o *.mod.c .*.cmd .tmp_versions modules.order *.so

install: all
	cp -rf libipt_SNATPOOL.so /usr/lib64/xtables/
	cp -rf ipt_SNATPOOL.ko /lib/modules/`uname -r`/kernel/net/ipv4/netfilter/
	@depmod -a

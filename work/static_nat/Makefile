CC=gcc

PWD = $(shell pwd)
KERNEL_SRC = /lib/modules/`uname -r`/build
obj-m = ipt_STATICSRC.o

all: ipt_STATICSRC.ko libipt_STATICSRC.so
ipt_STATICSRC.ko:  ipt_STATICSRC.c
	$(MAKE) -C $(KERNEL_SRC) SUBDIRS=$(PWD) modules
libipt_STATICSRC.so: libipt_STATICSRC.c
	$(CC)  -fPIC -c libipt_STATICSRC.c
	ld -shared -o libipt_STATICSRC.so libipt_STATICSRC.o

clean:
	rm -rf Module.symvers *.ko *.o *.mod.c .*.cmd .tmp_versions modules.order *.so

install: all
	cp -rf libipt_STATICSRC.so /usr/lib64/xtables/
	cp -rf ipt_STATICSRC.ko /lib/modules/`uname -r`/kernel/net/ipv4/netfilter/
	@depmod -a

CC=gcc

PWD = $(shell pwd)
IPTABLES_SRC = $(PWD)/iptables-1.4.21
INCLUDE = -I$(IPTABLES_SRC)/include
KERNEL_SRC = /lib/modules/`uname -r`/build
obj-m = ipt_TARGET.o ipt_match.o

all: ipt_TARGET.ko ipt_match.ko libipt_TARGET.so libipt_match.so
ipt_TARGET.ko:  ipt_TARGET.c
	$(MAKE) -C $(KERNEL_SRC) SUBDIRS=$(PWD) modules
ipt_match.ko:  ipt_match.c
	$(MAKE) -C $(KERNEL_SRC) SUBDIRS=$(PWD) modules
libipt_TARGET.so: libipt_TARGET.c
	$(CC)  $(INCLUDE) -fPIC -c libipt_TARGET.c
	ld -shared -o libipt_TARGET.so libipt_TARGET.o
libipt_match.so: libipt_match.c
	$(CC)  $(INCLUDE) -fPIC -c libipt_match.c
	ld -shared -o libipt_match.so libipt_match.o

clean:
	rm -rf Module.symvers *.ko *.o *.mod.c .*.cmd .tmp_versions modules.order *.so

install: all
	cp -rf libipt_TARGET.so /usr/lib64/xtables/
	cp -rf libipt_match.so /usr/lib64/xtables/
	cp -rf ipt_TARGET.ko /lib/modules/`uname -r`/kernel/net/ipv4/netfilter/
	cp -rf ipt_match.ko /lib/modules/`uname -r`/kernel/net/ipv4/netfilter/
	@depmod -a

ifneq ($(KERNELRELEASE),)
obj-m := nicmirror.o

else
#KERNEL_SRC = /usr/src/linux
KERNEL_SRC ?= $(firstword $(wildcard /lib/modules/$(shell uname -r)/build /usr/src/linux))
ifeq ($(KERNEL_SRC),)
$(error You need to define KERNEL_SRC)
endif

ifneq ($wildcard $(KERNEL_SRC)/include/linux/modversions.h),)
MODVERSIONS = -DMODVERSIONS
endif

_KVER = $(strip $(shell cat $(KERNEL_SRC)/Makefile | grep -e '^VERSION' | cut -d"=" -f2))
_KPL = $(strip $(shell cat $(KERNEL_SRC)/Makefile | grep -e '^PATCHLEVEL' | cut -d"=" -f2))
_KSUB = $(strip $(shell cat $(KERNEL_SRC)/Makefile | grep -e '^SUBLEVEL' | cut -d"=" -f2))
KERNEL_SERIES=$(_KVER).$(_KPL)

ifeq ($(KERNEL_SERIES), 2.6)
	TARGET=nicmirror.ko
else
	TARGET=nicmirror.o
endif

#CC = x86_64-pc-linux-gnu-gcc-4.1.2
#CFLAGS = -march=k8 -msse3 -O2 -pipe -Wall
CC = gcc
CFLAGS = -O3 -Wall



all: modules

modules: $(TARGET)

nicmirror.o: nicmirror.c
	$(CC) $(CFLAGS) -I$(KERNEL_SRC)/include -c nicmirror.c -D__KERNEL__ -DMODULE $(MODVERSIONS)

nicmirror.ko: nicmirror.c
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

install:
	cp nicmirror.ko /lib/modules/$(shell uname -r)/kernel/net/
	depmod -a
clean:
	-rm -rf *.o *.so *.ko .*.cmd *.mod.c .tmp_versions *~
endif

ifneq ($(KERNELRELEASE),)

obj-m := kobject.o

else
	
KDIR := /HHARM6410/linux-2.6.29
all:
	make -C $(KDIR) M=$(PWD) modules ARCH=arm CROSS_COMPILE=/opt/arm-2008q3/bin/arm-none-linux-gnueabi-
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers  modul*

endif

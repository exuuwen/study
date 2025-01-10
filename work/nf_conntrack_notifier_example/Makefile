ifneq ($(KERNELRELEASE),)

obj-m += ct_example.o
ct_example-objs += conntrack_notifier_example.o

else

KDIR :=/lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

endif

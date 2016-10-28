obj-m   += final.o 
final-objs := file1.o file2.o
KDIR    := /lib/modules/$(shell uname -r)/build
PWD             := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf Module.symvers *.ko *.o *.mod.c .*.cmd .tmp_versions modules.order


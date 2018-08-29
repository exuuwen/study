CC=gcc

PWD = $(shell pwd)
KERNEL_SRC = /lib/modules/`uname -r`/build
obj-m = mem.o

all: mem.ko test
mem.ko:  mem.c
	$(MAKE) -C $(KERNEL_SRC) SUBDIRS=$(PWD) modules
test: test.c
	$(CC) -o test test.c

clean:
	rm -rf Module.symvers *.ko *.o *.mod.c .*.cmd .tmp_versions modules.order test


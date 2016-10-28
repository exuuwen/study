#!/bin/sh

cat /proc/modules | grep mem_vmalloc > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod mem_vmalloc
fi
sudo insmod ./mem_vmalloc.ko


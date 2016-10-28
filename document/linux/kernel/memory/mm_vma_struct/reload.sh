#!/bin/sh

cat /proc/modules | grep mm_vma_struct > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod  mm_vma_struct
fi
sudo insmod ./mm_vma_struct.ko


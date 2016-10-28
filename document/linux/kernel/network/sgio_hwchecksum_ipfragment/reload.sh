#!/bin/sh

cat /proc/modules | grep scatter_gather_io > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod scatter_gather_io
fi

sudo insmod ./scatter_gather_io.ko


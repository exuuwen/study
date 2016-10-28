#!/bin/sh

cat /proc/modules | grep mem > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod mem
fi
sudo insmod ./mem.ko


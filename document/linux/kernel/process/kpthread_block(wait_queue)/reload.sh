#!/bin/sh

cat /proc/modules | grep kthread1 > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod kthread1
fi
sudo insmod ./kthread1.ko


#!/bin/sh

cat /proc/modules | grep kthread > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod kthread
fi
sudo insmod ./kthread.ko


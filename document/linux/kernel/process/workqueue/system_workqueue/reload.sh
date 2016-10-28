#! /bin/bash

cat /proc/modules | grep system_workqueue > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod system_workqueue
fi

sudo insmod system_workqueue.ko

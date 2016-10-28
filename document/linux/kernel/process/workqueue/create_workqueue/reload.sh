#! /bin/bash

cat /proc/modules | grep create_workqueue > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod create_workqueue
fi

sudo insmod create_workqueue.ko

#! /bin/bash

cat /proc/modules | grep wait_event_wq > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod wait_event_wq
fi

sudo insmod wait_event_wq.ko

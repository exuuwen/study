#! /bin/bash

cat /proc/modules | grep work_on_cpu > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod work_on_cpu
fi

sudo insmod work_on_cpu.ko

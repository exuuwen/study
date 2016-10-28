#! /bin/bash

cat /proc/modules | grep detail_wq > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod detail_wq
fi

sudo insmod detail_wq.ko

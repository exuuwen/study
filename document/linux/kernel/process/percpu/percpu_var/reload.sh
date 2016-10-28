#! /bin/bash

cat /proc/modules | grep percpu_var > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod percpu_var
fi

sudo insmod percpu_var.ko

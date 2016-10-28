#! /bin/bash

cat /proc/modules | grep percpu_ptr > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod percpu_ptr
fi

sudo insmod percpu_ptr.ko

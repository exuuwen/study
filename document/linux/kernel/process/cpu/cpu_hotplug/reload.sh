#! /bin/bash

cat /proc/modules | grep cpu_hotplug > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod cpu_hotplug
fi

sudo insmod cpu_hotplug.ko

#!/bin/sh

cat /proc/modules | grep sg_netfilter > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod sg_netfilter
fi

sudo insmod ./sg_netfilter.ko


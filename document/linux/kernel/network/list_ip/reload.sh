#! /bin/bash
if [ $# -lt 1 ]; then
   echo "./reload ifname"
   exit
fi

cat /proc/modules | grep list_ip > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod list_ip
fi

sudo insmod list_ip.ko  "ifname=$1"



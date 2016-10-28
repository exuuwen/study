#! /bin/bash
if [ $# -lt 5 ]; then
   echo "./reload int string array0..2"
   exit
fi

cat /proc/modules | grep module_params > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod module_params
fi

sudo insmod module_params.ko "myint=$1" "mycharp=$2" "myarray=$3,$4,$5"



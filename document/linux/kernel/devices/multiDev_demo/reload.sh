cat /proc/modules | grep multi_devs > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod multi_devs
fi 

sudo insmod multi_devs.ko

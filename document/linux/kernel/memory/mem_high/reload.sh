cat /proc/modules | grep mem_high > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod mem_high
fi
sudo insmod mem_high.ko

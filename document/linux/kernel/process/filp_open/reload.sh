make
cat /proc/modules | grep filp_open > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod filp_open
fi
sudo insmod filp_open.ko

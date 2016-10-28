make
cat /proc/modules | grep filp-open > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod filp-open
fi
sudo insmod filp-open.ko

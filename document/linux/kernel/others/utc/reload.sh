cat /proc/modules | grep test_utc > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod test_utc
fi 

sudo insmod test_utc.ko

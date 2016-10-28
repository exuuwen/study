cat /proc/modules | grep test_log > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod test_log
fi 

sudo insmod test_log.ko

cat /proc/modules | grep ringbuf > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod ringbuf
fi
sudo insmod ringbuf.ko

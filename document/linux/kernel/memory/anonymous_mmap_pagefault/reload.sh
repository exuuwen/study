
cat /proc/modules | grep anonymous_mmap > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod anonymous_mmap 
fi

sudo insmod anonymous_mmap.ko

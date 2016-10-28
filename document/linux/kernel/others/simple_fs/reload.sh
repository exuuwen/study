cat /proc/modules | grep simple_fs > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod simple_fs
fi

sudo insmod simple_fs.ko

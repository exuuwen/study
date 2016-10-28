make clean
make

cat /proc/modules | grep jprobe > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod jprobe
fi

cat /proc/modules | grep jprobe_func > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod jprobe_func
fi

sudo insmod jprobe_func.ko
sudo insmod jprobe.ko

make clean
make

cat /proc/modules | grep kretprobe > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod kretprobe
fi

cat /proc/modules | grep kretprobe_func > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod kretprobe_func
fi

sudo insmod kretprobe_func.ko
sudo insmod kretprobe.ko func=do_fork

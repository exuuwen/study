cat /proc/modules | grep seq_proc > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod seq_proc
fi
sudo insmod seq_proc.ko

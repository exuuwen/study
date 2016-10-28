cat /proc/modules | grep seq_list_proc > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod seq_list_proc
fi
sudo insmod seq_list_proc.ko

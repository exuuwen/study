make clean
make
cat /proc/modules | grep notify_chain > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod notify_chain
fi
sudo insmod notify_chain.ko


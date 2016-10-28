make clean
make

cat /proc/modules | grep pfring > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod pfring
fi
sudo insmod pfring.ko

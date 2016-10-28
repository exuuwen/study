make clean
make

cat /proc/modules | grep BUG_ON > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod BUG_ON
fi
sudo insmod BUG_ON.ko

make clean
make

cat /proc/modules | grep print > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod print
fi
sudo insmod print.ko

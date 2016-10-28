make clean
make

cat /proc/modules | grep final > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod final
fi
sudo insmod final.ko

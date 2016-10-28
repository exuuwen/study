make clean
make

cat /proc/modules | grep jit > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod jit
fi
sudo insmod jit.ko

make clean
make

cat /proc/modules | grep intercept_syscall > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod intercept_syscall
fi
sudo insmod intercept_syscall.ko

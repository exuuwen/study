make clean
make
#gcc -o test_netlink  test_netlink.c
cat /proc/modules | grep test_m > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod test_m
fi
sudo insmod test_m.ko

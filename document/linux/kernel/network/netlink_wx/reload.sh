make clean
make
gcc -o test_netlink  test_netlink.c
cat /proc/modules | grep netlink_event > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod netlink_event
fi
sudo insmod netlink_event.ko

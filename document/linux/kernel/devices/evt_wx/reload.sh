make clean
make
gcc -o test_evt  test_evt.c
cat /proc/modules | grep evt_dev > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod evt_dev
fi
sudo insmod evt_dev.ko

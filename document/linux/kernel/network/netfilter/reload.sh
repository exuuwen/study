make clean
make

cat /proc/modules | grep netfilter > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod netfilter
fi
sudo insmod netfilter.ko

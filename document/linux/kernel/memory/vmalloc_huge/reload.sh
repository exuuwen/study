make clean
make

cat /proc/modules | grep vmalloc_huge > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod vmalloc_huge
fi
sudo insmod vmalloc_huge.ko

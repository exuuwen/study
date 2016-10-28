make clean
make

cat /proc/modules | grep vmalloc_mmap > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod vmalloc_mmap
fi
sudo insmod vmalloc_mmap.ko

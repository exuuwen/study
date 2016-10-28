
cat /proc/modules | grep vmalloc_mmap_fault > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod vmalloc_mmap_fault 
fi

sudo insmod vmalloc_mmap_fault.ko

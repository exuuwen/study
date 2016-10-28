cat /proc/modules | grep kmem_cache > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod kmem_cache
fi
sudo insmod kmem_cache.ko

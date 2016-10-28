make clean
make
gcc -o main  main.c
cat /proc/modules | grep mmap_dev_kmem > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod mmap_dev_kmem
fi
sudo insmod mmap_dev_kmem.ko

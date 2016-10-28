make clean
make
gcc -o main  main.c
cat /proc/modules | grep mmap_proc > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod mmap_proc
fi
sudo insmod mmap_proc.ko

sudo rmmod  inputfifo
sudo insmod ./inputfifo.ko
sudo rm  /dev/input_fifo
sudo mknod  /dev/input_fifo c 233 0

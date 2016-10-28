

cat /proc/modules | grep send_packet > /dev/null
if [ $? -eq 0 ]; then
	sudo rmmod send_packet
fi

sudo insmod send_packet.ko 

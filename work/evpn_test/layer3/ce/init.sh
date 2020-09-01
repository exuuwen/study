ip a del dev lo 10.0.1.241/32
ifconfig net2 0
ifconfig net3 0
sleep 1
ip a a dev lo 10.0.1.241/32
ifconfig net2 172.168.70.241/24 up
ifconfig net3 172.168.69.241/24 up

ifconfig net2 0 down
sleep 1
ifconfig net2 10.0.1.241/24
ip r a 10.0.0.0/8 via 10.0.1.1


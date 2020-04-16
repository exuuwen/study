ip l del dev bond0
sleep 1
ip l add dev bond0 type bond mode 802.3ad
ifconfig net2 0 down
ifconfig net3 0 down
ip l set dev net2 master bond0
ip l set dev net3 master bond0

ifconfig bond0 10.0.1.241/24
ip r a 10.0.0.0/8 via 10.0.1.1


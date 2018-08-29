ip link add red type vrf table 1
ip link add green type vrf table 2

ifconfig eth0 down
ifconfig eth2 down

ip link set red up
ip link set green up

ip link set dev eth0 master red
ip link set dev eth2 master green
ifconfig eth0 up 
ifconfig eth2 up

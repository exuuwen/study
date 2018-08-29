echo 1 > /proc/sys/net/ipv4/ip_forward
ip link add red type vrf table 1
ip link add green type vrf table 2
ip link set red up
ip link set green up
ip netns add ns1
ip netns add ns2
ip netns add ns11
ip netns add ns22
ip l add dev veth1 type veth peer name eth0 netns ns1
ip l add dev veth2 type veth peer name eth0 netns ns2
ip l add dev veth22 type veth peer name eth0 netns ns22
ip l add dev veth11 type veth peer name eth0 netns ns11
ip link set dev veth1 master red
ip link set dev veth2 master green
ip link set dev veth11 master red
ip link set dev veth22 master green

ifconfig veth1 10.0.0.1/24 up
ifconfig veth11 10.0.1.1/24 up
ifconfig veth2 10.0.0.1/24 up
ifconfig veth22 10.0.1.1/24 up

ip netns exec ns1 ifconfig eth0 up
ip netns exec ns1 ip a add dev eth0 10.0.0.7/24
ip netns exec ns1 ip r add default via 10.0.0.1
ip netns exec ns11 ifconfig eth0 up
ip netns exec ns11 ip a add dev eth0 10.0.1.7/24
ip netns exec ns11 ip r add default via 10.0.1.1
ip netns exec ns2 ifconfig eth0 up
ip netns exec ns2 ip a add dev eth0 10.0.0.7/24
ip netns exec ns2 ip r add default via 10.0.0.1
ip netns exec ns22 ifconfig eth0 up
ip netns exec ns22 ip a add dev eth0 10.0.1.7/24
ip netns exec ns22 ip r add default via 10.0.1.1


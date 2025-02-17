ip l del dev vrf0
ip netns del ns1
ip netns del ns2

ip l a dev vrf0 type vrf table 10

ip netns add ns1
ip netns add ns2

ip l add dev veth1 type veth peer name eth0 netns ns1
ip l add dev veth2 type veth peer name eth0 netns ns2

ip l set dev vrf0 up
ip l set dev veth2 master vrf0
ip l set dev veth2 up
ip a a dev veth2 10.0.0.2/24

ip netns exec ns1 ip a a dev eth0 10.0.0.11/24
ip netns exec ns1 ip l set dev eth0 up
ip netns exec ns2 ip a a dev eth0 10.0.0.12/24
ip netns exec ns2 ip l set dev eth0 up

ip l set dev veth1 up
ip a a dev veth1 10.0.0.2/24

echo 1 > /proc/sys/net/ipv4/tcp_l3mdev_accept

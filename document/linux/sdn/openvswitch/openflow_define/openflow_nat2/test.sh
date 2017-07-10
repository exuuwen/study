ip netns add c
ip netns add s
ip link add dev cveth type veth peer name eth0 netns c
ip link add dev sveth type veth peer name eth0 netns s
ifconfig cveth 10.0.0.1/24 up
ifconfig sveth 11.0.0.1/24 up

ip netns exec c ip link set dev eth0 address 52:54:00:81:a0:71
ip netns exec s ip link set dev eth0 address 52:54:00:81:a0:72

ip netns exec c ifconfig eth0 10.0.0.7/24 up
ip netns exec s ifconfig eth0 11.0.0.7/24 up
ip netns exec c ip r add default via 10.0.0.1
ip netns exec s ip r add default via 11.0.0.1


ip netns exec c ip n a dev eth0 10.0.0.1 lladdr 52:54:00:81:a0:72
ip netns exec s ip n a dev eth0 11.0.0.1 lladdr 52:54:00:81:a0:71

ovs-ofctl del-flows br0
ovs-ofctl add-flows br0 flows

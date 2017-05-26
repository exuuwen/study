ip netns add ns1
ip netns add ns2
ip netns add client
ovs-vsctl del-br br0
ovs-vsctl add-br br0
ovs-ofctl del-flows br0
ip link add eth1 type veth peer name eth0 netns ns1 
ip link add eth2 type veth peer name eth0 netns ns2 
ip link add eth-client type veth peer name eth0 netns client
ovs-vsctl add-port br0 eth1  -- set interface eth1 ofport_request=1
ovs-vsctl add-port br0 eth2  -- set interface eth2 ofport_request=2
ovs-vsctl add-port br0 eth-client -- set  interface eth-client ofport_request=7
ifconfig eth1 up
ifconfig eth2 up
ifconfig eth-client up
ip netns exec ns1 ifconfig eth0 192.168.0.1/24 up
ip netns exec ns2 ifconfig eth0 192.168.0.2/24 up
ip netns exec client ifconfig eth0 192.168.0.7/24 up

ovs-ofctl add-flows br0 flows.txt

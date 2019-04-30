ip l del dev vxlan
ip l del dev br0
ip l del dev veth1
ip l del dev veth2
ip netns del ns1
ip netns del ns2

ip netns add ns1 
ip netns add ns2 
ip link add dev veth1 type veth peer name eth0 netns ns1
ip link add dev veth2 type veth peer name eth0 netns ns2
ifconfig veth1 up
ifconfig veth2 11.0.0.1/24 up

ip netns exec ns1 ip l set dev eth0 address fa:ff:ff:ff:ff:ff
ip netns exec ns1 ifconfig eth0 10.0.0.1/24 up
ip netns exec ns1 ip n r 10.0.0.2 lladdr fa:fa:ff:ff:ff:ff dev eth0
ip netns exec ns2 ifconfig eth0 11.0.0.2/24 up
ip netns exec ns2 ip l add dev vxlan type vxlan remote 11.0.0.1 id 1000 dstport 4789
ip netns exec ns2 ip l set dev vxlan address fa:fa:ff:ff:ff:ff
ip netns exec ns2 ifconfig vxlan 10.0.0.2/24 up
ip netns exec ns2 ip n r 10.0.0.1 lladdr fa:ff:ff:ff:ff:ff dev vxlan

ip l add dev vxlan type vxlan external dstport 4789
ifconfig vxlan up

ip l add dev br0 type bridge vlan_filtering 1
brctl addif br0 veth1
brctl addif br0 vxlan
ifconfig br0 up

bridge link set dev veth1 vlan_tunnel on
bridge link set dev vxlan vlan_tunnel on

bridge vlan add dev veth1 vid 100 pvid untagged
bridge vlan add dev vxlan vid 100 
bridge vlan add dev vxlan vid 100 tunnel_info id 1000

#bridge fdb add fa:fa:ff:ff:ff:ff dev vxlan dst 11.0.0.2 vni 1000 self
bridge fdb add fa:fa:ff:ff:ff:ff dev vxlan dst 11.0.0.2 src_vni 1000 vni 1000 self


#bridge fdb show br br0
#bridge fdb show brport vxlan

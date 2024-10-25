# add
ip netns add ns-v6
ip link add veth-v6 type veth peer name eth0 netns ns-v6 
ip link set dev veth-v6 mtu 1540
ip link set dev veth-v6 up
ip netns exec ns-v6 ip link set dev eth0 address fa:ff:ff:ff:ff:f6
ip netns exec ns-v6 ip link set dev eth0 mtu 1540
ip netns exec ns-v6 ip link set dev eth0 up
ip netns exec ns-v6 ip -6 a a 2001:da8:2004:1000:202:116:c0a8:106/96 dev eth0
ip netns exec ns-v6 ip -6 r add default via 2001:da8:2004:1000:202:116:0:1
ip netns exec ns-v6 ip -6 n r 2001:da8:2004:1000:202:116:0:1 lladdr fa:ff:ff:ff:ff:ff dev eth0


#2003:da8:2004:1000:c0a8:4:0:3e8
ip netns add ns-v4
ip link add veth-v4 type veth peer name eth0 netns ns-v4 
ip link set dev veth-v4 mtu 9000
ip link set dev veth-v4 up
ip a a 10.0.0.6/24 dev veth-v4
ip netns exec ns-v4 ip link set eth0 mtu 9000
ip netns exec ns-v4 ip link set eth0 up
ip netns exec ns-v4 ip a a 10.0.0.4/24 dev eth0
ip netns exec ns-v4 ip l add dev gretap type gretap key 1000 remote 10.0.0.6 local 10.0.0.4
ip netns exec ns-v4 ip link set dev gretap mtu 1500
ip netns exec ns-v4 ip link set dev gretap address fa:ff:ff:ff:ff:f4
ip netns exec ns-v4 ip link set dev gretap up
ip netns exec ns-v4 ip a a 192.168.0.4/24 dev gretap
ip netns exec ns-v4 ip r a default via 192.168.0.1 dev gretap
ip netns exec ns-v4 ip n r 192.168.0.1 lladdr fa:ff:ff:ff:ff:ff dev gretap

systemctl start openvswitch

echo 1 > /proc/sys/net/ovs/ovs_ipv4_to_ipv6

ovs-vsctl add-br br0
ovs-vsctl add-port br0 veth-v6
ovs-vsctl add-port br0 gretap -- set interface gretap type=gre options:local_ip=10.0.0.6 options:remote_ip=10.0.0.4 options:key=1000
ovs-ofctl del-flows br0
ovs-ofctl add-flow br0 "in_port=gretap,ip,nw_dst=192.168.1.6,actions=push_vlan:0x8100,set_field:fa:ff:ff:ff:ff:f6->eth_dst,set_field:fa:ff:ff:ff:ff:ff->eth_src,output:veth-v6" -Oopenflow13
ovs-ofctl add-flow br0 "in_port=veth-v6,ip6,ipv6_dst=2003:da8:2004:1000:c0a8:4:0:3e8,actions=push_vlan:0x8100,set_field:fa:ff:ff:ff:ff:f4->eth_dst,set_field:fa:ff:ff:ff:ff:ff->eth_src,output:gretap" -Oopenflow13



# ip netns exec ns-v6 iperf3 -6 -s

# ip netns exec ns-v4 ping 192.168.1.6 
# ip netns exec ns-v4 iperf3 -c 192.168.1.6 -t 10 -i1
# ip netns exec ns-v4 iperf3 -u -c 192.168.1.6 -t 10 -i1
# ip netns exec ns-v4 iperf3 -u -c 192.168.1.6 -t 10 -i1 -l 1600

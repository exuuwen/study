
# namespapce
ip netns add ns1
ip netns add ns2
ip netns add ns-vxlan
ip netns add ns-net
ip netns add ns-net-vxlan
ip lin add dev veth1 type veth peer name eth0 netns ns1
ip lin add dev veth2 type veth peer name eth0 netns ns2
ip lin add dev vxlan type veth peer name eth0 netns ns-vxlan
ip link add dev veth-net type veth peer name eth0 netns ns-net
ip link add dev veth-net-vxlan type veth peer name eth0 netns ns-net-vxlan
ifconfig veth1 up
ifconfig veth2 up
ifconfig vxlan up
ifconfig veth-net-vxlan up
ifconfig veth-net up

ip netns exec ns1 ifconfig eth0 172.16.1.7/24 up 
ip netns exec ns1 ip route add default via 172.16.1.1 
ip netns exec ns2 ifconfig eth0 172.16.1.8/24 up 
ip netns exec ns2 ip route add default via 172.16.1.1 

ip netns exec ns-vxlan ifconfig eth0 10.0.0.2/24 up 
ip netns exec ns-vxlan ip l add tun type vxlan id 10 dev eth0 remote 10.0.0.1 local 10.0.0.2 dstport 4789
ip netns exec ns-vxlan ifconfig tun 172.16.1.9/24 up
ip netns exec ns-vxlan ip route add default via 172.16.1.1 

ip netns exec ns-net ifconfig eth0 172.16.2.7/24 up 
ip netns exec ns-net ip route add default via 172.16.2.1 

ip netns exec ns-net-vxlan ifconfig eth0 10.0.0.3/24 up 
ip netns exec ns-net-vxlan ip l add tun type vxlan id 10 dev eth0 remote 10.0.0.11 local 10.0.0.3 dstport 4789
ip netns exec ns-net-vxlan ifconfig tun 172.16.2.8/24 up
ip netns exec ns-net-vxlan ip route add default via 172.16.2.1 



#vpp
# subnetwork 1, vni 10, bridge 10  route 10
vppctl create bridge-domain 10 learn 1 forward 1 uu-flood 1 flood 1 arp-term 1
vppctl create vxlan tunnel src 10.0.0.1 dst 10.0.0.2 vni 10  decap-next l2              
vppctl create host-interface name veth1
vppctl create host-interface name veth2
vppctl create host-interface name vxlan

vppctl set int state host-veth1 up
vppctl set int state host-veth2 up 
vppctl set int state host-vxlan up

vppctl set interface l2 bridge host-veth1 10     
vppctl set interface l2 bridge host-veth2 10 
vppctl set interface l2 bridge vxlan_tunnel0 10 

vppctl set int ip address host-vxlan 10.0.0.1/24

vppctl loopback create mac 1a:2b:3c:4d:5e:6f
vppctl set interface l2 bridge loop0 10 bvi
vppctl set interface state loop0 up
vppctl set interface ip table loop0 10
vppctl set interface ip address loop0 172.16.1.1/24



vppctl # subnetwork 2, vni 10, bridge 11  route 10
vppctl create bridge-domain 11 learn 1 forward 1 uu-flood 1 flood 1 arp-term 1
vppctl create vxlan tunnel src 10.0.0.11 dst 10.0.0.3 vni 10  decap-next l2
vppctl create host-interface name veth-net
vppctl create host-interface name veth-net-vxlan

vppctl set int state host-veth-net up
vppctl set int state host-veth-net-vxlan up

vppctl set interface l2 bridge host-veth-net 11
vppctl set interface l2 bridge vxlan_tunne1 11

vppctl set int ip address host-veth-net-vxlan 10.0.0.11/24

vppctl loopback create mac 1a:2b:3c:4d:4e:7f
vppctl set interface l2 bridge loop1 11 bvi
vppctl set interface state loop1 up
vppctl set interface ip table loop1 10
vppctl set interface ip address loop1 172.16.2.1/24



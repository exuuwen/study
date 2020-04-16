ip netns del ns11
ip netns del ns22
ip netns del nsc
ip l del dev vni-104001
ip l del dev vni-100100
ip l del dev vni-100200
ip l del dev vrf1
ip l del dev bridge
ip a del dev lo 10.0.0.73/32

sleep 1 

ip a a dev lo 10.0.0.73/32

ip l add dev vni-104001 type vxlan local 10.0.0.73 id 104001 dstport 4789 nolearning
ip l add dev vni-100100 type vxlan local 10.0.0.73 id 100100 dstport 4789 nolearning
ip l add dev vni-100200 type vxlan local 10.0.0.73 id 100200 dstport 4789 nolearning
ip netns add ns11
ip netns add ns22
ip netns add nsc

ip l add dev veth11 type veth peer name eth0 netns ns11
ip netns exec ns11 ifconfig eth0 10.0.1.73/24 up
ip netns exec ns11 ip r a default via 10.0.1.1

ip l add dev veth22 type veth peer name eth0 netns ns22
ip netns exec ns22 ifconfig eth0 10.0.2.73/24 up
ip netns exec ns22 ip r a default via 10.0.2.1

ip l add dev vethc type veth peer name eth0 netns nsc
ip netns exec nsc ifconfig eth0 10.10.10.7/24 up
ip netns exec nsc ip r a 10.0.0.0/8 via 10.10.10.1

ip link add name bridge up type bridge vlan_filtering 1 vlan_default_pvid 0 mcast_snooping 0
ip link add dev vrf1 up type vrf table 1


ip l set dev veth22 master bridge
ip l set dev vni-100200 master bridge
bridge vlan add vid 200 dev bridge self
bridge vlan add vid 200 dev veth22 pvid untagged
bridge vlan add vid 200 dev vni-100200 pvid untagged
ip link add link bridge name vlan200 up type vlan id 200
ip link set dev vlan200 master vrf1
ip a a dev vlan200 10.0.2.11/24
ip link add link vlan200 name vlan200-v up address 00:00:5e:00:01:01 type macvlan mode private
ip link set dev vlan200-v master vrf1 
sleep 1
ip a a dev vlan200-v 10.0.2.1/24 metric 1024
bridge fdb add 00:00:5e:00:01:01 dev bridge self local vlan 200
ip link set dev vni-100200 type bridge_slave neigh_suppress on

sleep 1

ip l set dev veth11 master bridge
ip l set dev vni-100100 master bridge
bridge vlan add vid 100 dev bridge self
bridge vlan add vid 100 dev veth11 pvid untagged
bridge vlan add vid 100 dev vni-100100 pvid untagged
ip link add link bridge name vlan100 up type vlan id 100
ip link add link vlan100 name vlan100-v up address 00:00:5e:00:01:01 type macvlan mode private
ip link set dev vlan100 master vrf1
ip a a dev vlan100 10.0.1.11/24
ip link set dev vlan100-v master vrf1 
sleep 1
ip a a dev vlan100-v 10.0.1.1/24 metric 1024
bridge fdb add 00:00:5e:00:01:01 dev bridge self local vlan 100
ip link set dev vni-100100 type bridge_slave neigh_suppress on

ip l set dev vni-104001 master bridge
bridge vlan add vid 4001 dev bridge self
bridge vlan add vid 4001 dev vni-104001 pvid untagged
ip link add link bridge name vlan4001 up type vlan id 4001
ip link set dev vlan4001 master vrf1 

sleep 1


ip link set dev vethc master vrf1 
ifconfig vethc 10.10.10.1/24 up

sysctl -w net.ipv4.conf.all.rp_filter=0
sysctl -w net.ipv4.conf.vlan200-v.rp_filter=0
sysctl -w net.ipv4.conf.vlan100-v.rp_filter=0
sysctl -w net.ipv4.conf.all.arp_ignore=1

ifconfig vlan200 up
ifconfig veth11 up
ifconfig vlan100 up
ifconfig veth22 up
ifconfig vni-100100 up
ifconfig vni-100200 up
echo 1 > /proc/sys/net/ipv4/ip_forward


ifconfig vlan200 up
ifconfig veth11 up
ifconfig vlan100 up
ifconfig veth22 up
ifconfig vni-104001 up
ifconfig vlan4001 up
ifconfig vni-100100 up
ifconfig vni-100200 up
echo 1 > /proc/sys/net/ipv4/ip_forward

#ip n r 10.0.0.241 lladdr 00:00:5e:00:04:02 dev vlan4001


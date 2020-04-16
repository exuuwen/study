ip l del dev vni-104001
ip l del dev vrf1
ip l del dev bridge
ip a del dev lo 10.0.0.73/32
ip netns del nsc

sleep 1

ip a a dev lo 10.0.0.73/32

ip netns add nsc
ip l add dev vethc type veth peer name eth0 netns nsc
ip netns exec nsc ifconfig eth0 10.10.10.7/24 up
ip netns exec nsc ip r a 10.0.0.0/8 via 10.10.10.1


ip l add dev vni-104001 type vxlan local 10.0.0.73 id 104001 dstport 4789 nolearning

ip link add name bridge up type bridge vlan_filtering 1 vlan_default_pvid 0 mcast_snooping 0
ip link add dev vrf1 up type vrf table 1

ip l set dev vni-104001 master bridge
bridge vlan add vid 4001 dev bridge self
bridge vlan add vid 4001 dev vni-104001 pvid untagged
ip link add link bridge name vlan4001 up type vlan id 4001
ip link set dev vlan4001 master vrf1 
ip link set dev vethc master vrf1 
ifconfig vethc 10.10.10.1/24 up

sysctl -w net.ipv4.conf.all.rp_filter=0
sysctl -w net.ipv4.conf.all.arp_ignore=1


ifconfig vni-104001 up
ifconfig vlan4001 up
echo 1 > /proc/sys/net/ipv4/ip_forward


modprobe bonding mode=4 miimon=100 lacp_rate=1
ip l del dev bond0
ip netns add ns1
ip netns add ns2
ip l add dev veth1 type veth peer name eth0 netns ns1
ip l add dev veth2 type veth peer name eth0 netns ns2
ip l add dev bond0 type bond mode 802.3ad
ifconfig bond0 up
echo 1 > /sys/class/net/bond0/bonding/xmit_hash_policy

ip l add link veth1 name veth1.1 type vlan id 1
ip l add link veth2 name veth2.2 type vlan id 2
ifconfig veth1 up
ifconfig veth2 up
ifconfig veth2.2 down
ip l set dev veth2.2 master bond0
ifconfig veth1.1 down
ip l set dev veth1.1 master bond0
ifconfig veth1.1 up
ifconfig veth2.2 up

#echo 2e:d1:93:25:2c:c0 > /sys/class/net/bond0/bonding/ad_actor_system
ip netns exec ns1 ip l add dev bond0 type bond mode 802.3ad
ip netns exec ns1 ifconfig bond0 up
ip netns exec ns1 ip l add link eth0 name eth0.1 type vlan id 1
ip netns exec ns1 ifconfig eth0 up
ip netns exec ns1 ifconfig eth0.1 down
ip netns exec ns1 ip l set dev eth0.1 master bond0
ip netns exec ns1 ifconfig eth0.1 up

ip netns exec ns2 ip l add dev bond0 type bond mode 802.3ad
ip netns exec ns2 ifconfig bond0 up
ip netns exec ns2 ip l add link eth0 name eth0.2 type vlan id 2
ip netns exec ns2 ifconfig eth0 up
ip netns exec ns2 ifconfig eth0.2 down
ip netns exec ns2 ip l set dev eth0.2 master bond0
ip netns exec ns2 ifconfig eth0.2 up



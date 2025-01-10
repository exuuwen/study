ip netns del ns0
ip netns del peer
ip l del dev vxlan
ip l del dev peer
ip l del dev veth0

ip netns add ns0
ip link add dev veth0 type veth peer name eth0 netns ns0
ip link add dev vxlan type vxlan dstport 4789 external
ip l set dev vxlan up
ip l set dev veth0 up
tc qdisc add dev veth0 clsact
tc qdisc add dev vxlan clsact

ip netns exec ns0 ip l set dev eth0 address fa:ff:ff:ff:ff:f7
ip netns exec ns0 ip l set dev eth0 up
ip netns exec ns0 ip a a dev eth0 10.0.0.7/24
ip netns exec ns0 ip n r 10.0.0.8 lladdr fa:ff:ff:ff:ff:f8 dev eth0

ip netns add peer
ip l add dev peer type veth peer name eth0 netns peer
ip l set dev peer up
ip a a dev peer 172.0.0.7/24 

ip netns exec peer ip l set dev eth0 up
ip netns exec peer ip a a dev eth0 172.0.0.8/24

ip netns exec peer ip l add vxlan type vxlan remote 172.0.0.7 vni 1000 local 172.0.0.8 dstport 4789
ip netns exec peer ip l set dev vxlan address fa:ff:ff:ff:ff:f8
ip netns exec peer ip n r 10.0.0.7 lladdr fa:ff:ff:ff:ff:f7 dev vxlan
ip netns exec peer ip l set dev vxlan up
ip netns exec peer ip a a dev vxlan 10.0.0.8/24


#     1000   0      10.0.0.7               0    21      0
# e8 03 0 0  0 0 0 0  7 0 0 10       0 0 0 0  21 0 0 0  0 0 0 0
# 0         21   10.0.0.8               1000      22       172.0.0.8
# 0 0 0 0  22 0 0 0  8 0 0 10       e8 03 0 0  22 0 0 0  8 0 0 ac
#bpftool map update name tunnel_map key 232 3 0 0 0 0 0 0 7 0 0 10 value 0 0 0 0 21 0 0 0 0 0 0 0
#bpftool map update name tunnel_map key 0 0 0 0 21 0 0 0 8 0 0 10 value 232 3 0 0 22 0 0 0 8 0 0 172


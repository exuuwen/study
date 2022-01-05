ovs-vsctl del-port veth1
ovs-vsctl del-port veth2
ovs-vsctl del-port gre

ip netns del ns1
ip netns del ns2
ip netns del nsc

ovs-ofctl del-flows br0

sleep 2


ip netns add ns1
ip netns add ns2
ip netns add nsc


ip l a dev veth1 type veth peer name eth0 netns ns1
ip l a dev veth2 type veth peer name eth0 netns ns2
ip l a dev vethc type veth peer name eth0 netns nsc


ip netns exec ns1 ip l set dev eth0 address fa:17:ff:ff:ff:ff
ip netns exec ns1 ifconfig eth0 mtu 1452
ip netns exec ns1 ifconfig eth0 10.0.0.7/24 up
ip netns exec ns1 ip r a default via 10.0.0.1
ip netns exec ns1 ip n a 10.0.0.1 lladdr fa:ff:ff:ff:ff:ff dev eth0
ip netns exec ns2 ip l set dev eth0 address fa:27:ff:ff:ff:ff
ip netns exec ns2 ifconfig eth0 mtu 1452
ip netns exec ns2 ifconfig eth0 10.0.0.7/24 up
ip netns exec ns2 ip r a default via 10.0.0.1
ip netns exec ns2 ip n a 10.0.0.1 lladdr fa:ff:ff:ff:ff:ff dev eth0

ip netns exec nsc ip l a dev tun type gretap key 1000 remote 172.168.0.7
ip netns exec nsc ifconfig eth0 172.168.0.1 up
ip netns exec nsc ip l set dev tun address fa:f7:ff:ff:ff:ff
ip netns exec nsc ifconfig tun 11.0.0.7/24 up
ip netns exec nsc ip r a default via 11.0.0.1
ip netns exec nsc ip n a 11.0.0.1 lladdr fa:ff:ff:ff:ff:ff dev tun

ifconfig veth1 up
ifconfig veth2 up
ifconfig vethc 172.168.0.7/24 up


ovs-vsctl add-port br0 veth1 -- set in veth1 ofport_request=1
ovs-vsctl add-port br0 veth2 -- set in veth2 ofport_request=2
ovs-vsctl add-port br0 gre -- set in gre type=gre ofport_request=7 options:key=1000 options:remote_ip=172.168.0.1


ovs-ofctl add-flow br0 "ip,in_port=7,nw_dst=1.1.1.7 actions=ct(table=1,zone=1,nat)"
ovs-ofctl add-flow br0 "ip,in_port=7,nw_dst=1.1.2.7 actions=ct(table=1,zone=2,nat)"
ovs-ofctl add-flow br0 "table=1,ct_state=+new+trk,ct_zone=1,ip,icmp,actions=ct(commit,zone=1,nat(dst=10.0.0.7)),set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:fa:17:ff:ff:ff:ff->dl_dst,dec_ttl,output:1"
ovs-ofctl add-flow br0 "table=1,ct_state=+new+trk,ct_zone=1,ip,tcp,tp_dst=21,actions=ct(commit,zone=1,nat(dst=10.0.0.7)),set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:fa:17:ff:ff:ff:ff->dl_dst,dec_ttl,output:1"
ovs-ofctl add-flow br0 "table=1,ct_state=+est+trk,ct_zone=1,ip,actions=set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:fa:17:ff:ff:ff:ff->dl_dst,dec_ttl,output:1"
ovs-ofctl add-flow br0 "table=1,ct_state=+new+trk,ct_zone=2,ip,icmp,actions=ct(commit,zone=2,nat(dst=10.0.0.7)),set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:fa:27:ff:ff:ff:ff->dl_dst,dec_ttl,output:2"
ovs-ofctl add-flow br0 "table=1,ct_state=+new+trk,ct_zone=2,ip,tcp,tp_dst=22,actions=ct(commit,zone=2,nat(dst=10.0.0.7)),set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:fa:27:ff:ff:ff:ff->dl_dst,dec_ttl,output:2"
ovs-ofctl add-flow br0 "table=1,ct_state=+est+trk,ct_zone=2,ip,actions=set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:fa:27:ff:ff:ff:ff->dl_dst,dec_ttl,output:2"


ovs-ofctl add-flow br0 "ip,in_port=1,nw_dst=11.0.0.7 actions=ct(table=2,zone=1,nat)"
ovs-ofctl add-flow br0 "ip,in_port=2,nw_dst=11.0.0.7 actions=ct(table=2,zone=2,nat)"
ovs-ofctl add-flow br0 "table=2,priority=65000,ct_state=+new+trk,ct_zone=1,ip,tcp,tp_dst=17,actions=drop"
ovs-ofctl add-flow br0 "table=2,priority=65000,ct_state=+new+trk,ct_zone=2,ip,tcp,tp_dst=27,actions=drop"
ovs-ofctl add-flow br0 "table=2,ct_state=+new+trk,ct_zone=1,ip,actions=ct(commit,zone=1,nat(src=1.1.1.7)),set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:fa:f7:ff:ff:ff:ff->dl_dst,dec_ttl,output:7"
ovs-ofctl add-flow br0 "table=2,ct_state=+new+trk,ct_zone=2,ip,actions=ct(commit,zone=2,nat(src=1.1.2.7)),set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:fa:f7:ff:ff:ff:ff->dl_dst,dec_ttl,output:7"
ovs-ofctl add-flow br0 "table=2,ct_state=+est+trk,ip,actions=set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:fa:f7:ff:ff:ff:ff->dl_dst,dec_ttl,output:7"

echo 1 > /proc/sys/net/ipv4/ip_forward

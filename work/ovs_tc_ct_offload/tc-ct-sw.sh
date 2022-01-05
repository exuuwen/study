ip netns del ns1
ip netns del ns2
ip netns del nsc

sleep 2


ip netns add ns1
ip netns add ns2
ip netns add nsc


ip l a dev veth1 type veth peer name eth0 netns ns1
ip l a dev veth2 type veth peer name eth0 netns ns2
ip l a dev vethc type veth peer name eth0 netns nsc


ip netns exec ns1 ip l set dev eth0 address fa:17:ff:ff:ff:ff
ip netns exec ns1 ifconfig eth0 10.0.0.7/24 up
ip netns exec ns1 ip r a default via 10.0.0.1
ip netns exec ns1 ip n a 10.0.0.1 lladdr fa:ff:ff:ff:ff:ff dev eth0
ip netns exec ns2 ip l set dev eth0 address fa:27:ff:ff:ff:ff
ip netns exec ns2 ifconfig eth0 10.0.0.7/24 up
ip netns exec ns2 ip r a default via 10.0.0.1
ip netns exec ns2 ip n a 10.0.0.1 lladdr fa:ff:ff:ff:ff:ff dev eth0
ip netns exec nsc ip l set dev eth0 address fa:f7:ff:ff:ff:ff
ip netns exec nsc ifconfig eth0 11.0.0.7/24 up
ip netns exec nsc ip r a default via 11.0.0.1
ip netns exec nsc ip n a 11.0.0.1 lladdr fa:ff:ff:ff:ff:ff dev eth0

ifconfig veth1 up
ifconfig veth2 up
ifconfig vethc up


tc qdisc add dev veth1 ingress
tc qdisc add dev veth2 ingress
tc qdisc add dev vethc ingress

tc filter add dev veth1 ingress prio 1 chain 0 proto ip flower dst_ip 11.0.0.7 ct_state -trk action ct zone 1 nat pipe action goto chain 1
tc filter add dev veth1 ingress prio 100 chain 1 proto ip flower ct_state +trk+new action ct zone 1 commit nat src addr 1.1.1.7 pipe action pedit ex munge eth dst set fa:f7:ff:ff:ff:ff pipe action pedit ex munge eth src set fa:ff:ff:ff:ff:ff pipe action mirred egress redirect dev vethc
tc filter add dev veth1 ingress prio 1 chain 1 proto ip flower ct_state +trk+new ip_proto tcp dst_port 17 action drop
tc filter add dev veth1 ingress prio 1 chain 1 proto ip flower ct_state +trk+est action pipe action pedit ex munge eth dst set fa:f7:ff:ff:ff:ff pipe action pedit ex munge eth src set fa:ff:ff:ff:ff:ff pipe action mirred egress redirect dev vethc

tc filter add dev veth2 ingress prio 1 chain 0 proto ip flower dst_ip 11.0.0.7 ct_state -trk action ct zone 2 nat pipe action goto chain 1
tc filter add dev veth2 ingress prio 100 chain 1 proto ip flower ct_state +trk+new action ct zone 2 commit nat src addr 1.1.2.7 pipe action pedit ex munge eth dst set fa:f7:ff:ff:ff:ff pipe action pedit ex munge eth src set fa:ff:ff:ff:ff:ff pipe action mirred egress redirect dev vethc
tc filter add dev veth2 ingress prio 1 chain 1 proto ip flower ct_state +trk+new ip_proto tcp dst_port 27 action drop
tc filter add dev veth2 ingress prio 1 chain 1 proto ip flower ct_state +trk+est action pipe action pedit ex munge eth dst set fa:f7:ff:ff:ff:ff pipe action pedit ex munge eth src set fa:ff:ff:ff:ff:ff pipe action mirred egress redirect dev vethc

tc filter add dev vethc ingress prio 1 chain 0 proto ip flower dst_ip 1.1.1.7 ct_state -trk action ct zone 1 nat pipe action goto chain 1
tc filter add dev vethc ingress prio 1 chain 1 proto ip flower ct_state +trk+new ip_proto tcp dst_port 21 action ct zone 1 commit nat dst addr 10.0.0.7 pipe action pedit ex munge eth dst set fa:17:ff:ff:ff:ff pipe action pedit ex munge eth src set fa:ff:ff:ff:ff:ff pipe action mirred egress redirect dev veth1
tc filter add dev vethc ingress prio 1 chain 1 proto ip flower ct_state +trk+new ip_proto icmp action ct zone 1 commit nat dst addr 10.0.0.7 pipe action pedit ex munge eth dst set fa:17:ff:ff:ff:ff pipe action pedit ex munge eth src set fa:ff:ff:ff:ff:ff pipe action mirred egress redirect dev veth1
tc filter add dev vethc ingress prio 100 chain 1 proto ip flower ct_state +trk+new action drop
tc filter add dev vethc ingress prio 1 chain 1 proto ip flower ct_state +trk+est action pipe action pedit ex munge eth dst set fa:17:ff:ff:ff:ff pipe action pedit ex munge eth src set fa:ff:ff:ff:ff:ff pipe action mirred egress redirect dev veth1

tc filter add dev vethc ingress prio 1 chain 0 proto ip flower dst_ip 1.1.2.7 ct_state -trk action ct zone 2 nat pipe action goto chain 2
tc filter add dev vethc ingress prio 1 chain 2 proto ip flower ct_state +trk+new ip_proto tcp dst_port 22 action ct zone 2 commit nat dst addr 10.0.0.7 pipe action pedit ex munge eth dst set fa:27:ff:ff:ff:ff pipe action pedit ex munge eth src set fa:ff:ff:ff:ff:ff pipe action mirred egress redirect dev veth2
tc filter add dev vethc ingress prio 1 chain 2 proto ip flower ct_state +trk+new ip_proto icmp action ct zone 2 commit nat dst addr 10.0.0.7 pipe action pedit ex munge eth dst set fa:27:ff:ff:ff:ff pipe action pedit ex munge eth src set fa:ff:ff:ff:ff:ff pipe action mirred egress redirect dev veth2
tc filter add dev vethc ingress prio 100 chain 2 proto ip flower ct_state +trk+new action drop
tc filter add dev vethc ingress prio 1 chain 2 proto ip flower ct_state +trk+est action pipe action pedit ex munge eth dst set fa:27:ff:ff:ff:ff pipe action pedit ex munge eth src set fa:ff:ff:ff:ff:ff pipe action mirred egress redirect dev veth2

echo 1 > /proc/sys/net/ipv4/ip_forward

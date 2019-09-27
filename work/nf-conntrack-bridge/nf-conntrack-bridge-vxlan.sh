nft delete table bridge firewall
#nft -a list ruleset
ip netns del ns11 
ip netns del ns12
ip netns del ns21 
ip netns del ns22
ip l del dev vxlan

ip l del dev br0

sleep 2

ip netns add ns11 
ip netns add ns12
ip netns add ns21 
ip netns add ns22
ip l add dev veth11 type veth peer name eth0 netns ns11
ip l add dev veth12 type veth peer name eth0 netns ns12
ip l add dev veth21 type veth peer name eth0 netns ns21
ip l add dev veth22 type veth peer name eth0 netns ns22
ifconfig veth11 up
ifconfig veth12 11.0.0.1/24 up
ifconfig veth21 up
ifconfig veth22 12.0.0.1/24 up
ip netns exec ns11 ip link a link eth0 name vlan type vlan id 100
ip netns exec ns11 ip a a dev vlan 10.0.0.7/24 
ip netns exec ns12 ifconfig eth0 11.0.0.2/24 up
ip netns exec ns12 ip l add dev vxlan type vxlan remote 11.0.0.1 id 1000 dstport 4789
ip netns exec ns12 ip l set dev vxlan address fa:fa:ff:ff:ff:ff
ip netns exec ns12 ip a a dev vxlan 10.0.0.8/24 

ip netns exec ns21 ip a a dev eth0 10.0.0.7/24 
ip netns exec ns22 ifconfig eth0 12.0.0.2/24 up
ip netns exec ns22 ip l add dev vxlan type vxlan remote 12.0.0.1 id 2000 dstport 4789
ip netns exec ns22 ip l set dev vxlan address fa:fb:ff:ff:ff:ff
ip netns exec ns22 ip a a dev vxlan 10.0.0.8/24 

ip netns exec ns11 ifconfig eth0 up
ip netns exec ns12 ifconfig eth0 up
ip netns exec ns21 ifconfig eth0 up
ip netns exec ns22 ifconfig eth0 up

ip netns exec ns11 ifconfig vlan up
ip netns exec ns12 ifconfig vxlan up
ip netns exec ns22 ifconfig vxlan up

ip l add dev vxlan type vxlan external dstport 4789
ifconfig vxlan up

ip l add dev br0 type bridge vlan_filtering 1 
brctl addif br0 veth11
brctl addif br0 veth21
brctl addif br0 vxlan

ifconfig br0 up

bridge link set dev veth11 vlan_tunnel on
bridge link set dev veth21 vlan_tunnel on
bridge link set dev vxlan vlan_tunnel on

bridge vlan add dev veth11 vid 100 
bridge vlan add dev veth21 vid 200 pvid untagged
bridge vlan add dev vxlan vid 100
bridge vlan add dev vxlan vid 200
bridge vlan add dev vxlan vid 100 tunnel_info id 1000
bridge vlan add dev vxlan vid 200 tunnel_info id 2000

bridge fdb add fa:fa:ff:ff:ff:ff dev vxlan dst 11.0.0.2 src_vni 1000 vni 1000 self
bridge fdb add fa:fb:ff:ff:ff:ff dev vxlan dst 12.0.0.2 src_vni 2000 vni 2000 self
bridge fdb add ff:ff:ff:ff:ff:ff dev vxlan dst 11.0.0.2 src_vni 1000 vni 1000 self
bridge fdb append ff:ff:ff:ff:ff:ff dev vxlan dst 12.0.0.2 src_vni 2000 vni 2000 self

nft add table bridge firewall
nft add chain bridge firewall zones { type filter hook prerouting priority - 300 \; }
nft add rule bridge firewall zones counter ct zone set iif map { "veth21" : 2 }
nft add rule bridge firewall zones counter ct zone set vlan id map { 100 : 1, 200 : 2 }

nft add chain bridge firewall rule-100-ingress
nft add rule bridge firewall rule-100-ingress ct zone 1 ct state established,related counter accept
nft add rule bridge firewall rule-100-ingress ct zone 1 ct state invalid counter drop
nft add rule bridge firewall rule-100-ingress ct zone 1 tcp dport 22 ct state new counter accept
nft add rule bridge firewall rule-100-ingress ct zone 1 ip protocol icmp ct state new counter accept
nft add rule bridge firewall rule-100-ingress counter drop

nft add chain bridge firewall rule-100-egress
nft add rule bridge firewall rule-100-egress ct zone 1 ct state established,related counter accept
nft add rule bridge firewall rule-100-egress ct zone 1 ct state invalid counter drop
nft add rule bridge firewall rule-100-egress ct zone 1 tcp dport 22 ct state new counter drop
nft add rule bridge firewall rule-100-egress ct zone 1 ip protocol icmp ct state new counter drop
nft add rule bridge firewall rule-100-egress counter accept

nft add chain bridge firewall rule-200-ingress
nft add rule bridge firewall rule-200-ingress ct zone 2 ct state established,related counter accept
nft add rule bridge firewall rule-200-ingress ct zone 2 ct state invalid counter drop
nft add rule bridge firewall rule-200-ingress ct zone 2 tcp dport 23 ct state new counter accept
nft add rule bridge firewall rule-200-ingress counter drop

nft add chain bridge firewall rule-200-egress
nft add rule bridge firewall rule-200-egress ct zone 2 ct state established,related counter accept
nft add rule bridge firewall rule-200-egress ct zone 2 ct state invalid counter drop
nft add rule bridge firewall rule-200-egress ct zone 2 tcp dport 23 ct state new counter drop
nft add rule bridge firewall rule-200-egress counter accept

nft add chain bridge firewall rules-all { type filter hook prerouting priority - 150 \; }
nft add rule bridge firewall rules-all counter meta protocol ip iif vmap { "veth11" : jump rule-100-egress, "veth21" : jump rule-200-egress } 
nft add rule bridge firewall rules-all counter iif "vxlan" vlan type ip vlan id vmap { 100 : jump rule-100-ingress, 200 : jump rule-200-ingress } 








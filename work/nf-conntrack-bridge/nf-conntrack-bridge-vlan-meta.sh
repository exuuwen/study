nft delete table bridge firewall
#nft -a list ruleset
#nft --debug=netlink add rule bridge firewall rule-100-ingress ip protocol icmp drop
ip netns del ns11 
ip netns del ns12
ip netns del ns21 
ip netns del ns22

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
ifconfig veth12 up
ifconfig veth21 up
ifconfig veth22 up
ip netns exec ns11 ip link a link eth0 name vlan type vlan id 100 
ip netns exec ns11 ip a a dev vlan 10.0.0.7/24 
ip netns exec ns12 ip link a link eth0 name vlan type vlan id 100 
ip netns exec ns12 ip a a dev vlan 10.0.0.8/24 
ip netns exec ns21 ip a a dev eth0 10.0.0.7/24 
ip netns exec ns22 ip a a dev eth0 10.0.0.8/24 
ip netns exec ns11 ifconfig eth0 up
ip netns exec ns12 ifconfig eth0 up
ip netns exec ns21 ifconfig eth0 up
ip netns exec ns22 ifconfig eth0 up

ip netns exec ns11 ifconfig vlan up
ip netns exec ns12 ifconfig vlan up

ip l add dev br0 type bridge vlan_filtering 1 
brctl addif br0 veth11
brctl addif br0 veth12
brctl addif br0 veth21
brctl addif br0 veth22

ifconfig br0 up

bridge vlan add dev veth11 vid 100 #pvid untagged
bridge vlan add dev veth12 vid 100 #pvid untagged
bridge vlan add dev veth21 vid 200 pvid untagged
bridge vlan add dev veth22 vid 200 pvid untagged

nft add table bridge firewall
nft add chain bridge firewall zones { type filter hook prerouting priority - 300 \; }
nft add rule bridge firewall zones counter meta brvlan set meta brpvid
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
nft add rule bridge firewall rules-all counter meta protocol ip iif vmap { "veth12" : jump rule-100-ingress, "veth22" : jump rule-200-ingress, "veth11" : jump rule-100-egress, "veth21" : jump rule-200-egress } 








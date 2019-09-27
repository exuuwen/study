nft delete table bridge firewall
#nft -a list ruleset
#nft --debug=netlink add rule bridge firewall rule-100-ingress ip protocol icmp drop
ip netns del ns1 

ip l del dev br0

sleep 2

ip netns add ns1 
ip l add dev veth1 type veth peer name eth0 netns ns1
ifconfig veth1 up
ip netns exec ns1 ip a a dev eth0 10.0.0.8/24 
ip netns exec ns1 ifconfig eth0 up

ip l add dev br0 type bridge vlan_filtering 1 
brctl addif br0 veth1

ifconfig br0 up
ifconfig br0 10.0.0.7/24 up

bridge vlan add dev veth1 vid 100 pvid untagged
bridge vlan add dev br0 vid 100 pvid untagged self  # set br-test port must take with self

rmmod nf_conntrack_bridge

#ip l a link br0 name br0.100 type vlan id 100
#ifconfig br0.100 10.0.0.7/24 up
#bridge vlan add dev br0 vid 100 self  # set br-test port must take with self


nft add table firewall-net
nft add table bridge firewall

nft add chain bridge firewall zone-pre { type filter hook prerouting priority - 300 \; }
nft add rule bridge firewall zone-pre counter ct zone set iif map { "veth1" : 1 }

nft add chain firewall-net zone-out { type filter hook output priority - 300 \; }
nft add rule firewall-net zone-out counter ct zone set oif map { "br0" : 1 }

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
#nft add rule bridge firewall rule-100-egress ct zone 1 ip protocol icmp ct state new counter drop
nft add rule bridge firewall rule-100-egress counter accept

nft add chain bridge firewall rules-ins { type filter hook prerouting priority - 150 \; }
nft add rule bridge firewall rules-ins counter meta protocol ip iif vmap { "veth1" : jump rule-100-ingress }

nft add chain bridge firewall rules-outs { type filter hook output priority - 150 \; }
nft add rule bridge firewall rules-outs counter meta protocol ip obrname vmap { "br0" : jump rule-100-egress }








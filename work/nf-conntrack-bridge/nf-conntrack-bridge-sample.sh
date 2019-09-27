nft delete table bridge firewall
#nft -a list ruleset
ip netns del ns1 
ip netns del ns2

sleep 2

ip netns add ns1 
ip netns add ns2
ip l add dev veth1 type veth peer name eth0 netns ns1
ip l add dev veth2 type veth peer name eth0 netns ns2
ifconfig veth1 up
ifconfig veth2 up
ip netns exec ns1 ip a a dev eth0 10.0.0.7/24 
ip netns exec ns2 ip a a dev eth0 10.0.0.8/24 
ip netns exec ns1 ifconfig eth0 up
ip netns exec ns2 ifconfig eth0 up


nft add table bridge firewall
nft add chain bridge firewall zones { type filter hook prerouting priority - 300 \; }
nft add rule bridge firewall zones counter ct zone set iif map { "veth1" : 1, "veth2" : 1 }

nft add chain bridge firewall rule-1000-ingress
nft add rule bridge firewall rule-1000-ingress ct zone 1 ct state established,related counter accept
nft add rule bridge firewall rule-1000-ingress ct zone 1 ct state invalid counter drop
nft add rule bridge firewall rule-1000-ingress ct zone 1 tcp dport 22 ct state new counter accept
nft add rule bridge firewall rule-1000-ingress ct zone 1 ip protocol icmp ct state new counter accept
nft add rule bridge firewall rule-1000-ingress counter drop
nft add chain bridge firewall rules-all { type filter hook prerouting priority - 150 \; }
nft add rule bridge firewall rules-all ip daddr vmap { "10.0.0.7" : jump rule-1000-ingress } 


nft add chain bridge firewall rule-1000-egress
nft add rule bridge firewall rule-1000-egress ct zone 1 ct state established,related counter accept
nft add rule bridge firewall rule-1000-egress ct zone 1 ct state invalid counter drop
nft add rule bridge firewall rule-1000-egress ct zone 1 tcp dport 22 ct state new counter drop
nft add rule bridge firewall rule-1000-egress counter accept
nft add rule bridge firewall rules-all iif vmap { "veth1" : jump rule-1000-egress } 






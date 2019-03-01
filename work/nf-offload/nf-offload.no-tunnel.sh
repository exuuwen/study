nft delete table firewall
#nft delete table netdev ingress-zones
ip netns del ns1 
ip netns del cl
ip l del dev br0 
ip l del dev user1 

sleep 2

ip netns add ns1
ip netns add cl
ip l add dev veth1 type veth peer name eth0 netns ns1
ip l add dev vethc type veth peer name eth0 netns cl
ip netns exec cl ifconfig eth0 1.1.1.7/24 up
ip netns exec ns1 ifconfig eth0 10.0.0.7/24 up
ifconfig veth1 up
ifconfig vethc up
brctl addbr br0
ifconfig br0 up
brctl addif br0 veth1
brctl addif br0 vethc
ip addr add dev br0 1.1.1.1/24 
ip addr add dev br0 10.0.0.1/24 
ip netns exec cl ip r a default via 1.1.1.1
ip netns exec ns1 ip r a default via 10.0.0.1
ip link add user1 type vrf table 1
ip l set user1 up 
ip l set dev br0 master user1


nft add table firewall
nft add chain firewall zones { type filter hook prerouting priority - 300 \; }
nft add rule firewall zones counter ct zone set iif map { "br0" : 1 }
nft add chain firewall rule-1000-ingress
nft add rule firewall rule-1000-ingress ct zone 1 ct state established,related counter accept
nft add rule firewall rule-1000-ingress ct zone 1 ct state invalid counter drop
nft add rule firewall rule-1000-ingress ct zone 1 tcp dport 22 ct state new counter accept
nft add rule firewall rule-1000-ingress ct zone 1 udp dport 22 ct state new counter accept
nft add rule firewall rule-1000-ingress ct zone 1 ip protocol icmp ct state new counter accept
nft add rule firewall rule-1000-ingress counter drop
nft add chain firewall rules-all { type filter hook prerouting priority - 150 \; }
nft add rule firewall rules-all ip daddr vmap { "2.2.2.11" : jump rule-1000-ingress } 
nft add chain firewall rule-1000-egress
nft add rule firewall rule-1000-egress ct state established,related counter accept
nft add rule firewall rule-1000-egress ct state invalid counter drop
nft add rule firewall rule-1000-egress tcp dport 22 ct state new counter drop
nft add rule firewall rule-1000-egress counter accept
nft add rule firewall rules-all ct zone vmap { 1 : jump rule-1000-egress } 
nft add chain firewall dnat-all { type nat hook prerouting priority - 100 \; }
nft add chain firewall dnat-1000
nft add rule firewall dnat-all ct zone vmap { 1 : jump dnat-1000 }
nft add rule firewall dnat-1000 ip daddr 2.2.2.11 counter dnat to 10.0.0.7
nft add chain firewall snat-all { type nat hook postrouting priority 100 \; }
nft add chain firewall snat-1000
nft add rule firewall snat-all ct zone vmap { 1 : jump snat-1000 }
nft add rule firewall snat-1000 ip saddr 10.0.0.7 counter snat to 2.2.2.11
nft add flowtable firewall fb1 { hook ingress priority 0 \; devices = { br0 } \; }
nft add chain firewall ftb-all {type filter hook forward priority 0 \; policy accept \; }
nft add rule firewall ftb-all ct zone 1 ip protocol tcp flow offload @fb1
nft add rule firewall ftb-all ct zone 1 ip protocol udp flow offload @fb1






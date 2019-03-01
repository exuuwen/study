nft delete table firewall
ip netns del ns1 
ip netns del ns2
ip netns del cl
ip l del dev tun1 
ip l del dev tun2
ip l del dev user1 
ip l del dev user2 
ip l del dev br0 

sleep 2

ip netns add ns1
ip netns add ns2
ip netns add cl
ip l add dev veth1 type veth peer name eth0 netns ns1
ip l add dev veth2 type veth peer name eth0 netns ns2
ip l add dev vethc type veth peer name eth0 netns cl
ip netns exec cl ifconfig eth0 172.168.0.7/24 up
ip netns exec ns1 ifconfig eth0 172.168.0.11/24 up
ip netns exec ns2 ifconfig eth0 172.168.0.12/24 up
ifconfig veth1 up
ifconfig veth2 up
ifconfig vethc up
brctl addbr br0
ifconfig br0 up
brctl addif br0 veth1
brctl addif br0 veth2
brctl addif br0 vethc
ifconfig br0 172.168.0.1/24 up
ip netns exec cl ip l add dev tun type gretap external
ip netns exec cl ip l set dev tun address 7a:0e:f8:b6:dc:e2
ip netns exec ns1 ip l add dev tun type gretap local 172.168.0.11 remote 172.168.0.1 key 1000
ip netns exec ns2 ip l add dev tun type gretap local 172.168.0.12 remote 172.168.0.1 key 2000
ip l add dev tun1 type gretap key 1000
ip l set dev tun1 address 16:ff:9b:1c:ae:04
ip l add dev tun2 type gretap key 2000
ip l set dev tun2 address fa:4f:d7:c5:91:52
ip netns exec cl ifconfig tun 1.1.1.7/24 up
ip netns exec ns1 ifconfig tun 10.0.0.7/24 up
ip netns exec ns2 ifconfig tun 10.0.0.7/24 up
ip netns exec cl ip r r 2.2.2.11 via 1.1.1.11 dev tun encap ip id 1000 dst 172.168.0.1 key
ip netns exec cl ip r r 2.2.2.12 via 1.1.1.12 dev tun encap ip id 2000 dst 172.168.0.1 key
ip netns exec ns1 ip r a default via 10.0.0.1
ip netns exec ns2 ip r a default via 10.0.0.1
ip netns exec ns1 ip n a 10.0.0.1 lladdr 16:ff:9b:1c:ae:04 dev tun
ip netns exec ns2 ip n a 10.0.0.1 lladdr fa:4f:d7:c5:91:52 dev tun
ip netns exec cl ip n a 1.1.1.11 lladdr 16:ff:9b:1c:ae:04 dev tun
ip netns exec cl ip n a 1.1.1.12 lladdr fa:4f:d7:c5:91:52 dev tun
ip link add user1 type vrf table 1
ip link add user2 type vrf table 2
ip l set user1 up
ip l set user2 up
ip l set dev tun1 master user1
ip l set dev tun2 master user2
ifconfig tun1 up
ifconfig tun2 up
ip r a 10.0.0.7 encap ip id 1000 dst 172.168.0.11 key dev tun1 table 1
ip r a 10.0.0.7 encap ip id 2000 dst 172.168.0.12 key dev tun2 table 2
ip r a default via 7.7.7.7 encap ip id 1000 dst 172.168.0.7 key dev tun1 table 1 onlink
ip r a default via 7.7.7.7 encap ip id 2000 dst 172.168.0.7 key dev tun2 table 2 onlink
ip n add 7.7.7.7 lladdr 7a:0e:f8:b6:dc:e2 dev tun1 
ip n add 7.7.7.7 lladdr 7a:0e:f8:b6:dc:e2 dev tun2 

nft add table firewall
nft add chain firewall zones { type filter hook prerouting priority - 300 \; }
nft add rule firewall zones counter ct zone set iif map { "tun1" : 1, "tun2" : 2 }
nft add chain firewall rule-1000-ingress
nft add rule firewall rule-1000-ingress iif "tun1" ct state established,related counter accept
nft add rule firewall rule-1000-ingress iif "tun1" ct state invalid counter drop
nft add rule firewall rule-1000-ingress iif "tun1" tcp dport 22 ct state new counter accept
nft add rule firewall rule-1000-ingress iif "tun1" ip protocol icmp ct state new counter accept
nft add rule firewall rule-1000-ingress iif "tun1" counter drop
nft add chain firewall rule-2000-ingress
nft add rule firewall rule-2000-ingress iif "tun2" ct state established,related counter accept
nft add rule firewall rule-2000-ingress iif "tun2" ct state invalid counter drop
nft add rule firewall rule-2000-ingress iif "tun2" tcp dport 23 ct state new counter accept
nft add rule firewall rule-2000-ingress iif "tun2" ip protocol icmp ct state new counter accept
nft add rule firewall rule-2000-ingress iif "tun2" counter drop
nft add chain firewall rules-all { type filter hook prerouting priority - 150 \; }
nft add rule firewall rules-all ip daddr vmap { "2.2.2.11" : jump rule-1000-ingress, "2.2.2.12" : jump rule-2000-ingress } 
nft add chain firewall rule-1000-egress
nft add rule firewall rule-1000-egress ct state established,related counter accept
nft add rule firewall rule-1000-egress ct state invalid counter drop
nft add rule firewall rule-1000-egress tcp dport 22 ct state new counter drop
nft add rule firewall rule-1000-egress counter accept
nft add chain firewall rule-2000-egress
nft add rule firewall rule-2000-egress ct state established,related counter accept
nft add rule firewall rule-2000-egress ct state invalid counter drop
nft add rule firewall rule-2000-egress tcp dport 23 ct state new counter drop
nft add rule firewall rule-2000-egress counter accept
nft add rule firewall rules-all iif vmap { "tun1" : jump rule-1000-egress, "tun2" : jump rule-2000-egress } 
nft add chain firewall dnat-all { type nat hook prerouting priority - 100 \; }
nft add chain firewall dnat-1000
nft add chain firewall dnat-2000
nft add rule firewall dnat-all iif vmap { "tun1" : jump dnat-1000, "tun2" : jump dnat-2000 }
nft add rule firewall dnat-1000 ip daddr 2.2.2.11 counter dnat to 10.0.0.7
nft add rule firewall dnat-2000 ip daddr 2.2.2.12 counter dnat to 10.0.0.7
nft add chain firewall snat-all { type nat hook postrouting priority 100 \; }
nft add chain firewall snat-1000
nft add chain firewall snat-2000
nft add rule firewall snat-all oif vmap { "tun1" : jump snat-1000, "tun2" : jump snat-2000 }
nft add rule firewall snat-1000 ip saddr 10.0.0.7 counter snat to 2.2.2.11
nft add rule firewall snat-2000 ip saddr 10.0.0.7 counter snat to 2.2.2.12







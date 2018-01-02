1. tcpdump -i veth-oxxx
2. ovs-ofctl dump-flows br0 | grep xxxxx

3. ip:10.11.12.13
iptables -t raw -A PREROUTING -p 47 -m u32 --u32 "58 = 0xa0b0c0d"
iptables -A INPUT -p 47 -m u32 --u32 "58 = 0xa0b0c0d"



1. 
tcpdump -p net2 -nnn -p 100

2. 
ifconfig net2 promisc

3.
ifconfig net2 -promisc
tcpdump net2 -nnn -p 100



tcpdump -i net2 -nnn proto gre and ip[58:4]=0xa0b0c0d

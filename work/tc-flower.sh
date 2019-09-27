tc qdisc add dev veth1 ingress
tc filter add dev veth1 parent ffff: protocol ip flower skip_hw ip_proto icmp action pedit ex munge eth dst set fa:fa:fb:fb:fb:fb pipe action mirred egress mirror dev veth1 action pedit ex munge eth dst set fa:fc:fb:fb:fb:fb pipe action mirred egress redirect dev veth1
tc filter del dev veth1 parent ffff: protocol ip pref 49152

tc -s filter ls dev veth1 ingress
filter protocol ip pref 49152 flower chain 0 
filter protocol ip pref 49152 flower chain 0 handle 0x1 
  eth_type ipv4
  ip_proto icmp
  skip_hw
  not_in_hw
	action order 1:  pedit action pipe keys 2
 	 index 1 ref 1 bind 1 installed 725 sec used 0 sec
	 key #0  at eth+0: val fafafbfb mask 00000000
	 key #1  at eth+4: val fbfb0000 mask 0000ffff
 	Action statistics:
	Sent 55440 bytes 660 pkt (dropped 0, overlimits 0 requeues 0) 
	backlog 0b 0p requeues 0

	action order 2: mirred (Egress Mirror to device veth1) pipe
 	index 1 ref 1 bind 1 installed 725 sec used 0 sec
 	Action statistics:
	Sent 55440 bytes 660 pkt (dropped 0, overlimits 0 requeues 0) 
	backlog 0b 0p requeues 0

	action order 3:  pedit action pipe keys 2
 	 index 2 ref 1 bind 1 installed 725 sec used 0 sec
	 key #0  at eth+0: val fafcfbfb mask 00000000
	 key #1  at eth+4: val fbfb0000 mask 0000ffff
 	Action statistics:
	Sent 55440 bytes 660 pkt (dropped 0, overlimits 0 requeues 0) 
	backlog 0b 0p requeues 0

	action order 4: mirred (Egress Redirect to device veth1) stolen
 	index 2 ref 1 bind 1 installed 725 sec used 0 sec
 	Action statistics:
	Sent 55440 bytes 660 pkt (dropped 0, overlimits 0 requeues 0) 
	backlog 0b 0p requeues 0


tc filter add dev gre_sys prio 3 parent ffff: handle 995 protocol ip flower skip_sw enc_dst_ip 172.168.152.75 enc_src_ip 172.168.152.241 enc_key_id 995 enc_dst_port 0 ip_flag nofrag action tunnel_key unset pipe action mirred egress redirect dev mlx_pf0vf0 

tc filter del dev gre_sys prio 3 parent ffff: protocol ip handle 995 flower

find del_* -print | xargs -n 1 -P 10 tc -b


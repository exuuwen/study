tc qdisc del dev mlx_0 ingress
tc qdisc del dev gre_sys ingress

tc qdisc add dev mlx_0 ingress
tc qdisc add dev gre_sys ingress



tc filter add dev gre_sys ingress pref 2 chain 0 proto ip flower enc_dst_ip 172.168.152.75 enc_src_ip 172.168.152.208 enc_key_id 1000 enc_tos 0x0/ff dst_ip 1.1.1.7 ct_state -trk action ct zone 1 nat pipe action goto chain 2
tc filter add dev gre_sys ingress pref 2 chain 2 proto ip flower ip_proto tcp dst_port 5001 enc_dst_ip 172.168.152.75 enc_src_ip 172.168.152.208 enc_key_id 1000 enc_tos 0x0/ff ct_state +trk+new ct_zone 1 action ct zone 1 commit nat dst addr 10.0.0.75 pipe action tunnel_key unset pipe action mirred egress redirect dev mlx_0 
tc filter add dev gre_sys ingress pref 2 chain 2 proto ip flower enc_dst_ip 172.168.152.75 enc_src_ip 172.168.152.208 enc_key_id 1000 enc_tos 0x0/ff ct_state +trk+est ct_zone 1 action tunnel_key unset pipe action pedit ex munge eth dst set 52:54:00:00:12:75 pipe action pedit ex munge eth src set 50:6b:4b:39:d0:d2 pipe action mirred egress redirect dev mlx_0 


tc filter add dev mlx_0 ingress pref 2 chain 0 proto ip flower dst_ip 11.0.0.7 ct_state -trk action ct zone 1 nat pipe action goto chain 2
tc filter add dev mlx_0 ingress pref 2 chain 2 proto ip flower ct_state +trk+est ct_zone 1 action pedit ex munge eth dst set 52:54:00:00:12:75 pipe action pedit ex munge eth src set 50:6b:4b:39:d0:d2 pipe action tunnel_key set dst_ip 172.168.152.208 src_ip 172.168.152.75 id 1000 nocsum pipe action mirred egress redirect dev gre_sys

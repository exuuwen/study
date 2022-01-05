 ovs-vsctl add-port br0 vxlan -- set in vxlan type=vxlan options:remote_ip=192.168.153.234 options:key=23400
 ovs-vsctl add-port br0 gre -- set in gre type=gre options:remote_ip=172.168.0.7 options:key=flow

ovs-ofctl add-flow br0 in_port=gre,tun_id=0x12345678,ip,nw_dst=10.0.0.0/24,actions=push_mpls:0x8847,set_mpls_label:284280,,push_mpls:0x8847,set_mpls_label:291,output:vxlan
ovs-ofctl add-flow br0 "mpls,in_port=vxlan,mpls_bos=0 actions=move:OXM_OF_MPLS_LABEL[0..11]->NXM_NX_TUN_ID[20..31],pop_mpls:0x8847,resubmit(,1)"
ovs-ofctl add-flow br0 "table=1,mpls,in_port=vxlan,mpls_bos=1 actions=move:OXM_OF_MPLS_LABEL[0..19]->NXM_NX_TUN_ID[0..19],pop_mpls:0x0800,resubmit(,2)"
ovs-ofctl add-flow br0 "table=2,ip,in_port=vxlan,nw_dst=10.0.1.7,tun_id=0x12345678,actions=mod_dl_dst:86:96:03:00:52:99,mod_dl_src:fa:ff:ff:ff:ff:ff,output:gre"


ip netns add ns-pub
ip l add dev vethc type veth peer name eth0 netns ns-pub
ifconfig vethc 172.168.0.1/24 up

ip netns exec ns-pub ifconfig eth0 172.168.0.7/24 up
ip netns exec ns-pub ip l add dev tun type gretap key 0x12345678
ip netns exec ns-pub ip l add dev tun type gretap key 0x12345678 remote 172.168.0.1
ip netns exec ns-pub ifconfig tun 10.0.1.7/24 up
ip netns exec ns-pub ip r a default via 10.0.1.1
ip netns exec ns-pub ip n a 10.0.1.1 dev tun lladdr fa:ff:ff:ff:ff:ff


# tc filter add dev pf0vf0 prio 1 ingress protocol ip flower skip_hw indev pf0vf0 action mpls push protocol 0x8847 label 1000 bos 1 pipe action mpls push protocol 0x8847 label 2000 bos 0 pipe action mirred egress redirect dev pf0vf1
# tc filter add dev eth1 prio 1 ingress protocol 0x8847 flower skip_hw indev eth1 mpls_bos 0 mpls_label 2000 action mpls pop protocol 0x8847 pipe goto chain 1
# tc filter add dev eth1 prio 1 ingress protocol 0x8847 flower skip_hw chain 1 indev eth1 mpls_bos 1 mpls_label 1000 action mpls pop protocol 0x0800 pass

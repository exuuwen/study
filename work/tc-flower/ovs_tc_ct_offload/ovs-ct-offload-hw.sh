ovs-ofctl del-flows br0

ovs-ofctl add-flow br0 "ip,in_port=64200,nw_dst=1.1.1.7 actions=ct(table=1,zone=1,nat)"
ovs-ofctl add-flow br0 "table=1,ct_state=+new+trk,ct_zone=1,ip,icmp,actions=ct(commit,zone=1,nat(dst=10.0.0.75)),set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:52:54:00:00:12:75->dl_dst,output:1"
ovs-ofctl add-flow br0 "table=1,ct_state=+new+trk,ct_zone=1,ip,tcp,tp_dst=5001,actions=ct(commit,zone=1,nat(dst=10.0.0.75)),set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:52:54:00:00:12:75->dl_dst,output:1"
ovs-ofctl add-flow br0 "table=1,ct_state=+est+trk,ct_zone=1,ip,actions=set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:52:54:00:00:12:75->dl_dst,output:1"

ovs-ofctl add-flow br0 "ip,in_port=1,nw_dst=11.0.0.7 actions=ct(table=2,zone=1,nat)"
ovs-ofctl add-flow br0 "table=2,priority=65000,ct_state=+new+trk,ct_zone=1,ip,tcp,tp_dst=17,actions=drop"
ovs-ofctl add-flow br0 "table=2,ct_state=+new+trk,ct_zone=1,ip,actions=ct(commit,zone=1,nat(src=1.1.1.7)),set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:50:6b:4b:39:d0:d2->dl_dst,output:64200"
ovs-ofctl add-flow br0 "table=2,ct_state=+est+trk,ip,actions=set_field:fa:ff:ff:ff:ff:ff->dl_src,set_field:50:6b:4b:39:d0:d2->dl_dst,output:64200"

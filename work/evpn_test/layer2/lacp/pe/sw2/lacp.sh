net add interface swp31 ip address 192.168.70.70/24
net add loop lo ip address 10.0.0.70/32
net add bond peerlink bond slaves swp1
net add loopback lo clag vxlan-anycast-ip 7.7.7.7
net add interface peerlink.4094 clag peer-ip 169.254.1.69
net add interface peerlink.4094 clag sys-mac 44:38:39:FF:40:94
net add interface peerlink.4094 ip address 169.254.1.70/24
net add clag port bond bond1 interface swp11 clag-id 1
net add clag port bond bond2 interface swp21 clag-id 2

net add vxlan vni-104001 vxlan id 104001
net add vxlan vni-100100 vxlan id 100100
net add vxlan vni-100200 vxlan id 100200
net add bridge bridge ports vni-104001,vni-100100,vni-100200,bond1,bond2,peerlink
net add bridge bridge pvid 1
net add bridge bridge vids 100,200,4001
net add bridge bridge vlan-aware
net add bond bond1 bridge access 100
net add bond bond2 bridge access 200
net add vrf vrf1 vrf-table 1001
net add vlan 100 vrf vrf1
net add vlan 100 hwaddress 00:00:5e:00:01:01
net add vlan 100 ip address 10.0.1.1/24
net add vlan 200 vrf vrf1
net add vlan 200 hwaddress 00:00:5e:00:01:01
net add vlan 200 ip address 10.0.2.1/24
net add vlan 4001 vrf vrf1
net add vlan 4001 hwaddress 44:39:39:FF:40:94
net add vxlan vni-100100 bridge access 100
net add vxlan vni-100100 vxlan local-tunnelip 10.0.0.70
net add vxlan vni-100100 bridge arp-nd-suppress on
net add vxlan vni-100100 bridge learning off
net add vxlan vni-100100 stp bpduguard
net add vxlan vni-100100 stp portbpdufilter
net add vxlan vni-100200 bridge access 200
net add vxlan vni-100200 vxlan local-tunnelip 10.0.0.70
net add vxlan vni-100200 bridge arp-nd-suppress on
net add vxlan vni-100200 bridge learning off
net add vxlan vni-100200 stp bpduguard
net add vxlan vni-100200 stp portbpdufilter
net add vxlan vni-104001 bridge access 4001
net add vxlan vni-104001 vxlan local-tunnelip 10.0.0.70
net add vxlan vni-104001 bridge arp-nd-suppress on
net add vxlan vni-104001 bridge learning off
net add vxlan vni-104001 stp bpduguard
net add vxlan vni-104001 stp portbpdufilter
net pending
net commit

net del bond peerlink
net del interface swp31 ip address 192.168.70.70/24
net del loopback lo clag vxlan-anycast-ip 7.7.7.7
net del loop lo ip address 10.0.0.70/32
net del bond bond1
net del bond bond2
net del vrf vrf1
net del vlan 100
net del vlan 200
net del vlan 4001
net del vxlan vni-104001
net del vxlan vni-100100
net del vxlan vni-100200
net del bridge bridge
net pending
net commit



net add bgp autonomous-system 65070
net add vrf vrf1 vni 104001
net add bgp router-id 10.0.0.70
net add bgp bestpath as-path multipath-relax
net add bgp bestpath compare-routerid
net add bgp neighbor 192.168.70.73 remote-as 65073
net add bgp neighbor 169.254.1.69 remote-as 65069
net add bgp ipv4 unicast network 7.7.7.7/32 
net add bgp l2vpn evpn neighbor 192.168.70.73 activate
net add bgp l2vpn evpn neighbor 169.254.1.69 activate
net add bgp l2vpn evpn advertise-all-vni
net pending
net commit

net del bgp autonomous-system 65070
net del vrf vrf1 vni 104001
net pending
net commit


net add interface swp31 ip address 192.168.70.70/24
net add interface swp11 ip address 172.168.70.70/24
net add loop lo ip address 10.0.0.70/32
net add vxlan vni-104001 vxlan id 104001
net add bridge bridge ports vni-104001
net add bridge bridge pvid 1
net add bridge bridge vids 4001
net add bridge bridge vlan-aware
net add vrf vrf1 vrf-table 1001
net add vlan 4001 vrf vrf1
net add interface swp11 vrf vrf1
net add vxlan vni-104001 bridge access 4001
net add vxlan vni-104001 vxlan local-tunnelip 10.0.0.70
net add vxlan vni-104001 bridge arp-nd-suppress on
net add vxlan vni-104001 bridge learning off
net add vxlan vni-104001 stp bpduguard
net add vxlan vni-104001 stp portbpdufilter
net pending
net commit

net del interface swp31 ip address 192.168.70.70/24
net del interface swp11 ip address 172.168.70.70/24
net del loop lo ip address 10.0.0.70/32
net del vrf vrf1
net del vlan 4001
net del vxlan vni-104001
net del bridge bridge
net pending
net commit



net del bgp autonomous-system 65000
net del vrf vrf1 vni 104001
net del bgp vrf vrf1 autonomous-system 65000
net pending
net commit


net add bgp autonomous-system 65000
net add vrf vrf1 vni 104001
net add bgp router-id 10.0.0.70
net add bgp bestpath as-path multipath-relax
net add bgp bestpath compare-routerid
net add bgp neighbor 192.168.70.73 remote-as 65073
net add bgp ipv4 unicast network 10.0.0.70/32 
net add bgp l2vpn evpn neighbor 192.168.70.73 activate
net add bgp l2vpn evpn advertise-all-vni
net add bgp vrf vrf1 autonomous-system 65000
net add bgp vrf vrf1 neighbor 172.168.70.241 remote-as 65241
net add bgp vrf vrf1 ipv4 unicast neighbor 172.168.70.241 activate
net add bgp vrf vrf1 l2vpn evpn advertise ipv4 unicast
net pending
net commit

net add routing route 10.0.1.0/24 172.168.70.241 vrf vrf1
net add bgp vrf vrf1 ipv4 unicast network 10.0.1.0/24

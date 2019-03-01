vat# acl_add_replace ipv6 deny dst 2001:db8:1:2:3:4:a00:8/128,ipv6 permmit
vl_api_acl_add_replace_reply_t_handler:108: ACL index: 0
vat# 
vat# acl_interface_set_acl_list sw_if_index 2 input 0
vat# acl_dump
vl_api_acl_details_t_handler:215: acl_index: 0, count: 1
   tag {}
   ipv6 action 0 src ::/0 dst 2001:db8:1:2:3:4:a00:8/128 proto 0 sport 0-65535 dport 0-65535 tcpflags 0 mask 0


create host-interface name veth-v4
set interface ip address host-veth-v4 10.0.0.1/24
set interface state host-veth-v4 up

create host-interface name veth-v6
set interface ip address host-veth-v6 2001:db8:1:2:3:4:b00:1/112
set interface state host-veth-v6 up

set interface nat64 in host-veth-v6
set interface nat64 out host-veth-v4
nat64 add pool address 11.0.0.7
nat64 add prefix 2001:db8:1:2:3:4::/96

trace add af-packet-input 1
show trace
clear trace


vppctl create host-interface name vethf75e62b
#set interface ip address host-vethf75e62b 10.0.0.1/24
#set interface state host-vethf75e62b up

vppctl create host-interface name vethf3c2e70
#set interface ip address host-vethf3c2e70 2001:db8:1:2:3:4:b00:1/112
#set interface state host-vethf3c2e70 up

vppctl enable tap-inject
ifconfig vpp0 10.0.0.1/24 up
ifconfig vpp1 up
ip -6 addr add dev vpp1 2001:db8:1:2:3:5:a00:1/120

vppctl set interface nat64 in host-vethf3c2e70
vppctl set interface nat64 out host-vethf75e62b
vppctl nat64 add pool address 11.0.0.7
vppctl nat64 add prefix 2001:db8:1:2:3:4::/96

set interface nat64 in host-vethf3c2e70
set interface nat64 out host-vethf75e62b
nat64 add pool address 11.0.0.7
nat64 add prefix 2001:db8:1:2:3:4::/96



0:10:38:987586: af-packet-input
  af_packet: hw_if_index 2 next-index 4
    tpacket2_hdr:
      status 0x20000001 len 118 snaplen 118 mac 66 net 80
      sec 0x5b52bb1c nsec 0x263e7916 vlan 0 vlan_tpid 0
00:10:38:987604: ethernet-input
  IP6: a6:9e:56:54:88:96 -> 02:fe:0c:26:30:b2
00:10:38:987614: ip6-input
  ICMP6: 2001:db8:1:2:3:4:b00:7 -> 2001:db8:1:2:3:4:a00:8
    tos 0x00, flow label 0x0, hop limit 64, payload length 64
  ICMP echo_request checksum 0x6a87
00:10:38:987619: nat64-in2out
  NAT64-in2out: sw_if_index 2, next index 0
00:10:38:987630: ip4-lookup
  fib 0 dpo-idx 6 flow hash: 0x00000000
  ICMP: 11.0.0.7 -> 10.0.0.8
    tos 0x00, ttl 64, length 84, checksum 0x659b
    fragment id 0x0000
  ICMP echo_request checksum 0x530d
00:10:38:987636: ip4-rewrite
  tx_sw_if_index 1 dpo-idx 6 : ipv4 via 10.0.0.8 host-veth-v4: mtu:9000 c608a1a9516202fed47e776c0800 flow hash: 0x00000000
  00000000: c608a1a9516202fed47e776c080045000054000000003f01669b0b0000070a00
  00000020: 00080800530d4a33000c1cbb525b0000000023ca0900000000001011
00:10:38:987639: host-veth-v4-output
  host-veth-v4
  IP4: 02:fe:d4:7e:77:6c -> c6:08:a1:a9:51:62
  ICMP: 11.0.0.7 -> 10.0.0.8
    tos 0x00, ttl 63, length 84, checksum 0x669b
    fragment id 0x0000
  ICMP echo_request checksum 0x530d





00:08:31:459934: af-packet-input
  af_packet: hw_if_index 2 next-index 4
    tpacket2_hdr:
      status 0x20000001 len 118 snaplen 118 mac 66 net 80
      sec 0x5b52ba9d nsec 0x1003012a vlan 0 vlan_tpid 0
00:08:31:459955: ethernet-input
  IP6: a6:9e:56:54:88:96 -> 02:fe:0c:26:30:b2
00:08:31:459965: ip6-input
  ICMP6: 2001:db8:1:2:3:4:b00:7 -> 2001:db8:1:2:3:4:a00:7
    tos 0x00, flow label 0x0, hop limit 64, payload length 64
  ICMP echo_request checksum 0xef40
00:08:31:459970: acl-plugin-in-ip6-fa
  acl-plugin: lc_index: 0, sw_if_index 2, next index 1, action: 1, match: acl 0 rule 1 trace_bits 00000000
  pkt info 02000100b80d0120 0700000b04000300 02000100b80d0120 0700000a04000300 0002033a00000080 0a00ffff00000000
   lc_index 0 (lsb16 of sw_if_index 2) l3 ip6 2001:db8:1:2:3:4:b00:7 -> 2001:db8:1:2:3:4:a00:7 l4 proto 58 l4_valid 1 port 128 -> 0 tcp flags (invalid) 00 rsvd 0
00:08:31:459978: ip6-lookup
  fib 0 dpo-idx 1 flow hash: 0x00000000
  ICMP6: 2001:db8:1:2:3:4:b00:7 -> 2001:db8:1:2:3:4:a00:7
    tos 0x00, flow label 0x0, hop limit 64, payload length 64
  ICMP echo_request checksum 0xef40
00:08:31:459983: ip6-drop
    ICMP6: 2001:db8:1:2:3:4:b00:7 -> 2001:db8:1:2:3:4:a00:7
      tos 0x00, flow label 0x0, hop limit 64, payload length 64
    ICMP echo_request checksum 0xef40
00:08:31:459985: error-drop
  ethernet-input: no error




00:29:06:865026: af-packet-input
  af_packet: hw_if_index 2 next-index 4
    tpacket2_hdr:
      status 0x20000001 len 118 snaplen 118 mac 66 net 80
      sec 0x5b52bf6e nsec 0xabf55c4 vlan 0 vlan_tpid 0
00:29:06:865047: ethernet-input
  IP6: a6:9e:56:54:88:96 -> 02:fe:0c:26:30:b2
00:29:06:865066: ip6-input
  ICMP6: 2001:db8:1:2:3:4:b00:7 -> 2001:db8:1:2:3:4:b00:1
    tos 0x00, flow label 0x0, hop limit 64, payload length 64
  ICMP echo_request checksum 0x1e78
00:29:06:865075: nat64-in2out
  NAT64-in2out: sw_if_index 2, next index 1
00:29:06:865088: ip6-lookup
  fib 0 dpo-idx 6 flow hash: 0x00000000
  ICMP6: 2001:db8:1:2:3:4:b00:7 -> 2001:db8:1:2:3:4:b00:1
    tos 0x00, flow label 0x0, hop limit 64, payload length 64
  ICMP echo_request checksum 0x1e78
00:29:06:865095: ip6-local
    ICMP6: 2001:db8:1:2:3:4:b00:7 -> 2001:db8:1:2:3:4:b00:1
      tos 0x00, flow label 0x0, hop limit 64, payload length 64
    ICMP echo_request checksum 0x1e78
00:29:06:865103: ip6-icmp-input
  ICMP6: 2001:db8:1:2:3:4:b00:7 -> 2001:db8:1:2:3:4:b00:1
    tos 0x00, flow label 0x0, hop limit 64, payload length 64
  ICMP echo_request checksum 0x1e78
00:29:06:865105: ip6-icmp-echo-request
  ICMP6: 2001:db8:1:2:3:4:b00:7 -> 2001:db8:1:2:3:4:b00:1
    tos 0x00, flow label 0x0, hop limit 64, payload length 64
  ICMP echo_request checksum 0x1e78
00:29:06:865108: ip6-lookup
  fib 0 dpo-idx 4 flow hash: 0x00000000
  ICMP6: 2001:db8:1:2:3:4:b00:1 -> 2001:db8:1:2:3:4:b00:7
    tos 0x00, flow label 0x0, hop limit 64, payload length 64
  ICMP echo_reply checksum 0x1d78
00:29:06:865110: ip6-rewrite
  tx_sw_if_index 2 adj-idx 4 : ipv6 via 2001:db8:1:2:3:4:b00:7 host-veth-v6: mtu:9000 a69e5654889602fe0c2630b286dd flow hash: 0x00000000
  00000000: a69e5654889602fe0c2630b286dd6000000000403a3f20010db8000100020003


create host-interface name veth5f260b6
set interface ip address host-veth5f260b6 10.0.0.1/24
set interface state host-veth5f260b6 up


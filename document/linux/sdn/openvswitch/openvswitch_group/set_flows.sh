#!/bin/bash
ovs-vsctl set bridge s1 protocols=OpenFlow13
ovs-vsctl set bridge s21 protocols=OpenFlow13
ovs-vsctl set bridge s22 protocols=OpenFlow13
ovs-vsctl set bridge s23 protocols=OpenFlow13
ovs-vsctl set bridge s3 protocols=OpenFlow13

ovs-ofctl -O OpenFlow13 add-group s1 group_id=5566,type=select,bucket=output:1,bucket=output:2,bucket=output:3
ovs-ofctl -O OpenFlow13 add-flow s1 in_port=4,actions=group:5566
ovs-ofctl -O OpenFlow13 add-flow s1 in_port=5,actions=group:5566
ovs-ofctl -O OpenFlow13 add-flow s1 in_port=6,actions=group:5566

ovs-ofctl -O OpenFlow13 add-flow s1 ip,ip_dst=192.168.0.101,actions=output:4
ovs-ofctl -O OpenFlow13 add-flow s1 arp,ip_dst=192.168.0.101,actions=output:4
ovs-ofctl -O OpenFlow13 add-flow s1 ip,ip_dst=192.168.0.102,actions=output:5
ovs-ofctl -O OpenFlow13 add-flow s1 arp,ip_dst=192.168.0.102,actions=output:5
ovs-ofctl -O OpenFlow13 add-flow s1 ip,ip_dst=192.168.0.103,actions=output:6
ovs-ofctl -O OpenFlow13 add-flow s1 arp,ip_dst=192.168.0.103,actions=output:6

ovs-ofctl -O OpenFlow13 add-flow s21 in_port=1,actions=output:2
ovs-ofctl -O OpenFlow13 add-flow s21 in_port=2,actions=output:1
ovs-ofctl -O OpenFlow13 add-flow s22 in_port=1,actions=output:2
ovs-ofctl -O OpenFlow13 add-flow s22 in_port=2,actions=output:1
ovs-ofctl -O OpenFlow13 add-flow s23 in_port=1,actions=output:2
ovs-ofctl -O OpenFlow13 add-flow s23 in_port=2,actions=output:1


ovs-ofctl -O OpenFlow13 add-flow s3 ip,ip_dst=192.168.0.1,actions=output:4
ovs-ofctl -O OpenFlow13 add-flow s3 ip,ip_dst=192.168.0.2,actions=output:5
ovs-ofctl -O OpenFlow13 add-flow s3 ip,ip_dst=192.168.0.3,actions=output:6
ovs-ofctl -O OpenFlow13 add-flow s3 ip,ip_dst=192.168.0.4,actions=output:7
ovs-ofctl -O OpenFlow13 add-flow s3 ip,ip_dst=192.168.0.5,actions=output:8
ovs-ofctl -O OpenFlow13 add-flow s3 ip,ip_dst=192.168.0.6,actions=output:9

#ARP
ovs-ofctl -O OpenFlow13 add-flow s3 arp,ip_dst=192.168.0.1,actions=output:4
ovs-ofctl -O OpenFlow13 add-flow s3 arp,ip_dst=192.168.0.2,actions=output:5
ovs-ofctl -O OpenFlow13 add-flow s3 arp,ip_dst=192.168.0.3,actions=output:6
ovs-ofctl -O OpenFlow13 add-flow s3 arp,ip_dst=192.168.0.4,actions=output:7
ovs-ofctl -O OpenFlow13 add-flow s3 arp,ip_dst=192.168.0.5,actions=output:8
ovs-ofctl -O OpenFlow13 add-flow s3 arp,ip_dst=192.168.0.6,actions=output:9


ovs-ofctl -O OpenFlow13 add-flow s3 ip,ip_dst=192.168.0.101,actions=output:1
ovs-ofctl -O OpenFlow13 add-flow s3 arp,ip_dst=192.168.0.101,actions=output:1

ovs-ofctl -O OpenFlow13 add-flow s3 ip,ip_dst=192.168.0.102,actions=output:2
ovs-ofctl -O OpenFlow13 add-flow s3 arp,ip_dst=192.168.0.102,actions=output:2

ovs-ofctl -O OpenFlow13 add-flow s3 ip,ip_dst=192.168.0.103,actions=output:3
ovs-ofctl -O OpenFlow13 add-flow s3 arp,ip_dst=192.168.0.103,actions=output:3

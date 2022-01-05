#!/bin/bash

mst start
VFS=`mlxconfig -d /dev/mst/mt4121_pciconf0 q | grep NUM_OF_VFS | sed 's/^ *//' | sed 's/^NUM_OF_VFS *//'`
num=$((2*VFS-1))

ip l del dev vrf1000

tc qdisc del dev mlx_$num ingress
tc qdisc del dev bond2 ingress

echo 0 > /sys/class/net/net2/device/sriov_numvfs
echo 0 > /sys/class/net/net3/device/sriov_numvfs

#rmmod cls_flower mlx5_fpga_tools mlx5_ib
#rmmod mlx5_core

#sleep 10
#modprobe mlx5_core

sleep 5

echo $VFS > /sys/class/net/net2/device/sriov_numvfs
echo $VFS > /sys/class/net/net3/device/sriov_numvfs

pcie=`lspci -m | grep Ether |grep "Virtual Function" | cut -d " " -f 1 | tail -1`
pci_vf="0000:$pcie"

lspci -m | grep Ether |grep "Virtual Function" | cut -d " " -f 1  > "/tmp/net"
while read pcie
do
	pci="0000:$pcie"
	echo $pci > /sys/bus/pci/drivers/mlx5_core/unbind
done < "/tmp/net"

#TODO mst name
mcra /dev/mst/mt4121_pciconf0 0x31500.17 0
mcra /dev/mst/mt4121_pciconf0.1 0x31500.17 0
echo switchdev > /sys/class/net/net2/compat/devlink/mode
echo switchdev > /sys/class/net/net3/compat/devlink/mode

sleep 20

echo $pci_vf > /sys/bus/pci/drivers/mlx5_core/bind

sleep 2

ifconfig mlx_$num up

/opt/mellanox/iproute2/sbin/ip lin add dev vrf1000 type vrf table 1000
ip l set dev net5 master vrf1000
ifconfig vrf1000 up
ip r a default via 10.66.168.1 table 1000
ifup net5
#ifconfig eth0 10.66.168.7/25

#for i in {0..8}; do echo 0 > /sys/class/net/net5/queues/tx-0/xps_cpus; done

mac=`ip l lst dev net5 | grep 'link/ether' | sed 's/ *//' | cut -d " " -f 2`
tc qdisc add dev mlx_$num ingress
tc qdisc add dev bond2 ingress
tc filter add dev mlx_$num pref 1 ingress  protocol ip flower skip_sw action mirred egress redirect dev bond2
tc filter add dev mlx_$num pref 2 ingress  protocol arp flower skip_hw action mirred egress redirect dev bond2
tc filter add dev bond2 pref 1 ingress  protocol ip flower skip_sw dst_mac $mac action mirred egress redirect dev mlx_$num
tc filter add dev bond2 pref 2 ingress  protocol arp flower skip_hw dst_mac $mac action mirred egress redirect dev mlx_$num
tc filter add dev bond2 pref 3 ingress  protocol arp flower skip_hw dst_mac ff:ff:ff:ff:ff:ff arp_tip 172.168.153.0/24 action mirred egress redirect dev mlx_$num


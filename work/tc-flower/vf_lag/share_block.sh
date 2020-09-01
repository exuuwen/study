VFS=2
devlink dev eswitch set pci/0000:b3:00.0  mode legacy
devlink dev eswitch set pci/0000:b3:00.1  mode legacy

echo 0 > /sys/class/net/net2/device/sriov_numvfs
echo 0 > /sys/class/net/net3/device/sriov_numvfs

ip l del dev bond0

echo $VFS > /sys/class/net/net2/device/sriov_numvfs
echo $VFS > /sys/class/net/net3/device/sriov_numvfs

pcie=`lspci -m | grep Ether |grep "Virtual Function" | cut -d " " -f 1 | tail -1`
pci_vf="0000:$pcie"

lspci -m | grep Ether |grep "Virtual Function" | cut -d " " -f 1  > "/tmp/net"
while read pcie
do
        pci="0000:$pcie"
        #pci="$pcie"
        echo $pci > /sys/bus/pci/drivers/mlx5_core/unbind
done < "/tmp/net"

devlink dev eswitch set pci/0000:b3:00.0  mode switchdev encap enable
devlink dev eswitch set pci/0000:b3:00.1  mode switchdev encap enable

ip l add dev bond0 type bond mode 802.3ad

ifconfig net2 down
echo "+net2" > /sys/class/net/bond0/bonding/slaves
ifconfig net3 down
echo "+net3" > /sys/class/net/bond0/bonding/slaves

echo $pci_vf > /sys/bus/pci/drivers/mlx5_core/bind

ifconfig bond0 172.168.152.117/24 

ifconfig eth0 up
ifconfig pf1vf1 up
tc qdisc add dev pf1vf1 ingress


mac=`ip l lst dev eth0 | grep 'link/ether' | sed 's/ *//' | cut -d " " -f 2`
tc filter add dev pf1vf1 pref 1 ingress  protocol ip flower skip_sw action mirred egress redirect dev bond0
tc filter add dev pf1vf1 pref 2 ingress  protocol arp flower skip_sw action mirred egress redirect dev bond0

tc qdisc add dev bond0 ingress_block 22 ingress 
tc qdisc add dev net2 ingress_block 22 ingress 
tc qdisc add dev net3 ingress_block 22 ingress 

tc filter add block 22 pref 1 protocol ip flower dst_mac $mac action mirred egress redirect dev pf1vf1
tc filter add block 22 pref 2 protocol arp flower dst_mac $mac action mirred egress redirect dev pf1vf1
tc filter add block 22 pref 2 protocol arp flower dst_mac ff:ff:ff:ff:ff:ff arp_tip 172.168.153.0/24 action mirred egress redirect dev pf1vf1


ifconfig eth0 172.168.153.117/24


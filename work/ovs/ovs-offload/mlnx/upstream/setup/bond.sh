ip l del dev bond0
ip l del dev eth0
systemctl stop openvswitch
rmmod mlx5_ib
rmmod mlx5_core
rmmod bonding
ip lin add dev eth0 type veth peer name eth1
modprobe mlx5_core
echo 0 > /sys/class/net/eth2/device/sriov_numvfs
echo 0 > /sys/class/net/eth3/device/sriov_numvfs
echo 2 > /sys/class/net/eth2/device/sriov_numvfs
echo 2 > /sys/class/net/eth3/device/sriov_numvfs
lspci -nn | grep Mellanox
echo 0000:81:00.2 > /sys/bus/pci/drivers/mlx5_core/unbind
echo 0000:81:00.3 > /sys/bus/pci/drivers/mlx5_core/unbind
echo 0000:81:03.6 > /sys/bus/pci/drivers/mlx5_core/unbind
echo 0000:81:03.7 > /sys/bus/pci/drivers/mlx5_core/unbind
/etc/init.d/mst start
mcra /dev/mst/mt4119_pciconf0  0x31500.17  0
mcra /dev/mst/mt4119_pciconf0.1  0x31500.17  0

~/iproute2/devlink/devlink dev eswitch set pci/0000:81:00.0  mode switchdev encap enable
~/iproute2/devlink/devlink dev eswitch set pci/0000:81:00.1  mode switchdev encap enable

modprobe bonding mode=802.3ad miimon=100 lacp_rate=1
ip l del dev bond0
ifconfig mlx_p0 down
ifconfig mlx_p1 down
ip l add dev bond0 type bond mode 802.3ad
ifconfig bond0 172.168.152.75/24 up
echo 1 > /sys/class/net/bond0/bonding/xmit_hash_policy
ip l set dev mlx_p0 master bond0
ip l set dev mlx_p1 master bond0
ifconfig mlx_p0 up
ifconfig mlx_p1 up

systemctl start openvswitch
ovs-vsctl set Open_vSwitch . other_config:hw-offload=true
systemctl restart openvswitch

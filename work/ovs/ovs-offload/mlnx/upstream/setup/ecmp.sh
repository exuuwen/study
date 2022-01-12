ip l del dev eth0
systemctl stop openvswitch
rmmod mlx5_core
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

~/iproute2/devlink/devlink dev eswitch set pci/0000:81:00.0  mode switchdev encap enable
~/iproute2/devlink/devlink dev eswitch set pci/0000:81:00.1  mode switchdev encap enable

ifconfig mlx_p0 11.0.1.75/24 up
ifconfig mlx_p1 11.0.2.75/24 up
ip a a dev lo 172.168.152.75/32
ip r a 172.168.152.241 nexthop via 11.0.1.241 nexthop via 11.0.2.241
systemctl start openvswitch
ovs-vsctl set Open_vSwitch . other_config:hw-offload=true
systemctl restart openvswitch




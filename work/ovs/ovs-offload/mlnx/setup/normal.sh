echo 0 > /sys/class/net/net2/device/sriov_numvfs
ip l del dev gre_sys
systemctl stop openvswitch
sleep 1
rmmod mlx5_ib
rmmod cls_flower
rmmod mlx5_core
modprobe mlx5_core
sleep 2
echo 0 > /sys/class/net/net2/device/sriov_numvfs
echo 2 > /sys/class/net/net2/device/sriov_numvfs
lspci -nn | grep Mellanox
echo 0000:81:00.2 > /sys/bus/pci/drivers/mlx5_core/unbind
echo 0000:81:00.3 > /sys/bus/pci/drivers/mlx5_core/unbind
/etc/init.d/mst start
mcra /dev/mst/mt4119_pciconf0  0x31500.17  0

echo switchdev > /sys/class/net/net2/compat/devlink/mode
echo basic > /sys/class/net/net2/compat/devlink/encap
#mcra /dev/mst/mt4119_pciconf0 0x31480.28 0

ifconfig net2 172.168.152.75/24
ip l a dev gre_sys type gretap external
ifconfig gre_sys up
systemctl start openvswitch
ovs-vsctl set Open_vSwitch . other_config:hw-offload=true
systemctl restart openvswitch

#~/iproute2/devlink/devlink dev eswitch set pci/0000:04:00.0  mode switchdev inline-mode transport encap enable




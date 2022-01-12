systemctl stop openvswitch
rmmod mlx5_ib
rmmod mlx5_core
rmmod act_ct
rmmod nf_flow_table
modprobe mlx5_core
sleep 2
echo 0 > /sys/class/net/net2/device/sriov_numvfs
echo 2 > /sys/class/net/net2/device/sriov_numvfs
lspci -nn | grep Mellanox
echo 0000:81:00.2 > /sys/bus/pci/drivers/mlx5_core/unbind
echo 0000:81:00.3 > /sys/bus/pci/drivers/mlx5_core/unbind
/etc/init.d/mst start
mcra /dev/mst/mt4119_pciconf0  0x31500.17  0

~/iproute2/devlink/devlink dev eswitch set pci/0000:81:00.0  mode switchdev encap enable

ifconfig net2 172.168.152.75/24
#systemctl start openvswitch





#!/bin/bash

if [ $# -lt 1 ]; then
	echo "usage nic-name"
	exit;
fi

dev="$1"
echo 28 > /sys/class/net/$dev/device/sriov_numvfs

lspci | grep Ether |grep "Virtual Function" | cut -d " " -f 1  > "/tmp/$dev"
while read pcie
do
	pci="0000:$pcie"
	echo $pci > /sys/bus/pci/drivers/mlx5_core/unbind
done < "/tmp/$dev"

echo switchdev > /sys/class/net/$dev/compat/devlink/mode
echo basic > /sys/class/net/$dev/compat/devlink/encap
/etc/init.d/mst start
#TODO mst name
mcra /dev/mst/mt4119_pciconf0 0x31500.17 0
ovs-vsctl set Open_vSwitch . other_config:hw-offload=true







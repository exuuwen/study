#!/bin/bash

if [ $# -lt 1 ]; then
	echo "usage nic-name"
	exit;
fi

dev="$1"

phys_switch_id=$(cat /sys/class/net/eth2/phys_switch_id)

echo "SUBSYSTEM==\"net\", ACTION==\"add\", ATTR{phys_switch_id}==\""$phys_switch_id"\", \\
ATTR{phys_port_name}!=\"\", NAME=\"mlx_\$attr{phys_port_name}\"" >> /etc/udev/rules.d/82-net-setup-link.rulese

/etc/init.d/mst start
#TODO mst name
mlxconfig -d /dev/mst/mt4119_pciconf0 -y set NUM_OF_VFS=28 NUM_VF_MSIX=18




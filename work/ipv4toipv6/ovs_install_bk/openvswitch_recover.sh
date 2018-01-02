#!/bin/bash

cd `dirname ${BASH_SOURCE[0]}`

module_file=$(modinfo openvswitch | grep 'filename' | awk '{print $2}')
module_dir="${module_file%/*}/"

if [[ -z $module_dir ]]; then
    exit 1
fi

install $module_dir/openvswitch.ko.bak $module_dir/openvswitch.ko

/etc/init.d/openvswitch force-reload-kmod

ip link set dev br0 up
ovs-vsctl set bridge br0 protocols=OpenFlow10,OpenFlow13
sh .br0_flow.sh
ovs-vsctl set bridge br0 protocols=OpenFlow13

if ovs-vsctl br-exists manbr; then
    ip link set dev manbr up
    ovs-vsctl set bridge manbr protocols=OpenFlow10,OpenFlow13
    sh .manbr_flow.sh
    ovs-vsctl set bridge manbr protocols=OpenFlow13
fi
if ovs-vsctl br-exists wanbr; then
    ip link set dev wanbr up
    ovs-vsctl set bridge wanbr protocols=OpenFlow10,OpenFlow13
    sh .wanbr_flow.sh
    ovs-vsctl set bridge wanbr protocols=OpenFlow13
fi

if [[ -f .interface.sh ]]; then
    sh .interface.sh
fi

if [[ -f .pbr.sh ]]; then
    sh .pbr.sh
fi

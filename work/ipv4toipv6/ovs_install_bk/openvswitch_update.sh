#!/bin/bash

cd `dirname ${BASH_SOURCE[0]}`

. functions

> .interface.sh
> .manbr_flow.sh
> .br0_flow.sh
> .wanbr_flow.sh

module_file=$(modinfo openvswitch | grep 'filename' | awk '{print $2}')
module_dir="${module_file%/*}/"

if [[ -z $module_dir ]]; then
    exit 1
fi

cp $module_dir/openvswitch.ko $module_dir/openvswitch.ko.bak
install ./openvswitch.ko $module_dir/openvswitch.ko

# ovs-2.3.0 need
ovs-ofctl add-flow br0 -Oopenflow13 priority=0,actions=CONTROLLER:65535

ovs-vsctl set bridge br0 protocols=OpenFlow10,OpenFlow13
/usr/share/openvswitch/scripts/ovs-save save-flows br0 > .br0_flow.sh

if ovs-vsctl br-exists manbr; then
    ovs-vsctl set bridge manbr protocols=OpenFlow10,OpenFlow13
    /usr/share/openvswitch/scripts/ovs-save save-flows manbr > .manbr_flow.sh
fi
if ovs-vsctl br-exists wanbr; then
    ovs-vsctl set bridge wanbr protocols=OpenFlow10,OpenFlow13
    /usr/share/openvswitch/scripts/ovs-save save-flows wanbr > .wanbr_flow.sh
fi

gw=$(ip ro get 8.8.8.8 | grep 8.8.8.8 | awk '{print $3}')
num_of_flows_before=$(ovs-ofctl dump-tables br0 -Oopenflow13 | grep ' 0:' | grep -Eo 'active=[^,]+' | cut -d= -f2)

#save interfaces
ifaces=`internal_interfaces`
/usr/share/openvswitch/scripts/ovs-save save-interfaces $ifaces > .interface.sh

#save conf.db
\cp -f /etc/openvswitch/conf.db .

#save pbr
pbrfile=.pbr.sh
rm -f $pbrfile
for tbname in `ip rule | grep UCLOUD | awk '{print $NF}'`; do
    nic_name=$(echo $tbname | egrep -Eo '[0-9.]+')
    if [[ $nic_name == 172.* ]]; then
	gateway=$(ip ro | grep 'default' | awk '{print $3}')
    else
	gateway=$(echo $nic_name | cut -d'.' -f1,2)".0.1"
    fi
    trick_ip="${gateway%.*}.254"/16
    echo "#$nic_name" >> $pbrfile
    echo "ip addr add dev $nic_name $trick_ip" >> $pbrfile
    ip ro show table $tbname | sed 's/^/ip ro add /' | sed "s/$/ table $tbname/" >> $pbrfile
    echo "ip addr del dev $nic_name $trick_ip" >> $pbrfile
done

if ovs-vsctl --version | head -n 1 | grep -Eq '(2.0.0|2.1.2)'; then
    rpm -Uvh openvswitch-2.3.0-1.x86_64.rpm --nodeps --force
fi

/etc/init.d/openvswitch force-reload-kmod

#restore pbr
if [[ -f .pbr.sh ]]; then
    sh $pbrfile
fi

#check and recover if necessary
if [[ -z $gw ]]; then
    exit 1
fi

ping -n -W 1 -c 5 $gw >.ping
ret=$?
if [[ $ret -ne 0 ]] || grep -q '100% packet loss' .ping; then
    sh openvswitch_recover.sh
    exit 3
fi

num_of_flows_now=$(ovs-ofctl dump-tables br0 -Oopenflow13 | grep ' 0:' | grep -Eo 'active=[^,]+' | cut -d= -f2)
if [[ $num_of_flows_now -lt $num_of_flows_before ]]; then
    echo "flow number not match"
    #exit 4
fi
#
ovs-vsctl set bridge br0 protocols=OpenFlow13
if ovs-vsctl br-exists manbr; then
    ovs-vsctl set bridge manbr protocols=OpenFlow13
fi
if ovs-vsctl br-exists wanbr; then
    ovs-vsctl set bridge wanbr protocols=OpenFlow13
fi

#echo 1 > /proc/net/gso_size_enable
#if ! grep -q 'gso_size_enable' /etc/rc.d/rc.local; then
#    echo "echo 1 > /proc/net/gso_size_enable" >> /etc/rc.d/rc.local
#fi
if ! grep -iq 'priority=0.*CONTROLLER' /etc/rc.d/rc.local; then
    echo "ovs-ofctl add-flow br0 -Oopenflow13 priority=0,actions=CONTROLLER:65535" >> /etc/rc.d/rc.local
fi

ovs-ofctl add-flow br0 -Oopenflow13 priority=0,actions=CONTROLLER:65535
ovs-ofctl add-flow br0 -Oopenflow13 "priority=60040,arp,dl_dst=fa:ff:ff:ff:ff:ff,arp_tpa=10.23.0.1,actions=move:NXM_OF_ETH_SRC[]->NXM_OF_ETH_DST[],set_field:fa:ff:ff:ff:ff:ff->eth_src,load:0x2->NXM_OF_ARP_OP[],move:NXM_NX_ARP_SHA[]->NXM_NX_ARP_THA[],move:NXM_OF_ARP_SPA[]->NXM_OF_ARP_TPA[],set_field:fa:ff:ff:ff:ff:ff->arp_sha,set_field:10.23.0.1->arp_spa,IN_PORT"
ovs-ofctl add-flow br0 -Oopenflow13 "priority=60040,arp,dl_dst=ff:ff:ff:ff:ff:ff,arp_tpa=10.23.0.1,actions=move:NXM_OF_ETH_SRC[]->NXM_OF_ETH_DST[],set_field:fa:ff:ff:ff:ff:ff->eth_src,load:0x2->NXM_OF_ARP_OP[],move:NXM_NX_ARP_SHA[]->NXM_NX_ARP_THA[],move:NXM_OF_ARP_SPA[]->NXM_OF_ARP_TPA[],set_field:fa:ff:ff:ff:ff:ff->arp_sha,set_field:10.23.0.1->arp_spa,IN_PORT"

if ! grep -q '10.23.0.1' /etc/rc.d/rc.local; then
    echo 'ovs-ofctl add-flow br0 -Oopenflow13 "priority=60040,arp,dl_dst=fa:ff:ff:ff:ff:ff,arp_tpa=10.23.0.1,actions=move:NXM_OF_ETH_SRC[]->NXM_OF_ETH_DST[],set_field:fa:ff:ff:ff:ff:ff->eth_src,load:0x2->NXM_OF_ARP_OP[],move:NXM_NX_ARP_SHA[]->NXM_NX_ARP_THA[],move:NXM_OF_ARP_SPA[]->NXM_OF_ARP_TPA[],set_field:fa:ff:ff:ff:ff:ff->arp_sha,set_field:10.23.0.1->arp_spa,IN_PORT"' >> /etc/rc.d/rc.local
    echo 'ovs-ofctl add-flow br0 -Oopenflow13 "priority=60040,arp,dl_dst=ff:ff:ff:ff:ff:ff,arp_tpa=10.23.0.1,actions=move:NXM_OF_ETH_SRC[]->NXM_OF_ETH_DST[],set_field:fa:ff:ff:ff:ff:ff->eth_src,load:0x2->NXM_OF_ARP_OP[],move:NXM_NX_ARP_SHA[]->NXM_NX_ARP_THA[],move:NXM_OF_ARP_SPA[]->NXM_OF_ARP_TPA[],set_field:fa:ff:ff:ff:ff:ff->arp_sha,set_field:10.23.0.1->arp_spa,IN_PORT"' >> /etc/rc.d/rc.local
fi
exit 0

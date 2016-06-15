#!/bin/bash

if [ ! -e /lib/modules/`uname -r`/extra/openvswitch ]
then
        mkdir -p  /lib/modules/`uname -r`/kernel/net/openvswitch
fi


install openvswitch.ko /lib/modules/`uname -r`/kernel/net/openvswitch/
depmod -a

# if the bridge not support OpenFLow10
ovs-vsctl set bridge br0 protocols=OpenFlow10,OpenFlow13

rpm -Uvh openvswitch-2.3.0-1.x86_64.rpm --nodeps --force

# only support openflow10
/etc/init.d/openvswitch force-reload-kmod
#Detected internal interfaces: br0                          [  OK  ]
#Saving flows                                               [  OK  ]
#Killing ovsdb-server (21655)                               [  OK  ]
#Starting ovsdb-server                                      [  OK  ]
#Configuring Open vSwitch system IDs                        [  OK  ]
#Killing ovs-vswitchd (21712)                               [  OK  ]
#Saving interface configuration                             [  OK  ]
#Removing datapath: system@ovs-system                       [  OK  ]
#Removing openvswitch module                                [  OK  ]
#Inserting openvswitch module                               [  OK  ]
#Starting ovs-vswitchd                                      [  OK  ]
#Restoring saved flows                                      [  OK  ]
#Enabling remote OVSDB managers                             [  OK  ]
#Restoring interface configuration                          [  OK  ]

ovs-vsctl set bridge br0 protocols=OpenFlow13
echo "++++++++done+++++++!"

#!/bin/bash
cp actions.c ./openvswitch-2.6.0/datapath/actions.c
cp dpif.c ./openvswitch-2.6.0/lib/dpif.c
cp dpif-netdev.c ./openvswitch-2.6.0/lib/dpif-netdev.c
cp flow.h ./openvswitch-2.6.0/include/openvswitch/flow.h
cp flow_netlink.c ./openvswitch-2.6.0/datapath/flow_netlink.c
cp odp-execute.c ./openvswitch-2.6.0/lib/odp-execute.c
cp odp-util.c ./openvswitch-2.6.0/lib/odp-util.c
cp ofp-actions.c ./openvswitch-2.6.0/lib/ofp-actions.c
cp ofp-actions.h  ./openvswitch-2.6.0/include/openvswitch/ofp-actions.h
cp ofproto-dpif-sflow.c ./openvswitch-2.6.0/ofproto/ofproto-dpif-sflow.c
cp ofproto-dpif-xlate.c ./openvswitch-2.6.0/ofproto/ofproto-dpif-xlate.c
cp openvswitch.h ./openvswitch-2.6.0/datapath/linux/compat/include/linux/openvswitch.h

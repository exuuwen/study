#!/bin/sh
# pktgen.conf -- Sample configuration for send on two devices on a UP system

#modprobe pktgen

if [[ `lsmod | grep pktgen` == "" ]];then
   modprobe pktgen
fi

#/proc/net/pktgen
# kpktgend_0  kpktgend_1  kpktgend_2  kpktgend_3  pgctrl
# kpktgend_n: n is the number of the cores

if [[ $1 == "" ]];then
   pktsize=550
else
   pktsize=$1
fi

function pgset() {
    local result

    echo $1 > $PGDEV

    result=`cat $PGDEV | fgrep "Result: OK:"`
    if [ "$result" = "" ]; then
         cat $PGDEV | fgrep Result:
    fi
}


# On UP systems only one thread exists -- so just add devices
# We use eth1, eth1

echo "Adding devices to run".

PGDEV=/proc/net/pktgen/kpktgend_0
pgset "rem_device_all"
pgset "add_device eth0"
pgset "max_before_softirq 10000"

# Configure the individual devices
echo "Configuring devices"

PGDEV=/proc/net/pktgen/eth0

pgset "src_mac 6c:92:bf:04:7e:b0"
pgset "dst_mac 6c:92:bf:04:7f:de"
pgset "src_min 10.0.0.1"
pgset "src_max 10.0.0.1"
pgset "dst_min 10.0.0.2"
#pgset "dst_max 10.0.0.2"
pgset "dst_max 10.255.255.254"
pgset "udp_src_min  1024"
pgset "udp_src_max 65000"
pgset "udp_dst_min  1024"
pgset "udp_dst_max 65000"
#pgset "flag IPSRC_RND"
pgset "flag IPDST_RND"
pgset "flag UDPSRC_RND"
pgset "flag UDPDST_RND"
pgset "delay 0"
pgset "count 0"
pgset "pkt_size $pktsize"
pgset "ratep 90000"
pgset "count 0"

# Time to run

PGDEV=/proc/net/pktgen/pgctrl
echo "pkgsize:$pktsize"
echo "Running... ctrl^C to stop"

pgset "start"

echo "Done"

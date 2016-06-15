#!/bin/bash

if [ $# != 2 ]
then
	printf "%s nsname ip \n" $0
	exit
fi
nsname=$1
ip=$2

ip netns add $1 
ip link add name $1 type veth peer name eth0 netns $1
ip link set $1 up
ip netns exec $1 ip link set eth0 up
ip netns exec $1 ip addr add "$ip/24" dev eth0
ip netns exec $1 ip link set lo up
ip netns exec $1 bash


name="test"
state_ok="running"
vf_prefix="mlx_"

state=$(virsh domstate $name 2> /dev/null)
if [[ $state != $state_ok ]]; then
	echo "domain $name is not running"
	exit 1
fi

mac=$(virsh dumpxml $name | grep -m 1 'mac address' | cut -d\' -f2)
vmac=$(echo $mac | sed 's/^../fe/g')
if [ -z $vmac ]; then
	echo "mac is empty"
	exit 1
fi

vnet_name=$(ip l | grep -B 1 "link/ether $vmac" | cut -d " " -f 2 | sed 's/://g')
if [ -z $vnet_name ]; then
	echo "vnet_name is empty"
	exit 1
fi

vf_num=$(ip l | grep "$mac" | sed 's/^ *//g' | cut -d " " -f 2)
vf_name="$vf_prefix$vf_num"
ofport=$(ovs-vsctl --bare --columns=ofport list interface $vf_name)
if [ -z $ofport ]; then
	echo "ofport is empty"
	exit 1
fi

xml=$(virsh dumpxml $name | sed -n /type=\'hostdev\'/,/interface/p)
if [ -z "$xml" ]; then
	echo "xml is empty, may no sriov"
	exit 0
fi

virsh dumpxml $name | sed -n /type=\'hostdev\'/,/interface/p > vf.xml

virsh detach-device $name vf.xml 2>/dev/null
if [ $? -eq 0 ]; then
	ovs-vsctl del-port br0 $vf_name
	sleep 10
	ovs-vsctl add-port br0 $vnet_name -- set interface $vnet_name ofport_request=$ofport
	# ~/iproute2/tc/tc qdisc del  dev $vnet_name  ingress
	echo "start live migration"
	#virsh migrate-setspeed $name 10000000000
	#virsh migrate --verbose --live --copy-storage-all --unsafe --persistent $name qemu+ssh://192.168.152.75/system  tcp://192.168.152.75
else
	echo "detach vf.xml fail" 	
	exit 1;
fi

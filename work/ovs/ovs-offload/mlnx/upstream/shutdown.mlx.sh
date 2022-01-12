name="$1"
state_ok="running"
vf_prefix="mlx_pf0vf"

state=$(virsh domstate $name 2> /dev/null)
if [[ $state != $state_ok ]]; then
	echo "domain $name is not running"
	exit 1
fi

mac=$(virsh dumpxml $name | grep -m 1 'mac address' | cut -d\' -f2)
if [ -z $mac ]; then
	echo "warning mac is empty, can't delete vf port from ovs"
else
	vf_num=$(ip l | grep "$mac" | sed 's/^ *//g' | cut -d " " -f 2)
	vf_name="$vf_prefix$vf_num"
	if [ -z $vf_name ]; then
		echo "warning vf_name is empty, can't delete vf port from ovs"
	else 
		ovs-vsctl --if-exists del-port br0 $vf_name
	fi
fi

virsh shutdown $name
sleep 2
pci_e=$(lspci | grep Ether |grep "Virtual Function" | cut -d " " -f 1 | head -$((vf_num+1)) | tail -1)
pci="0000:$pci_e" 
echo $pci > "/sys/bus/pci/drivers/mlx5_core/unbind"
ip link set mlx_p0 vf $vf_num mac 0:0:0:0:0:0 



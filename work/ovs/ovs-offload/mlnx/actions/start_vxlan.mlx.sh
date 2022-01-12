name="$1"
state_ok="running"
vf_prefix="mlx_"
extra_msix=4

set_affinity()
{
    printf "%s mask=%X for /proc/irq/%d/smp_affinity\n" $DEV $MASK $IRQ
    printf "%X" $MASK > /proc/irq/$IRQ/smp_affinity
}

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

ofport=$(ovs-vsctl --bare --columns=ofport list interface $vnet_name)
if [ -z $ofport ]; then
	echo "ofport is empty"
	exit 1
fi

num=$(echo $vnet_name | sed 's/vnet//g')
if [ -z $num ]; then
	echo "vnet num is empty"
	exit 1
fi

vf_name="$vf_prefix$num"

pci_e=$(lspci | grep Ether |grep "Virtual Function" | cut -d " " -f 1 | head -$((num+1)) | tail -1)
pci="0000:$pci_e" #$(ethtool -i $vf_name | grep bus-info | sed 's/bus-info:.* //g')
domain=$(echo $pci | cut -d ":" -f 1)
bus=$(echo $pci | cut -d ":" -f 2)
sf=$(echo $pci | cut -d ":" -f 3)
slot=$(echo $sf | cut -d "." -f 1)
func=$(echo $sf | cut -d "." -f 2)

if [ -z $domain -o -z $bus -o -z $slot -o -z $func ]; then
	echo "pci num fail $vf_name"
	exit 1
fi

msix_num=$(cat /proc/interrupts | grep vfio-msix | wc -l)
vf_num=$(cat /proc/interrupts | grep "vfio-msix\[0\]" |  wc -l)
s_core=$((msix_num-extra_msix*vf_num))
core_num=$(cat /proc/cpuinfo | grep 'processor' | wc -l)

#TODO pcie num
cat vf-tmp.xml | sed "s/MAC/$mac/g" | sed "s/DOMAIN/$domain/g" | sed "s/BUS/$bus/g" | sed "s/SLOT/$slot/g" | sed "s/FUNCTION/$func/g" > vf.xml

virsh attach-device $name vf.xml 2>/dev/null
if [ $? -eq 0 ]; then
	ovs-vsctl del-port vxlan-br $vnet_name
	ip l s dev $vf_name  up
	ovs-vsctl --may-exist add-port vxlan-br $vf_name  -- set interface $vf_name ofport_request=$ofport
	
	vf_msix_num=0
	while [ $vf_msix_num -eq 0 ]
	do
		vf_msix_num=$(cat /proc/interrupts | grep vfio-msix | grep $pci | wc -l)
	done
	
	sleep 1

	vf_msix_num=$(cat /proc/interrupts | grep vfio-msix | grep $pci | wc -l)
	vf_msix_num=$((vf_msix_num-extra_msix))
	cat /proc/interrupts | grep vfio-msix | grep $pci | tail -$vf_msix_num | sed s/^\ *//g |cut -d ":" -f 1  > "/tmp/vf_$vf_name" 
	while read irq 
	do
		if [ -n "${irq}" ]; then
			queue=$(($[s_core%${core_num}]))
			MASK=$((1<<$((queue))))
			IRQ=$irq
			DEV="${vf_name}"
			set_affinity
			s_core=$((s_core+1))
		fi
	done < "/tmp/vf_$vf_name"
else
	echo "attach vf.xml fail" 	
	exit 1;
fi


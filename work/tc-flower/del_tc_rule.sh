ovs-dpctl show | grep mlx_ | sed s/^.*mlx_/mlx_/ > /tmp/mlx_interfaces
echo "gre_sys" >>  /tmp/mlx_interfaces

while read interface
do
	echo $interface
	tc filter ls dev $interface ingress protocol ip | grep "flower handle" | sed "s/filter //" | sed "s/flower //" > /tmp/mlx_rules_$interface
	while read rule
	do
		tc filter ls dev $interface ingress protocol ip $rule 2> /dev/null | grep "ip_proto tcp" > /dev/null
		if [ $? == 0 ]
		then
			tc filter ls dev $interface ingress protocol ip $rule 2> /dev/null | grep "src_port" > /dev/null
			if [ $? == 0 ]
			then
				echo "delete rule $rule on interface $interface"
				tc filter del dev $interface ingress protocol ip $rule flower
				sleep 1
			fi
		fi
		
	done < /tmp/mlx_rules_$interface
done < /tmp/mlx_interfaces

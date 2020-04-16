
rm add* -f
rm del* -f
rm dump* -f

batch=$1
num=$2

#echo $batch,$num

for i in $( eval echo {1..$batch} )
do
	cmd_d="filter ls dev gre_sys ingress"
	echo $cmd_d >> dump_$i
	for j in $( eval echo {1..$num} )
	do
		echo $i,$j
		k=$(((i-1)*num+j))
		cmd_a="filter add dev gre_sys prio 3 parent ffff: handle $k protocol ip flower skip_sw enc_dst_ip 172.168.152.75 enc_src_ip 172.168.152.241 enc_key_id $k enc_dst_port 0 ip_flag nofrag action tunnel_key unset pipe action mirred egress redirect dev mlx_pf0vf0"
		echo $cmd_a >> add_$i
		cmd_d="filter del dev gre_sys prio 3 parent ffff: protocol ip handle $k flower"
		echo $cmd_d >> del_$i
	done
done

# bash set_batch.sh 10 1000
# time find dump_* -print | xargs -n 1 -P 10 tc -b

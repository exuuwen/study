ip l del dev br-test
ip l del dev veth1
ip l del dev veth2
ip netns del ns1
ip netns del ns2

ip netns add ns1 
ip netns add ns2 
ip link add dev veth1 type veth peer name eth0 netns ns1
ip link add dev veth2 type veth peer name eth0 netns ns2
ifconfig veth1 up
ifconfig veth2 11.0.0.1/24 up

ip netns exec ns1 ifconfig eth0 10.0.0.1/24 up
ip netns exec ns2 ifconfig eth0 10.0.0.2/24 up

ip l add dev br-test type bridge vlan_filtering 1 vlan_default_pvid 200 # if no set the default pvid 1
brctl addif br-test veth1
brctl addif br-test veth2
ifconfig br-test up


bridge vlan add dev veth1 vid 100 pvid untagged
bridge vlan add dev veth2 vid 100 pvid untagged

ip l a link br-test name br-test.100 type vlan id 100
ifconfig br-test.100 10.0.0.3/24 up
bridge vlan add dev br-test vid 100 self  # set br-test port must take with self

#ifconfig br-test0 10.0.0.3/24 up
#bridge vlan add dev br-test vid 100 self  # set br-test port must take with self



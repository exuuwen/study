echo 0 > /proc/sys/net/ipv6/conf/all/disable_ipv6
echo 0 > /proc/sys/net/ipv6/conf/all/accept_ra
ip -6 addr add dev eth0 2001:db8:1:2:3:5:a00:7/120

router bgp 7676
  bgp router-id 192.168.200.1
  neighbor 2001:db8:1:2:3:5:a00:1 remote-as 7676
  address-family ipv6
  network ::/0
  neighbor 2001:db8:1:2:3:5:a00:1 activate
  exit-address-family



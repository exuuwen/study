ifconfig eth0 0.0.0.0
ip addr a dev eth0 10.0.0.7/24
ip addr a dev eth0 12.0.0.7/32

router bgp 7676
  bgp router-id 10.0.0.7
  network 12.0.0.0/24
  neighbor 10.0.0.1 remote-as 7676


router bgp 7676
bgp router-id 172.168.1.1
  network 11.0.0.0/24 route-map set_metric
  neighbor 172.168.1.7 remote-as 7676

route-map set_metric permit 10
  set metric 1000


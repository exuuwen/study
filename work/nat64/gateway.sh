
#vppctl create host-interface name vethf75e62b
#vppctl create host-interface name vethf3c2e70

#ipv6 bgp 2001:db8:1:2:3:5:a00:1/120 <-----> 2001:db8:1:2:3:5:a00:7/120
#ipv4 bgp 10.0.0.1/24 <-----> 10.0.0.7/24
#nat64: 
# src pool 11.0.0.7, prefix 2001:db8:1:2:3:4::/96
# ipv6 addr: 2001:db8:1:2:3:4:c00:7/128  ipv4 addr: 12.0.0.7

#device index 1 is vpp0 for ipv4
#device index 2 is vpp1 for ipv6
echo 0 > /proc/sys/net/ipv6/conf/all/disable_ipv6
echo 0 > /proc/sys/net/ipv6/conf/all/accept_ra

create host-interface name vethf75e62b
create host-interface name vethf3c2e70
enable tap-inject

ifconfig vpp0 10.0.0.1/24 up
ifconfig vpp1 up
ip -6 addr add dev vpp1 2001:db8:1:2:3:5:a00:1/120

set interface nat64 in  host-vethf3c2e70
set interface nat64 out host-vethf75e62b
nat64 add pool address 11.0.0.7
nat64 add prefix 2001:db8:1:2:3:4::/96


router bgp 7676
  bgp router-id 192.168.200.7
  neighbor 2001:db8:1:2:3:5:a00:7 remote-as 7676
  address-family ipv6
  network 2001:db8:1:2:3:4:c00:7/128
  neighbor 2001:db8:1:2:3:5:a00:7 activate
  exit-address-family
!
  network 11.0.0.0/24
  neighbor 10.0.0.7 remote-as 7676

git clone https://github.com/FDio/vpp.git
git clone https://gerrit.fd.io/r/vppsb

#build vpp rpm
cd vpp
git checkout v18.07
git am 0001-make-tap-inject-plugin-work.patch
git am 0002-bypass-interface-address.patch
first time: build-root/vagrant/build.sh
others: make pkg-rpm

rpm -ivh vpp-lib-18.07-2~g5bce438.x86_64.rpm vpp-18.07-2~g5bce438.x86_64.rpm vpp-devel-18.07-2~g5bce438.x86_64.rpm vpp-plugins-18.07-2~g5bce438.x86_64.rpm vpp-selinux-policy-18.07-2~g5bce438.x86_64.rpm

#build tap-inject plugin
cd vppsb
git am 0001-make-router-compiler-ok.patch

cd ../vpp
ln -sf ../vppsb/netlink
ln -sf ../vppsb/router
ln -sf ../../netlink/netlink.mk build-data/packages/
ln -sf ../../router/router.mk build-data/packages/
make -C build-root PLATFORM=vpp TAG=vpp install-packages netlink-install router-install
ln -sf ./build-root/install-vpp-native/router/lib64/router.so.0.0.0  /usr/lib/vpp_plugins/router.so


build-root/vagrant/build.sh

make -C build-root PLATFORM=vpp TAG=vpp install-packages netlink-install router-install

rpm -ivh vpp-selinux-policy-18.07-rc0~51_g5c9083d.x86_64.rpm vpp-lib-18.07-rc0~51_g5c9083d.x86_64.rpm vpp-18.07-rc0~51_g5c9083d.x86_64.rpm vpp-devel-18.07-rc0~51_g5c9083d.x86_64.rpm vpp-plugins-18.07-rc0~51_g5c9083d.x86_64.rpm


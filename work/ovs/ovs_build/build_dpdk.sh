#!/bin/bash
set -x
pushd ../dpdk-ctyun
rm -rf build output
if [ "x$1" == "xdebug" ];then
    meson -Dc_args="-DRTE_LIBRTE_VIRTIO_DEBUG_TX" -Dexamples=l2fwd,fpga_simulator -Dbuildtype=debug -Denable_drivers=bus/vdev,net/bonding,net/af_packet,net/dpe,mempool/ring,net/virtio --prefix $(pwd)/output $(pwd)/build
else
    meson  -Dexamples=l2fwd,fpga_simulator -Denable_drivers=bus/vdev,net/bonding,net/af_packet,net/dpe,mempool/ring,net/virtio --prefix $(pwd)/output $(pwd)/build
fi
ninja -C ./build
ninja -C ./build install
#rm -rf ./build # delete for disk space
cp -f $(pwd)/output/lib64/pkgconfig/libdpdk.pc  ../build
cp -f $(pwd)/output/lib64/pkgconfig/libdpdk-libs.pc ../build
popd

sed -i 's/-l:librte_net_dpe.a//g' $(pwd)/libdpdk.pc
sed -i 's/-l:librte_bus_vdev.a/-l:librte_net_dpe.a -l:librte_bus_vdev.a/g' $(pwd)/libdpdk.pc

set +x

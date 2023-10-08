#!/bin/bash
export PKG_CONFIG_PATH=`pwd`:$PKG_CONFIG_PATH
pushd ../ovs-ctyun
rm -rf ovs_install
make distclean
./boot.sh
if [ "$1" == "test" ];then
     ./configure --with-dpdk=static --with-debug CFLAGS='-O0 -g' --prefix=$(pwd)/ovs_install
     make check TESTSUITEFLAGS=--list
else
    ./configure --with-dpdk=static --with-debug CFLAGS='-O0 -g' --prefix=$(pwd)/ovs_install
    make -j4
    make install
fi
popd

#!/usr/bin/python

from mininet.net import Mininet
from mininet.node import Host, Controller, RemoteController, Node, OVSSwitch
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from mininet.link import Link, Intf

def emptyNet():

    net = Mininet(topo=None, build=False)
    c0 = Controller('c0', inNamespace=False)

    h1 = Host('h1')
    h2 = Host('h2')
    #intf1 = Intf("h1-eth1")
    #intf2 = Intf("h2-eth1")
    s1 = OVSSwitch('br0', inNamespace=False)    

    Link(h1, s1)
    Link(h2, s1)


    c0.start()
    s1.start([c0])

    net.start()    
    #s1.cmd('ovs-vsctl set bridge br0 protocols=OpenFlow13')
    CLI(net)

    net.stop()
    h1.stop()
    h2.stop()
    s1.stop()
    c0.stop()
    

if __name__ == '__main__':
    setLogLevel( 'info' )
    emptyNet()

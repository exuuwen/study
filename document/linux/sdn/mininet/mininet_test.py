#!/usr/bin/python

from mininet.net import Mininet
from mininet.node import OVSSwitch, Controller, RemoteController, Node
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from mininet.link import Link, Intf

def emptyNet():

    net = Mininet(switch=OVSSwitch, build=False)

    #c0 = net.addController('c0', controller=RemoteController, port=6633)
    c0 = net.addController('c0', controller=Controller, port=6634)

    h1 = net.addHost('host1', ip='192.168.0.1/24', mac='00:1e:65:15:fc:01')
    h2 = net.addHost('host2', ip='192.168.0.2/24', mac='00:1e:65:15:fc:02')
    s1 = net.addSwitch('br0')
    net.addLink(h1, s1)
    net.addLink(h2, s1)

    #net.start()
    net.build()

    c0.start()
    s1.start([c0])

    #s1.cmd('ovs-vsctl set bridge br0 protocols=OpenFlow13')
    #s1.cmd('ovs-vsctl set bridge br0 other-config:disable-in-band=true')

    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel( 'info' )
    emptyNet()

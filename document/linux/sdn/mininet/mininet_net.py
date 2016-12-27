#!/usr/bin/python

from mininet.net import Mininet
from mininet.node import OVSSwitch, Controller, RemoteController, Node, Host
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from mininet.link import Link, Intf

def emptyNet():

    net = Mininet(switch=OVSSwitch, build=False)

    #c0 = net.addController('c0', controller=RemoteController, port=6633)
    #c1 = net.addController('c1', controller=RemoteController, port=6634)
    c0 = net.addController('c0', controller=Controller, port=6634)
    c1 = net.addController('c1', controller=Controller, port=6635)

    h1 = net.addHost('host1', ip='192.168.0.2/24', mac='00:1e:65:15:fc:01')
    h2 = net.addHost('host2', ip='192.168.0.3/24', mac='00:1e:65:15:fc:02')
    s1 = net.addSwitch('br0')
    net.addLink(h1, s1)
    net.addLink(h2, s1)

    h3 = net.addHost('host3', ip='172.168.0.2/24', mac='00:1e:65:15:fc:03')
    h4 = net.addHost('host4', ip='172.168.0.3/24', mac='00:1e:65:15:fc:04')
    s2 = net.addSwitch('br1')
    net.addLink(h3, s2)
    net.addLink(h4, s2)

    # hide the gatway in net
    gateway = Host('gateway')
    net.addLink(s1, gateway)
    net.addLink(s2, gateway)
    gateway.intf("gateway-eth0").setIP('192.168.0.1/24')
    gateway.intf("gateway-eth1").setIP('172.168.0.1/24')

    # show gateway in net
    #gateway = net.addHost('gateway', ip='192.168.0.1/24')
    #net.addLink(s1, gateway)
    #net.addLink(s2, gateway)
    #gateway.intf("gateway-eth1").setIP('172.168.0.1/24')

    net.build()

    print h1.cmd('ip route add default via 192.168.0.1 dev host1-eth0')
    print h2.cmd('ip route add default via 192.168.0.1 dev host2-eth0')
    print h3.cmd('ip route add default via 172.168.0.1 dev host3-eth0')
    print h4.cmd('ip route add default via 172.168.0.1 dev host4-eth0')

    c0.start()
    c1.start()
    s1.start([c0])
    s2.start([c1])

    #s1.cmd('ovs-vsctl set bridge br0 protocols=OpenFlow13')
    #s2.cmd('ovs-vsctl set bridge br1 protocols=OpenFlow13')
    CLI( net )
    net.stop()


if __name__ == '__main__':
    setLogLevel( 'info' )
    emptyNet()


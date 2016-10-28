#!/usr/bin/python

from mininet.net import Mininet
from mininet.node import OVSSwitch, Controller, RemoteController, Node, Host
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from mininet.link import Link, Intf

def emptyNet():

    net = Mininet(switch=OVSSwitch, build=False)


    c1 = net.addHost('c1', ip='192.168.0.101/24', mac='00:1e:65:15:fc:11')
    c2 = net.addHost('c2', ip='192.168.0.102/24', mac='00:1e:65:15:fc:12')
    c3 = net.addHost('c3', ip='192.168.0.103/24', mac='00:1e:65:15:fc:13')

    s1 = net.addSwitch('s1')

    s21 = net.addSwitch('s21')
    s22 = net.addSwitch('s22')
    s23 = net.addSwitch('s23')

    net.addLink(s21, s1)
    net.addLink(s22, s1)
    net.addLink(s23, s1)

    net.addLink(c1, s1)
    net.addLink(c2, s1)
    net.addLink(c3, s1)

    h1 = net.addHost('h1', ip='192.168.0.1/24', mac='00:1e:65:15:fc:01')
    h2 = net.addHost('h2', ip='192.168.0.2/24', mac='00:1e:65:15:fc:02')
    h3 = net.addHost('h3', ip='192.168.0.3/24', mac='00:1e:65:15:fc:03')
    h4 = net.addHost('h4', ip='192.168.0.4/24', mac='00:1e:65:15:fc:04')
    h5 = net.addHost('h5', ip='192.168.0.5/24', mac='00:1e:65:15:fc:05')
    h6 = net.addHost('h6', ip='192.168.0.6/24', mac='00:1e:65:15:fc:06')
    
    s3 = net.addSwitch('s3')

    net.addLink(s21, s3)
    net.addLink(s22, s3)
    net.addLink(s23, s3)

    net.addLink(h1, s3)
    net.addLink(h2, s3)
    net.addLink(h3, s3)
    net.addLink(h4, s3)
    net.addLink(h5, s3)
    net.addLink(h6, s3)


    net.start()

    CLI( net )
    net.stop()


if __name__ == '__main__':
    setLogLevel( 'info' )
    emptyNet()


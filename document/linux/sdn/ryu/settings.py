#!/usr/bin/python
# -*- coding: utf-8 -*-

from ryu.lib.packet import arp
import logging

ARP = arp.arp.__name__

UINT32_MAX = 0xffffffff

BROADCAST_MAC = "ff:ff:ff:ff:ff:ff"

HOST_IP = "172.22.1.2"

ARP_HOST_IP = "172.22.1.2"

HOST_PORT = 7250

# log conf
LOG_PATH = "/var/log/"
LOG_LEVEL = logging.DEBUG
LOG_MAX_SIZE_BYTES = 10 * 1024 * 1024
LOG_ROLLOVER_COUNT = 10

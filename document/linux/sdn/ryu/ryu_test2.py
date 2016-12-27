# Copyright (C) 2011 Nippon Telegraph and Telephone Corporation.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from ryu.base import app_manager
from ryu.controller import ofp_event
from ryu.controller.handler import CONFIG_DISPATCHER, MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_3, ether
from ryu.lib.packet import packet
from ryu.lib.packet import ethernet
from ryu.lib.packet import arp

import settings

ip2mac = {"192.168.0.1":"00:1e:65:15:fc:01", "192.168.0.2":"00:1e:65:15:fc:02"}

class Simple(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_3.OFP_VERSION]

    def __init__(self, *args, **kwargs):
        super(Simple, self).__init__(*args, **kwargs)

    @set_ev_cls(ofp_event.EventOFPSwitchFeatures, CONFIG_DISPATCHER)
    def switch_features_handler(self, ev):
        datapath = ev.msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        match = parser.OFPMatch(eth_dst=settings.BROADCAST_MAC, eth_type=ether.ETH_TYPE_ARP)
        actions = [parser.OFPActionOutput(ofproto.OFPP_CONTROLLER, ofproto.OFPCML_NO_BUFFER)]
        self.add_flow(datapath, 60010, match, actions)

    def add_flow(self, datapath, priority, match, actions):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        inst = [parser.OFPInstructionActions(ofproto.OFPIT_APPLY_ACTIONS,
                                             actions)]

        mod = parser.OFPFlowMod(datapath=datapath, priority=priority,
                                match=match, instructions=inst)
        datapath.send_msg(mod)

    def send_arp(self, arp_opcode, src_mac, dst_mac, src_ip, dst_ip, arp_target_mac, in_port, output, ofproto, datapath, msg, parser):
        # Generate ARP packet
        ether_proto = ether.ETH_TYPE_ARP
        hwtype = 1
        arp_proto = ether.ETH_TYPE_IP
        hlen = 6
        plen = 4

        pkt = packet.Packet()
        e = ethernet.ethernet(dst_mac, src_mac, ether_proto)
        a = arp.arp(hwtype, arp_proto, hlen, plen, arp_opcode, src_mac, src_ip, arp_target_mac, dst_ip)
        pkt.add_protocol(e)
        pkt.add_protocol(a)
        pkt.serialize()

        self.logger.info("send arp pkt:\n%s" %(pkt))

        actions = [parser.OFPActionOutput(output, 0)]
        out = parser.OFPPacketOut(datapath=datapath, buffer_id=0xffffffff,
                        in_port = in_port, actions=actions, data=pkt.data)
        datapath.send_msg(out)
    
    def _packetin_arp(self, msg, header_list, in_port, ofproto, datapath, parser, arp_answer):
        src_addr =  header_list[settings.ARP].src_ip
        if src_addr is None:
            self.logger.error("src_addr is none")
            return

        src_mac = header_list[settings.ARP].src_mac
        dst_mac = arp_answer
        arp_target_mac = src_mac
        output = in_port
        in_port = ofproto.OFPP_CONTROLLER
        src_ip = header_list[settings.ARP].src_ip
        dst_ip = header_list[settings.ARP].dst_ip

        self.logger.info("call send_arp")
        self.send_arp(arp.ARP_REPLY, dst_mac, src_mac, dst_ip, src_ip, arp_target_mac, in_port, output, ofproto, datapath, msg, parser)

    def proxy_arp(self, arp_answer, msg, in_port, ofproto, datapath, parser):
        self.logger.info("START ARP reply logic...")
        header_list = dict((p.protocol_name, p) for p in self.pkt.protocols if type(p) != str)

        if header_list:
            # Analyze event type.
            if settings.ARP in header_list:
                self.logger.info("is arp_request")
                if header_list[settings.ARP].opcode == arp.ARP_REQUEST:
                    self.logger.info("do packetin_arp")
                    self._packetin_arp(msg, header_list, in_port, ofproto, datapath, parser, arp_answer)
                    self.logger.info("END ARP reply logic...")
                    return
    
    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def _packet_in_handler(self, ev):
        msg = ev.msg
        datapath = msg.datapath
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser
        in_port = msg.match['in_port']

        self.pkt = packet.Packet(msg.data)
        pkt = packet.Packet(msg.data)
        eth = pkt.get_protocols(ethernet.ethernet)[0]

        dst = eth.dst
        src = eth.src

        dpid = datapath.id
        #self.mac_to_port.setdefault(dpid, {})

        self.logger.info("packet in %s %s %s %s", dpid, src, dst, in_port)

        _arp = None
        if eth.ethertype == ether.ETH_TYPE_ARP:
            _arp = pkt.get_protocol(arp.arp)
        
            if _arp.opcode == arp.ARP_REQUEST and _arp.src_ip != "0.0.0.0" and _arp.src_ip != _arp.dst_ip:
                if ip2mac.has_key(_arp.dst_ip):
                    arp_tha = ip2mac[_arp.dst_ip]
                    self.logger.info("dst mac=%s, dst ip=%s, arp hit" %(ip2mac[_arp.dst_ip], _arp.dst_ip))
                    #self.proxy_arp(arp_tha, msg, in_port, ofproto, datapath, parser)
                    self.send_arp(arp.ARP_REPLY, arp_tha, src, _arp.dst_ip, _arp.src_ip, src, ofproto.OFPP_CONTROLLER, in_port, ofproto, datapath, msg, parser)
                    match = parser.OFPMatch(in_port=in_port, eth_src=src, eth_dst=arp_tha)
                    actions = [] 
                    if in_port == 1:
                        port = 2
                    elif in_port == 2:
                        port = 1
                    else:
                        return 
                    actions.append(parser.OFPActionOutput(port))
                    self.add_flow(datapath, 0, match, actions)
                    return
        self.logger.info("other\n")

        
        # learn a mac address to avoid FLOOD next time.
        #self.mac_to_port[dpid][src] = in_port

        #if dst in self.mac_to_port[dpid]:
            #out_port = self.mac_to_port[dpid][dst]
        #else:
            #out_port = ofproto.OFPP_FLOOD

        #actions = [parser.OFPActionOutput(out_port)]

        # install a flow to avoid packet_in next time
        #if out_port != ofproto.OFPP_FLOOD:
            #match = parser.OFPMatch(in_port=in_port, eth_dst=dst)
            #self.add_flow(datapath, 1, match, actions)

        #data = None
        #if msg.buffer_id == ofproto.OFP_NO_BUFFER:
            #data = msg.data

        #out = parser.OFPPacketOut(datapath=datapath, buffer_id=msg.buffer_id, in_port=in_port, actions=actions, data=data)
        #datapath.send_msg(out)



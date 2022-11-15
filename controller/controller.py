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

# Modified version of the simple_switch.py example from Ryu

"""
An OpenFlow 1.0 controller installing Per-connection flows.
"""
from ryu.base import app_manager
from ryu.topology.event import EventSwitchEnter, EventSwitchLeave
from ryu.controller import ofp_event
from ryu.controller.handler import MAIN_DISPATCHER
from ryu.controller.handler import set_ev_cls
from ryu.ofproto import ofproto_v1_0
from ryu.lib.mac import haddr_to_bin
from ryu.lib.packet import packet, ethernet, ether_types, arp, ipv4, tcp, udp
from ryu.app.ofctl.api import get_datapath

HOST_PORT = 2
OUT_PORT = 1
IP_TCP = 6
IP_UDP = 17
IP_ICMP = 1

class SimpleSwitch(app_manager.RyuApp):
    OFP_VERSIONS = [ofproto_v1_0.OFP_VERSION]

    def __init__(self, *args, **kwargs):
        super(SimpleSwitch, self).__init__(*args, **kwargs)
        self.mac_to_port = {}
        # Get a list of all the datapaths
        # print("datapath: %s" % datapath)
        # # Delete all existing flows
        # self.delete_all_flows()
        # # Init ARP flood
        # self.arp_flood(datapath)
        # # Init host egress
        # self.host_egress(datapath)

    @set_ev_cls(EventSwitchEnter)
    def onSwitchConnection(self, event):
        self.datapaths = get_datapath(self)
        self.delete_all_flows(self.datapaths[0])
        self.arp_flood(self.datapaths[0])
        self.host_egress(self.datapaths[0])

    def delete_all_flows(self, datapath):
        ofproto = datapath.ofproto
        parser = datapath.ofproto_parser

        match = parser.OFPMatch()
        mod = parser.OFPFlowMod(datapath=datapath, command=ofproto.OFPFC_DELETE,
                                match=match)
        datapath.send_msg(mod)

    def arp_flood(self, datapath):
        match = datapath.ofproto_parser.OFPMatch(dl_type=ether_types.ETH_TYPE_ARP)
        actions = [datapath.ofproto_parser.OFPActionOutput(datapath.ofproto.OFPP_FLOOD)]
        mod = datapath.ofproto_parser.OFPFlowMod(
            datapath=datapath, 
            match=match, 
            cookie=0,
            command=datapath.ofproto.OFPFC_ADD,
            idle_timeout=0,
            hard_timeout=0,
            priority=0,
            actions=actions)
        datapath.send_msg(mod)

    def host_egress(self, datapath):
        match = datapath.ofproto_parser.OFPMatch(in_port=HOST_PORT)
        actions = [datapath.ofproto_parser.OFPActionOutput(OUT_PORT)]
        mod = datapath.ofproto_parser.OFPFlowMod(
            datapath=datapath, 
            match=match, 
            cookie=0,
            command=datapath.ofproto.OFPFC_ADD,
            idle_timeout=0,
            hard_timeout=0,
            priority=0,
            actions=actions)
        datapath.send_msg(mod)

    def add_flow(self, datapath, in_port, flow_spec, actions):
        ofproto = datapath.ofproto

        if flow_spec["nw_proto"] == IP_ICMP:
            match = datapath.ofproto_parser.OFPMatch(
                in_port=in_port,
                dl_type=ether_types.ETH_TYPE_IP,
                nw_proto=flow_spec["nw_proto"],
                nw_src=flow_spec["nw_src"])
        elif flow_spec["nw_proto"] == IP_TCP or flow_spec["nw_proto"] == IP_UDP:
            match = datapath.ofproto_parser.OFPMatch(
                in_port=in_port,
                # Specify that the packet is an IPv4 packet
                dl_type=flow_spec['dl_type'],
                nw_proto=flow_spec['nw_proto'],
                nw_src=flow_spec['nw_src'],
                tp_src=flow_spec['tp_src'])
        else:
            raise Exception("Unsupported nw_proto: %s" % flow_spec["nw_proto"])

        mod = datapath.ofproto_parser.OFPFlowMod(
            datapath=datapath, match=match, cookie=0,
            command=ofproto.OFPFC_ADD, idle_timeout=0, hard_timeout=0,
            priority=ofproto.OFP_DEFAULT_PRIORITY,
            flags=ofproto.OFPFF_SEND_FLOW_REM, actions=actions)
        datapath.send_msg(mod)

    @set_ev_cls(ofp_event.EventOFPPacketIn, MAIN_DISPATCHER)
    def _packet_in_handler(self, ev):
        msg = ev.msg
        datapath = msg.datapath
        flow_spec = {
            'dl_type': '',
            'nw_proto': '',
            'nw_src': '',
            'tp_src': ''
        }
        pkt = packet.Packet(msg.data)
        # Get the Ethernet frame
        eth = pkt.get_protocol(ethernet.ethernet)

        if eth.ethertype == ether_types.ETH_TYPE_LLDP:
            # ignore lldp packet
            return
        flow_spec['dl_type'] = eth.ethertype
        # If the packet is an IPv4 packet
        if (flow_spec['dl_type'] == ether_types.ETH_TYPE_IP):
            # Get the IP packet
            ip = pkt.get_protocol(ipv4.ipv4)
            flow_spec['nw_proto'] = ip.proto
            flow_spec['nw_src'] = ip.src
            if (flow_spec['nw_proto'] == 6):
                # Get the TCP packet
                tcp_pkt = pkt.get_protocol(tcp.tcp)
                flow_spec['tp_src'] = tcp_pkt.src_port
            elif (flow_spec['nw_proto'] == 17):
                # Get the UDP packet
                udp_pkt = pkt.get_protocol(udp.udp)
                flow_spec['tp_src'] = udp_pkt.src_port
            actions = [datapath.ofproto_parser.OFPActionOutput(HOST_PORT)]
            self.add_flow(datapath, msg.in_port, flow_spec, actions)
            self.logger.info("packet received on port %s from %s", msg.in_port, ip.src)

    @set_ev_cls(ofp_event.EventOFPPortStatus, MAIN_DISPATCHER)
    def _port_status_handler(self, ev):
        msg = ev.msg
        reason = msg.reason
        port_no = msg.desc.port_no

        ofproto = msg.datapath.ofproto
        if reason == ofproto.OFPPR_ADD:
            self.logger.info("port added %s", port_no)
        elif reason == ofproto.OFPPR_DELETE:
            self.logger.info("port deleted %s", port_no)
        elif reason == ofproto.OFPPR_MODIFY:
            self.logger.info("port modified %s", port_no)
        else:
            self.logger.info("Illeagal port state %s %s", port_no, reason)

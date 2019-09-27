from trex_stl_lib.api import *

class STLS1(object):
    """ attack 48.0.0.1 at port 80
    """

    def __init__ (self):
        self.max_pkt_size_l3  =9*1024;

    def create_stream (self):

        # TCP SYN
	base_pkt = Ether(src='3c:fd:fe:a6:21:3c',dst='3c:fd:fe:a7:7a:24')/IP(dst='172.168.152.77')/GRE(key_present=1,key=1000,proto=0x6558)/Ether(src='52:54:00:00:00:00', dst='fa:ff:ff:ff:ff:ff')/IP()/UDP(sport=22345, dport=5001)


        # create an empty program (VM)
        vm = STLVM()

        # define two vars
        vm.var(name = "ip_dst", min_value = "10.7.254.255", max_value = "10.7.255.255", size = 4, op = "inc")
        vm.var(name = "ip_src", min_value = "1.1.255.255", max_value = "1.1.255.255", size = 4, op = "inc")
        vm.var(name = "out_ip_src", min_value = "172.168.152.75", max_value = "172.168.152.75", size = 4, op = "inc")
        #vm.var(name = "src_port", min_value = 1025, max_value = 65000, size = 2, op = "random")
        
        # write src IP and fix checksum
        vm.write(fv_name = "ip_dst", pkt_offset = 72)
        vm.write(fv_name = "ip_src", pkt_offset = 68)
        vm.write(fv_name = "out_ip_src", pkt_offset = 26)
        vm.fix_chksum()
        
        # write TCP source port
        #vm.write(fv_name = "src_port", pkt_offset = "TCP.sport")
        
        # create the packet
        pkt = STLPktBuilder(pkt = base_pkt, vm = vm)

        return STLStream(packet = pkt,
                         random_seed = 0x1234,# can be remove. will give the same random value any run
                         mode = STLTXCont())



    def get_streams (self, direction = 0, **kwargs):
        # create 1 stream 
        return [ self.create_stream() ]


# dynamic load - used for trex console or simulator
def register():
    return STLS1()




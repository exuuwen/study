ip xfrm state add src 192.168.0.1 dst 192.168.0.2 proto esp spi 0x00000301 mode tunnel auth md5 0x96358c90783bbfa3d7b196ceabe0536b enc des3_ede 0xf6ddb555acfd9d77b03ea3843f2653255afe8eb5573965df  
ip xfrm state add src 192.168.0.2 dst 192.168.0.1 proto esp spi 0x00000302 mode tunnel auth md5 0x99358c90783bbfa3d7b196ceabe0536b enc des3_ede 0xffddb555acfd9d77b03ea3843f2653255afe8eb5573965df 
ip xfrm state get src 192.168.0.1 dst 192.168.0.2 proto esp spi 0x00000301
ip xfrm policy add src 192.168.0.1 dst 192.168.0.2 dir out ptype main tmpl src 192.168.0.1 dst 192.168.0.2 proto esp mode tunnel 
#ip xfrm policy add src 192.168.0.2 dst 192.168.0.1 dir in ptype main tmpl src 192.168.0.2 dst 192.168.0.1 proto esp mode tunnel 
ip xfrm policy list

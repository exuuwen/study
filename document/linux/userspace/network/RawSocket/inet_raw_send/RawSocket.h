#ifndef RAW_SOCKET_H
#define RAW_SOCKET_H

#include <CommonLib.h>

#define IP_AND_UDP_HEADER_SIZE (sizeof(struct iphdr) + sizeof(struct udphdr))
#define MTU 1500 

class RawSocket
{
public:
	RawSocket(u32 p_addr, u16 port);
	RawSocket(u16 port);	
	virtual ~RawSocket();

public:
	int Open();
	int Send(unsigned char * buf, u32 len, u32 dest_ip, u16 dest_port);
	void SetFakeSrcIpAndPort(u32 ip, u16 port);   

private:
	void Close();
	int sock;
	bool isOpen; 
	u32 Fake_Src_Ip;
	u16 Fake_Src_Port; 
};

#endif

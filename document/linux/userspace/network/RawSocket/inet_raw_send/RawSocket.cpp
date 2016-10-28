#include "RawSocket.h"
#include "CheckSum.h"

RawSocket::RawSocket(u16 port) 
{
	// Init the fake ip and port number
	SetFakeSrcIpAndPort(INADDR_ANY, port);
}

RawSocket::RawSocket(u32 ip_addr, u16 port)
{
	SetFakeSrcIpAndPort(ip_addr, port);
}

void RawSocket::Close()                                                                                          
{       
 	if (sock > 0)                                                                                                  
	{                                                                                                              
		close(sock);                                                                                               
	}   
                                                                                                           
	sock = 0;                                                                                                      
	isOpen = false;                                                                                               
}       

RawSocket::~RawSocket()
{
	Close();
}

// by default, fake ip and port are same with server binding ip and port.
void RawSocket::SetFakeSrcIpAndPort(u32 ip, u16 port)
{
	Fake_Src_Ip = ip;
	Fake_Src_Port = port;		
}

int RawSocket::Open()
{
	// Only used for sending UDP packets
	sock = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
	if (sock == -1)
	{
		printf("Create raw socket failed\n");
		return FAIL;
	}

	struct sockaddr_in srvaddr;
	bzero(&srvaddr, sizeof(srvaddr));

	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(Fake_Src_Port);
	srvaddr.sin_addr.s_addr = htonl((u32)Fake_Src_Ip);

	// bind address and port to socket
	if ((bind(sock, (struct sockaddr *)&srvaddr, sizeof(srvaddr))) < 0)
	{
		printf("Bind raw socket failed, errno %d: %s\n", errno, strerror(errno));
		return FAIL;
	}

	//
	// let the kernel know the IP header is included in the data
	//
	
	int tmp = 1;
	if (setsockopt(sock, 0, IP_HDRINCL, &tmp, sizeof(tmp)) < 0)
	{
		printf("set IP_HDRINCL for raw socket failed\n");
		return FAIL;			
	}
	
	return SUCCESS;
}

//////////////////////////////////////////
// The buf format is: 
// |-----ipheader-----+----udpheader-----+--------payload-------|
//
// The input of buf is start, while the len parameter is the payload length.
//
int RawSocket::Send(unsigned char* buf, u32 len, u32 dest_ip, u16 dest_port)
{
	u32 total_len = 0;
	struct sockaddr_in to;
	struct iphdr * ip_header;
	struct udphdr * udp_header;
	int ret;
	unsigned char *temp = buf;

	// The target server ip and address
	bzero((char *)&to, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_port = htons(dest_port);
	to.sin_addr.s_addr = htonl(dest_ip);
	
	static u16 ip_id_gen = 1;
	total_len = sizeof(struct iphdr) + sizeof(struct udphdr) + len;	
	
	//	
	// ip header filling
	//
	ip_header = (struct iphdr *) buf;
	ip_header->ihl = 5;
	ip_header->version = 4;
	ip_header->tos = 0;

	// ip length contains the total length of ip header, tcp/udp header and payload in bytes.
	ip_header->tot_len = htons(total_len);
	ip_header->id = 0; // the value doesnt matter here
	ip_header->ttl = 64;
	ip_header->frag_off = 0x40;
	ip_header->protocol = IPPROTO_UDP;
	ip_header->check = 0; 
	ip_header->daddr = htonl(dest_ip);

	// fake ip address, may different with ServerIPAddr.
	ip_header->saddr = htonl(Fake_Src_Ip); 
	
	//
	// udp header filling
	//
	udp_header = (struct udphdr *) (buf + sizeof(struct iphdr));	

	// fake port, may different with ServerPort.
	udp_header->source = htons(Fake_Src_Port);
	udp_header->dest = htons(dest_port);

	// the length of udp header and payload data in bytes.
	udp_header->len = htons(sizeof(struct udphdr) + len);  
	udp_header->check = 0;
	
	//udp_header->check = (udp_checksum((unsigned short *)udp_header, sizeof(struct udphdr) + len, Fake_Src_Ip, dest_ip));

	if(total_len <= MTU)
	{

		ret = sendto(sock, buf, total_len, 0, (struct sockaddr *)&to, sizeof(to));
		if (ret < 0)
		{
			printf("RawSocket sendto ERROR : return value %d, %d, %s\n", ret, errno, strerror(errno));
	    		return ret;
		}

		return ret;
	}
	else
	{
		/* The fields which may be affected by fragmentation include(from RFC791):
		 (1) options field(not for us, no option)
		 (2) more fragments flag (yes)
		 (3) fragment offset (yes)
		 (4) internet header length field (not for us, fixed 20)
		 (5) total length field (yes, each fragments shall update it's total length)
		 (6) header checksum (yes)
		*/
		// Need to fragement, since packet length greater than MTU size
		// Must be able to put at least 8 bytes per fragment.
		unsigned short offset = 0;
		int nfrags;
		const int hlen = 20; // Length of IP header.
		u32 send_len = 0;

		/* Note that we must never set id to 0 here (or reuse one from
		 * 'buf' which might be 0), since that means letting the stack
		 * allocate an ID for us -- different ones for different
		 * fragments.
		 * See above for other caveats. XXX
		 */
		ip_header->id = ip_id_gen;
		++ip_id_gen;
		if(!ip_id_gen) ++ip_id_gen;

		ip_header->frag_off = htons((offset >> 3) |  0x2000);// More fragments.
		ip_header->tot_len = htons(MTU);
		// XXX Most likely doesn't need to be filled in -- see the raw(7) man page.
		ip_header->check = 0;
		udp_header->check = 0;

		ret = sendto(sock, buf, MTU, 0, (struct sockaddr *)&to, sizeof(to));
		if (ret < 0)
		{
		    printf("RawSocket sendto ERROR : return value %d, %d, %s\n", ret, errno, strerror(errno));
		    return ret;
		}
		send_len = ret;
		

		u32 len = 1480; // NFB * 8
		temp = buf + len;
		// take care of the rest fragements
		// offset is the offset against ip header front other than data front.
		// len is the data length(ip head not included) of current fragment.
		for (nfrags = 1, offset = hlen + len; offset < total_len; offset += len, temp += len, nfrags++)
		{
			if (offset + len >= total_len)
			{
				// last fragment
				len = total_len - offset;
				ip_header->frag_off = htons((offset - hlen) >> 3);
			}
			else
			{
				ip_header->frag_off = htons(((offset - hlen) >> 3) |  0x2000);
			}
			ip_header->check = 0;
			ip_header->tot_len = htons(len + hlen);
			memcpy(temp, (unsigned char *)ip_header, sizeof(struct iphdr));
			ret = sendto(sock, temp, len + hlen, 0, (struct sockaddr *)&to, sizeof(to));
			if (ret < 0)
			{
				printf("RawSocket sendto ERROR : return value %d, %d, %s\n", ret, errno, strerror(errno));
		    		return ret;
			}
			send_len += ret;
		}
		// Sending of fragmented packet should end up here.
		// Which result of sendto should we return?
		return send_len;
	}
    
}




#include "CheckSum.h"

inline int chksumAdjust(unsigned char *chksum,
                        unsigned char *optr,
                        int olen, 
                        unsigned char *nptr,
                        int nlen)
{
    long x, oldVal, newVal;

    /* Return false if length are not even */
    if ( (olen % 2 ) || ( nlen % 2  ) )
        return(0);

    x=(chksum[0] << 8) + chksum[1];
    x=~x & 0xFFFF;
    while (olen)
    {
        oldVal=(optr[0] << 8) +optr[1]; 
        optr+=2;
        x-=oldVal & 0xffff;
        if (x<=0) 
        { 
            x--; 
            x&=0xffff;
        }
        olen-=2;
    }
    while (nlen)
    {
        newVal=(nptr[0] << 8)+nptr[1];
        nptr+=2;
        x+=newVal & 0xffff;
        if (x & 0x10000) 
        { 
            x++; 
            x&=0xffff; 
        }
        nlen-=2;
    }
    x = ~x & 0xFFFF;
    chksum[0] = (unsigned char) (x >> 8); 
    chksum[1] = (unsigned char) x & 0xff;

    return 1;
}

unsigned short ip_sum_calc(unsigned short length, unsigned short *buf)
{
	unsigned int cksum = 0;
	unsigned int nbytes = length;

	assert(length%2 == 0);

	for (; nbytes > 1; nbytes -= 2)
	{
		cksum += *buf++;
	}

	while (cksum >> 16)
	{
		cksum = (cksum >> 16) + (cksum & 0xffff);
	}

	return (unsigned short) (~cksum);
}

unsigned short udp_checksum(unsigned short *buffer, unsigned short size, unsigned int src_addr,
        unsigned int dest_addr)
{
	
	unsigned int cksum = 0;
	const u16 protocol_udp = 17;
	u16 checksum;
	// Add UDP pseudo header.
	struct pseudo_header_
	{
		unsigned int src;
		unsigned int dst;
		unsigned char zero;
		unsigned char proto;
		unsigned short length;
	} header;

	header.dst = htonl(dest_addr);
	header.src = htonl(src_addr);
	header.zero = 0;
	header.proto = protocol_udp;
	header.length = htons(size);
	unsigned short *pseudo_header_p = (unsigned short *) &header;
	for (u32 i = 0; i < sizeof(header); i += 2)
	{
		cksum = cksum + *pseudo_header_p;
		pseudo_header_p++;
	}
	while (size > 1)
	{
		cksum += *buffer++;
		size -= 2;
	}
	if (size)
	{
		unsigned char last_word[2] = { *(unsigned char*) buffer, 0 };
		cksum += *((unsigned short*) last_word);
	}
	while (cksum >> 16)
	{
		cksum = (cksum >> 16) + (cksum & 0xffff);
	}
	
	checksum  = (unsigned short) (~cksum);
	
	if (checksum == 0)
	{
		checksum = 0xffff;
	}

	return checksum;
}



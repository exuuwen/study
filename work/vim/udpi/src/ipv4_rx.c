/*-
 *
 * Copyright(c) UCloud. All rights reserved.
 * All rights reserved.
 *
 */

#include <rte_malloc.h>
#include <rte_lcore.h>
#include <rte_port.h>
#include <rte_cycles.h>
#include <rte_byteorder.h>
#include <rte_ether.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_jhash.h>

#include <unistd.h>

#include "main.h"
#include "stats.h"
#include "ipaddr_map.h"

/*
#if REGIONS == DB_XJ
#include "ipaddr_map_contry.c"
#else
#include "ipaddr_map_isp.c"
#endif
*/

extern struct addr_map group[];

#if REGIONS == DB_XJ
static void udpi_find_remote_type(uint32_t addr, uint8_t *region_type, uint8_t *operator_type)
{

	if (addr < group[0].start || addr > group[UDPI_NUM_ADDR - 1].end)
	{
		*region_type = UDPI_REGION_STATS - 1;   
		*operator_type = UDPI_OPERATOR_STATS -1;       
		return;
	}

	int low = 0;
	int high = UDPI_NUM_ADDR - 1;

	while (low <= high)
	{
		int middle = (low + high)/2;
		if (group[middle].start == addr)
		{
			*region_type = group[middle].region;
			*operator_type = group[middle].isp;        
			return;
		}
		else if(group[middle].start > addr)
		{
			high = middle - 1;
		}
		else
		{
			low = middle + 1;
		}
	}

	if (addr > group[high].end)
	{
		*region_type = UDPI_REGION_STATS - 1;
		*operator_type = UDPI_OPERATOR_STATS - 1;
	}
	else
	{
		*region_type = group[high].region;      
		*operator_type = group[high].isp;  
	}
}
#else
static void udpi_find_remote_type(uint32_t addr, uint8_t *region_type, uint8_t *operator_type)
{
	if (addr < UDPI_FIRST_IP || addr > UDPI_LAST_IP)
	{
		*region_type = UDPI_REGION_UNKNOW;   
		*operator_type = UDPI_OPERATOR_UNKNOW;       
		return;
	}

	uint32_t slot = ((addr&0xffffff00) - UDPI_FIRST_IP)>>8;
	/*if (slot >= UDPI_NUM_ADDR)
	{
		*region_type = UDPI_REGION_UNKNOW;   
		*operator_type = UDPI_OPERATOR_UNKNOW;       
		return;
	}*/

	uint8_t region = group[slot].region;
	uint8_t isp = group[slot].isp;

	if (region >= UDPI_REGION_STATS)
		region = UDPI_REGION_UNKNOW;

	if (isp >= UDPI_OPERATOR_STATS)
		isp = UDPI_OPERATOR_UNKNOW;

	*region_type = region;
	*operator_type = isp;
}
#endif

static int udpi_find_dir(uint32_t eip, uint8_t *packet_type)
{
	int ret;

	uint32_t key = eip;
	uint8_t *type;
	ret = rte_hash_lookup_data(udpi.addr_hash[udpi.hash_id], (const void*)&key, (void**)&type);    
	if (ret < 0)
	{   
		return -1;
	}   

	if (*type < 2)
		*packet_type = *type;
	else
		*packet_type = 0;

	return 0;
}

static void udpi_find_comp(uint32_t rip, uint8_t *comp_type)
{
	int ret;

	uint32_t key = rip&0xffffff00;
	uint8_t *type;
	ret = rte_hash_lookup_data(udpi.comp_addr_hash[udpi.comp_hash_id], (const void*)&key, (void**)&type);    
	if (ret < 0)
	{   
		*comp_type = UDPI_COMP_MAX;
		return;
	}   

	if (*type < UDPI_COMP_MAX)
		*comp_type = *type;
	else
		*comp_type = UDPI_COMP_MAX;

	return;
}


static inline int32_t udpi_pkt_metadata_fill(struct rte_mbuf *m, uint32_t *hashed, uint8_t *packet, struct udpi_local_stats *stats)
{
	int32_t is_next_do_analysis=1;
	uint8_t *m_data = rte_pktmbuf_mtod(m, uint8_t *);//m 数据buff
	struct udpi_pkt_metadata *c = (struct udpi_pkt_metadata *) RTE_MBUF_METADATA_UINT8_PTR(m, UDPI_METADATA_OFFSET(0));
	uint8_t port_type;
	struct ether_hdr *eth_hdr = (struct ether_hdr *)m_data;

again:
	if  (eth_hdr->ether_type == rte_cpu_to_be_16(ETHER_TYPE_VLAN))
	{
		rte_vlan_strip(m);	
		m_data = rte_pktmbuf_mtod(m, uint8_t *);
		eth_hdr = (struct ether_hdr *)m_data;		
		goto again;
	}

	if (eth_hdr->ether_type != rte_cpu_to_be_16(ETHER_TYPE_IPv4))
	{
		is_next_do_analysis= -1;
		return is_next_do_analysis;
	}


	
	struct ipv4_hdr *ip_hdr = (struct ipv4_hdr *)(eth_hdr + 1); 

	/* TTL and Header Checksum are set to 0 */
	c->ip_src = rte_be_to_cpu_32(ip_hdr->src_addr);
	c->ip_dst = rte_be_to_cpu_32(ip_hdr->dst_addr);
	c->proto = ip_hdr->next_proto_id;

	
	uint8_t packet_type;
	int ret = udpi_find_dir(c->ip_src, &packet_type);
	if (ret < 0)
	{
		ret = udpi_find_dir(c->ip_dst, &packet_type);
		if (ret < 0)
		{
			uint8_t line_num = 0;
			uint8_t is_inline = 0;
			for (line_num = 0; line_num < UDPI_LINE_STATS; line_num++) 
			{
				uint32_t src_mac = 0;
				uint32_t dst_mac = 0;
				if(1==udpi.line_src_judge[line_num])
				{
					src_mac = *((uint32_t*)&LINE_MAC_S_ADD.addr_bytes[0]);
					dst_mac = *((uint32_t*)&LINE_MAC_D_ADD.addr_bytes[0]);
				}
				else
				{
					src_mac = *((uint32_t*)&LINE_MAC_D_ADD.addr_bytes[0]);
					dst_mac = *((uint32_t*)&LINE_MAC_S_ADD.addr_bytes[0]);
				}

				if(udpi.line[line_num]==src_mac)
				{
					is_inline = 1;
					*packet = 0;
					uint8_t comp_type;
					udpi_find_comp(c->ip_src, &comp_type);
					c->comp_type = comp_type;
					port_type = UDPI_PORT_INPUT;//判断为入流量
					break;
				}
				else if(udpi.line[line_num]==dst_mac)
				{
					is_inline = 1;
					*packet = 0;
					uint8_t comp_type;
					udpi_find_comp(c->ip_dst, &comp_type);
					c->comp_type = comp_type;
					port_type = UDPI_PORT_OUTPUT;//判断为出流量
					break;
				}
				else
				{
					continue;
				}
			}			
			is_next_do_analysis =-1;
			if(0==is_inline)
			{
				return is_next_do_analysis;
			}
		}
		else 
		{
			*packet = packet_type;
			uint8_t comp_type;
			udpi_find_comp(c->ip_src, &comp_type);
			c->comp_type = comp_type;
			port_type = UDPI_PORT_INPUT;//判断为入流量
		}
	}
	else
	{
		*packet = packet_type;
		uint8_t comp_type;
		udpi_find_comp(c->ip_dst, &comp_type);
		c->comp_type = comp_type;
		port_type = UDPI_PORT_OUTPUT;//判断为出流量
	}
	//对端地址分类
	if (port_type == UDPI_PORT_INPUT)
	{
		udpi_find_remote_type(c->ip_src, &c->region_type, &c->operator_type);
	}
	else if (port_type == UDPI_PORT_OUTPUT)
	{
		udpi_find_remote_type(c->ip_dst, &c->region_type, &c->operator_type);
	}
	//line
	/*
	uint16_t line[UDPI_LINE_STATS][3]={{0x0c45,0xba77,0x1ff2},
						{0x0c45,0xbab9,0x3f42},
						{0x845b,0x123c,0xf682},
						{0xfce3,0x3ca9,0xa660},
						{0x74a0,0x63fc,0x2d60}
	};
	
	uint32_t line[UDPI_LINE_STATS]={0x77ba450c,
						0xb9ba450c,
						0x3c125b84,
						0xa93ce3fc,
						0xfc63a074
		};
	*/
	uint8_t line_num = 0;
	uint8_t is_inline = 0;
	if (port_type == UDPI_PORT_INPUT)
	{
		for (line_num = 0; line_num < UDPI_LINE_STATS; line_num++) 
		{

			//if(line[i]==*((uint32_t*)(&eth_hdr->s_addr.addr_bytes[0])))
			uint32_t src_mac = 0;
			if(1==udpi.line_src_judge[line_num])
			{
				src_mac = *((uint32_t*)&LINE_MAC_S_ADD.addr_bytes[0]);
			}
			else
			{
				src_mac = *((uint32_t*)&LINE_MAC_D_ADD.addr_bytes[0]);
			}
			if(udpi.line[line_num]==src_mac)
			{
				stats->line_stats[line_num].input_stats.l3.ip_pkts += 1;
				stats->line_stats[line_num].input_stats.l3.ip_bytes += m->pkt_len;

				stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].input_stats.l3.ip_pkts += 1;
				stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].input_stats.l3.ip_bytes += m->pkt_len;
				stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].seen = 1;
				is_inline = 1;
				break;
			}
		}

	}
	else if (port_type == UDPI_PORT_OUTPUT)
	{
		for (line_num = 0; line_num < UDPI_LINE_STATS; line_num++) 
		{
			//if(line[i]==*((uint32_t*)(&eth_hdr->d_addr.addr_bytes[0])))
			uint32_t dst_mac = 0;
			if(1==udpi.line_src_judge[line_num])
			{
				dst_mac = *((uint32_t*)&LINE_MAC_D_ADD.addr_bytes[0]);
			}
			else
			{
				dst_mac = *((uint32_t*)&LINE_MAC_S_ADD.addr_bytes[0]);
			}
			if(udpi.line[line_num]==dst_mac)
			{
				stats->line_stats[line_num].output_stats.l3.ip_pkts += 1;
				stats->line_stats[line_num].output_stats.l3.ip_bytes += m->pkt_len;

				stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].output_stats.l3.ip_pkts += 1;
				stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].output_stats.l3.ip_bytes += m->pkt_len;
				stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].seen = 1;
				is_inline = 1;
				break;
			}
		}
	}
	
	//3层
	if (port_type == UDPI_PORT_INPUT)
	{
		stats->input_stats.l3.ip_pkts += 1;
		stats->input_stats.l3.ip_bytes += m->pkt_len;

		stats->map_stats[c->region_type][c->operator_type].input_stats.l3.ip_pkts += 1;
		stats->map_stats[c->region_type][c->operator_type].input_stats.l3.ip_bytes += m->pkt_len;

		if (c->comp_type < UDPI_COMP_MAX)
		{
			stats->input_comp_stats[c->comp_type].ip_bytes += m->pkt_len;
		}
	}
	else if (port_type == UDPI_PORT_OUTPUT)
	{
		stats->output_stats.l3.ip_pkts += 1;
		stats->output_stats.l3.ip_bytes += m->pkt_len;

		stats->map_stats[c->region_type][c->operator_type].output_stats.l3.ip_pkts += 1;
		stats->map_stats[c->region_type][c->operator_type].output_stats.l3.ip_bytes += m->pkt_len;

		if (c->comp_type < UDPI_COMP_MAX)
		{
			stats->output_comp_stats[c->comp_type].ip_bytes += m->pkt_len;
		}
	}

	uint8_t hlen = (ip_hdr->version_ihl & 0xf) * 4;
	//4层
	if (c->proto == IPPROTO_UDP)
	{
		struct udp_hdr *uhr = (struct udp_hdr *)&m_data[sizeof(struct ether_hdr) + hlen];
		c->port_src = rte_be_to_cpu_16(uhr->src_port);
		c->port_dst = rte_be_to_cpu_16(uhr->dst_port);

		if (port_type == UDPI_PORT_INPUT)
		{
			stats->input_stats.l4.udp_pkts += 1;
			stats->input_stats.l4.udp_bytes += m->pkt_len;
			
			stats->map_stats[c->region_type][c->operator_type].input_stats.l4.udp_pkts += 1;
			stats->map_stats[c->region_type][c->operator_type].input_stats.l4.udp_bytes += m->pkt_len;
			if(is_inline)
			{
			stats->line_stats[line_num].input_stats.l4.udp_pkts += 1;
			stats->line_stats[line_num].input_stats.l4.udp_bytes += m->pkt_len;
			stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].input_stats.l4.udp_pkts += 1;
			stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].input_stats.l4.udp_bytes += m->pkt_len;
			}

		}
		else if (port_type == UDPI_PORT_OUTPUT)
		{
			stats->output_stats.l4.udp_pkts += 1;
			stats->output_stats.l4.udp_bytes += m->pkt_len;
			
			stats->map_stats[c->region_type][c->operator_type].output_stats.l4.udp_pkts += 1;
			stats->map_stats[c->region_type][c->operator_type].output_stats.l4.udp_bytes += m->pkt_len;
			if(is_inline)
			{
			stats->line_stats[line_num].output_stats.l4.udp_pkts += 1;
			stats->line_stats[line_num].output_stats.l4.udp_bytes += m->pkt_len;
			stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].output_stats.l4.udp_pkts += 1;
			stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].output_stats.l4.udp_bytes += m->pkt_len;
			}

		}
		c->payload_len = rte_be_to_cpu_16(ip_hdr->total_length) - sizeof(struct udp_hdr) - hlen;
	}
	else if (c->proto == IPPROTO_TCP)
	{
		struct tcp_hdr *thr = (struct tcp_hdr *)&m_data[sizeof(struct ether_hdr) + hlen];
		c->port_src = rte_be_to_cpu_16(thr->src_port);
		c->port_dst = rte_be_to_cpu_16(thr->dst_port);
	
		if (port_type == UDPI_PORT_INPUT)
		{
			stats->input_stats.l4.tcp_pkts += 1;
			stats->input_stats.l4.tcp_bytes += m->pkt_len;
			
			stats->map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_pkts += 1;
			stats->map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_bytes += m->pkt_len;
			if(is_inline)
			{
			stats->line_stats[line_num].input_stats.l4.tcp_pkts += 1;
			stats->line_stats[line_num].input_stats.l4.tcp_bytes += m->pkt_len;
			stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_pkts += 1;
			stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_bytes += m->pkt_len;
			}


			if (thr->tcp_flags == TCP_SYN_FLAG)	
			{
				stats->input_stats.l4.tcp_syn_pkts += 1;			
				stats->map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_syn_pkts += 1;
				if(is_inline)
				{
				stats->line_stats[line_num].input_stats.l4.tcp_syn_pkts += 1;
				stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].input_stats.l4.tcp_syn_pkts += 1;
				}
				
			}
		}
		else if (port_type == UDPI_PORT_OUTPUT)
		{
			stats->output_stats.l4.tcp_pkts += 1;
			stats->output_stats.l4.tcp_bytes += m->pkt_len;
			
			stats->map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_pkts += 1;
			stats->map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_bytes += m->pkt_len;
			if(is_inline)
			{
			stats->line_stats[line_num].output_stats.l4.tcp_pkts += 1;
			stats->line_stats[line_num].output_stats.l4.tcp_bytes += m->pkt_len;
			stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_pkts += 1;
			stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_bytes += m->pkt_len;
			}

			if (thr->tcp_flags == TCP_SYN_FLAG)	
			{
				stats->output_stats.l4.tcp_syn_pkts += 1;			
				stats->map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_syn_pkts += 1;
				if(is_inline)
				{
				stats->line_stats[line_num].output_stats.l4.tcp_syn_pkts += 1;
				stats->line_stats[line_num].map_stats[c->region_type][c->operator_type].output_stats.l4.tcp_syn_pkts += 1;
				}

			}
		}	
		c->payload_len = rte_be_to_cpu_16(ip_hdr->total_length) - ((thr->data_off & 0xf0) >> 2) - hlen; 
	}
	else
	{
		is_next_do_analysis =-1;
		return is_next_do_analysis;
	}
	//7层
	if (c->port_src == PROTOCOL_SSH || c->port_dst == PROTOCOL_SSH)
	{
		if (port_type == UDPI_PORT_INPUT)
		{
			stats->input_stats.l7.ssh += 1;
			stats->input_stats.l7.ssh_bytes += m->pkt_len;

			stats->map_stats[c->region_type][c->operator_type].input_stats.l7.ssh += 1;
			stats->map_stats[c->region_type][c->operator_type].input_stats.l7.ssh_bytes += m->pkt_len;
		}
		else if (port_type == UDPI_PORT_OUTPUT)
		{
			stats->output_stats.l7.ssh += 1;
			stats->output_stats.l7.ssh_bytes += m->pkt_len;

			stats->map_stats[c->region_type][c->operator_type].output_stats.l7.ssh += 1;
			stats->map_stats[c->region_type][c->operator_type].output_stats.l7.ssh_bytes += m->pkt_len;
		}
	}
	else if (c->port_src == PROTOCOL_HTTP || c->port_dst == PROTOCOL_HTTP ||  c->port_src == 8080 || c->port_dst == 8080)
	{
		if (port_type == UDPI_PORT_INPUT)
		{
			stats->input_stats.l7.http += 1;
			stats->input_stats.l7.http_bytes += m->pkt_len;

			stats->map_stats[c->region_type][c->operator_type].input_stats.l7.http += 1;
			stats->map_stats[c->region_type][c->operator_type].input_stats.l7.http_bytes += m->pkt_len;
		}
		else if (port_type == UDPI_PORT_OUTPUT)
		{
			stats->output_stats.l7.http += 1;
			stats->output_stats.l7.http_bytes += m->pkt_len;

			stats->map_stats[c->region_type][c->operator_type].output_stats.l7.http += 1;
			stats->map_stats[c->region_type][c->operator_type].output_stats.l7.http_bytes += m->pkt_len;
		}
	}
	else if (c->port_src == PROTOCOL_HTTPS || c->port_dst == PROTOCOL_HTTPS)
	{
		if (port_type == UDPI_PORT_INPUT)
		{
			stats->input_stats.l7.https += 1;
			stats->input_stats.l7.https_bytes += m->pkt_len;

			stats->map_stats[c->region_type][c->operator_type].input_stats.l7.https += 1;
			stats->map_stats[c->region_type][c->operator_type].input_stats.l7.https_bytes += m->pkt_len;
		}
		else if (port_type == UDPI_PORT_OUTPUT)
		{
			stats->output_stats.l7.https += 1;
			stats->output_stats.l7.https_bytes += m->pkt_len;

			stats->map_stats[c->region_type][c->operator_type].output_stats.l7.https += 1;
			stats->map_stats[c->region_type][c->operator_type].output_stats.l7.https_bytes += m->pkt_len;
		}
	}
	else
	{
		if (port_type == UDPI_PORT_INPUT)
		{
			stats->input_stats.l7.others += 1;
			stats->input_stats.l7.others_bytes += m->pkt_len;

			stats->map_stats[c->region_type][c->operator_type].input_stats.l7.others += 1;
			stats->map_stats[c->region_type][c->operator_type].input_stats.l7.others_bytes += m->pkt_len;
		}
		else if (port_type == UDPI_PORT_OUTPUT)
		{
			stats->output_stats.l7.others += 1;
			stats->output_stats.l7.others_bytes += m->pkt_len;

			stats->map_stats[c->region_type][c->operator_type].output_stats.l7.others += 1;
			stats->map_stats[c->region_type][c->operator_type].output_stats.l7.others_bytes += m->pkt_len;
		}
	}

	stats->map_stats[c->region_type][c->operator_type].seen = 1;
	c->port_type = port_type;
	c->l4_offset = (uint16_t)(sizeof(struct ether_hdr) + hlen);
	c->is_inline = is_inline;
	c->line_num = line_num;

	if (port_type == UDPI_PORT_INPUT)
	{
		*hashed = rte_jhash_1word(c->ip_dst, 0);
		
	}
	else if (port_type == UDPI_PORT_OUTPUT)
	{
		*hashed = rte_jhash_1word(c->ip_src, 0);
	}
	return is_next_do_analysis;
}

static void udpi_update_allstats(struct udpi_local_stats *stats)
{
	rte_atomic64_add(&(udpi.stats.input_stats.l3.ip_pkts), stats->input_stats.l3.ip_pkts);
	rte_atomic64_add(&(udpi.stats.output_stats.l3.ip_pkts), stats->output_stats.l3.ip_pkts);
	rte_atomic64_add(&(udpi.stats.input_stats.l3.ip_bytes), stats->input_stats.l3.ip_bytes);
	rte_atomic64_add(&(udpi.stats.output_stats.l3.ip_bytes), stats->output_stats.l3.ip_bytes);

	rte_atomic64_add(&(udpi.stats.input_stats.l4.tcp_pkts), stats->input_stats.l4.tcp_pkts);
	rte_atomic64_add(&(udpi.stats.output_stats.l4.tcp_pkts), stats->output_stats.l4.tcp_pkts);
	rte_atomic64_add(&(udpi.stats.input_stats.l4.tcp_bytes), stats->input_stats.l4.tcp_bytes);
	rte_atomic64_add(&(udpi.stats.output_stats.l4.tcp_bytes), stats->output_stats.l4.tcp_bytes);
	rte_atomic64_add(&(udpi.stats.input_stats.l4.tcp_syn_pkts), stats->input_stats.l4.tcp_syn_pkts);
	rte_atomic64_add(&(udpi.stats.output_stats.l4.tcp_syn_pkts), stats->output_stats.l4.tcp_syn_pkts);

	rte_atomic64_add(&(udpi.stats.input_stats.l4.udp_pkts), stats->input_stats.l4.udp_pkts);
	rte_atomic64_add(&(udpi.stats.output_stats.l4.udp_pkts), stats->output_stats.l4.udp_pkts);
	rte_atomic64_add(&(udpi.stats.input_stats.l4.udp_bytes), stats->input_stats.l4.udp_bytes);
	rte_atomic64_add(&(udpi.stats.output_stats.l4.udp_bytes), stats->output_stats.l4.udp_bytes);

	rte_atomic64_add(&(udpi.stats.input_stats.l7.ssh), stats->input_stats.l7.ssh);
	rte_atomic64_add(&(udpi.stats.output_stats.l7.ssh), stats->output_stats.l7.ssh);
	rte_atomic64_add(&(udpi.stats.input_stats.l7.ssh_bytes), stats->input_stats.l7.ssh_bytes);
	rte_atomic64_add(&(udpi.stats.output_stats.l7.ssh_bytes), stats->output_stats.l7.ssh_bytes);

	rte_atomic64_add(&(udpi.stats.input_stats.l7.http), stats->input_stats.l7.http);
	rte_atomic64_add(&(udpi.stats.output_stats.l7.http), stats->output_stats.l7.http);
	rte_atomic64_add(&(udpi.stats.input_stats.l7.http_bytes), stats->input_stats.l7.http_bytes);
	rte_atomic64_add(&(udpi.stats.output_stats.l7.http_bytes), stats->output_stats.l7.http_bytes);

	rte_atomic64_add(&(udpi.stats.input_stats.l7.https), stats->input_stats.l7.https);
	rte_atomic64_add(&(udpi.stats.output_stats.l7.https), stats->output_stats.l7.https);
	rte_atomic64_add(&(udpi.stats.input_stats.l7.https_bytes), stats->input_stats.l7.https_bytes);
	rte_atomic64_add(&(udpi.stats.output_stats.l7.https_bytes), stats->output_stats.l7.https_bytes);

	rte_atomic64_add(&(udpi.stats.input_stats.l7.others), stats->input_stats.l7.others);
	rte_atomic64_add(&(udpi.stats.output_stats.l7.others), stats->output_stats.l7.others);
	rte_atomic64_add(&(udpi.stats.input_stats.l7.others_bytes), stats->input_stats.l7.others_bytes);
	rte_atomic64_add(&(udpi.stats.output_stats.l7.others_bytes), stats->output_stats.l7.others_bytes);

	int i, j;
	for (i=0; i<UDPI_COMP_MAX; i++)
	{
		rte_atomic64_add(&(udpi.stats.input_comp_stats[i].ip_bytes), stats->input_comp_stats[i].ip_bytes);
		rte_atomic64_add(&(udpi.stats.output_comp_stats[i].ip_bytes), stats->output_comp_stats[i].ip_bytes);
	}

	for (i=0; i<UDPI_REGION_STATS; i++)
		for(j=0; j<UDPI_OPERATOR_STATS; j++)
		{
			if (stats->map_stats[i][j].seen)
			{
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l3.ip_pkts), stats->map_stats[i][j].input_stats.l3.ip_pkts);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l3.ip_pkts), stats->map_stats[i][j].output_stats.l3.ip_pkts);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l3.ip_bytes), stats->map_stats[i][j].input_stats.l3.ip_bytes);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l3.ip_bytes), stats->map_stats[i][j].output_stats.l3.ip_bytes);

				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l4.tcp_pkts), stats->map_stats[i][j].input_stats.l4.tcp_pkts);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l4.tcp_pkts), stats->map_stats[i][j].output_stats.l4.tcp_pkts);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l4.tcp_syn_pkts), stats->map_stats[i][j].input_stats.l4.tcp_syn_pkts);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l4.tcp_syn_pkts), stats->map_stats[i][j].output_stats.l4.tcp_syn_pkts);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l4.tcp_bytes), stats->map_stats[i][j].input_stats.l4.tcp_bytes);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l4.tcp_bytes), stats->map_stats[i][j].output_stats.l4.tcp_bytes);

				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l4.udp_pkts), stats->map_stats[i][j].input_stats.l4.udp_pkts);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l4.udp_pkts), stats->map_stats[i][j].output_stats.l4.udp_pkts);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l4.udp_bytes), stats->map_stats[i][j].input_stats.l4.udp_bytes);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l4.udp_bytes), stats->map_stats[i][j].output_stats.l4.udp_bytes);

				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l7.ssh), stats->map_stats[i][j].input_stats.l7.ssh);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l7.ssh), stats->map_stats[i][j].output_stats.l7.ssh);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l7.ssh_bytes), stats->map_stats[i][j].input_stats.l7.ssh_bytes);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l7.ssh_bytes), stats->map_stats[i][j].output_stats.l7.ssh_bytes);

				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l7.http), stats->map_stats[i][j].input_stats.l7.http);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l7.http), stats->map_stats[i][j].output_stats.l7.http);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l7.http_bytes), stats->map_stats[i][j].input_stats.l7.http_bytes);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l7.http_bytes), stats->map_stats[i][j].output_stats.l7.http_bytes);

				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l7.https), stats->map_stats[i][j].input_stats.l7.https);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l7.https), stats->map_stats[i][j].output_stats.l7.https);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l7.https_bytes), stats->map_stats[i][j].input_stats.l7.https_bytes);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l7.https_bytes), stats->map_stats[i][j].output_stats.l7.https_bytes);

				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l7.others), stats->map_stats[i][j].input_stats.l7.others);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l7.others), stats->map_stats[i][j].output_stats.l7.others);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].input_stats.l7.others_bytes), stats->map_stats[i][j].input_stats.l7.others_bytes);
				rte_atomic64_add(&(udpi.stats.map_stats[i][j].output_stats.l7.others_bytes), stats->map_stats[i][j].output_stats.l7.others_bytes);
			}
		}
	//line
	int k =0;
	for (k=0; k<UDPI_LINE_STATS; k++)
	{
		rte_atomic64_add(&(udpi.stats.line_stats[k].input_stats.l3.ip_pkts), stats->line_stats[k].input_stats.l3.ip_pkts);
		rte_atomic64_add(&(udpi.stats.line_stats[k].output_stats.l3.ip_pkts), stats->line_stats[k].output_stats.l3.ip_pkts);
		rte_atomic64_add(&(udpi.stats.line_stats[k].input_stats.l3.ip_bytes), stats->line_stats[k].input_stats.l3.ip_bytes);
		rte_atomic64_add(&(udpi.stats.line_stats[k].output_stats.l3.ip_bytes), stats->line_stats[k].output_stats.l3.ip_bytes);
		rte_atomic64_add(&(udpi.stats.line_stats[k].input_stats.l4.tcp_pkts), stats->line_stats[k].input_stats.l4.tcp_pkts);
		rte_atomic64_add(&(udpi.stats.line_stats[k].output_stats.l4.tcp_pkts), stats->line_stats[k].output_stats.l4.tcp_pkts);
		rte_atomic64_add(&(udpi.stats.line_stats[k].input_stats.l4.tcp_bytes), stats->line_stats[k].input_stats.l4.tcp_bytes);
		rte_atomic64_add(&(udpi.stats.line_stats[k].output_stats.l4.tcp_bytes), stats->line_stats[k].output_stats.l4.tcp_bytes);
		rte_atomic64_add(&(udpi.stats.line_stats[k].input_stats.l4.tcp_syn_pkts), stats->line_stats[k].input_stats.l4.tcp_syn_pkts);
		rte_atomic64_add(&(udpi.stats.line_stats[k].output_stats.l4.tcp_syn_pkts), stats->line_stats[k].output_stats.l4.tcp_syn_pkts);

		rte_atomic64_add(&(udpi.stats.line_stats[k].input_stats.l4.udp_pkts), stats->line_stats[k].input_stats.l4.udp_pkts);
		rte_atomic64_add(&(udpi.stats.line_stats[k].output_stats.l4.udp_pkts), stats->line_stats[k].output_stats.l4.udp_pkts);
		rte_atomic64_add(&(udpi.stats.line_stats[k].input_stats.l4.udp_bytes), stats->line_stats[k].input_stats.l4.udp_bytes);
		rte_atomic64_add(&(udpi.stats.line_stats[k].output_stats.l4.udp_bytes), stats->line_stats[k].output_stats.l4.udp_bytes);

		for (i=0; i<UDPI_REGION_STATS; i++)
		for(j=0; j<UDPI_OPERATOR_STATS; j++)
		{
			if (stats->line_stats[k].map_stats[i][j].seen)
			{
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].input_stats.l3.ip_pkts), stats->line_stats[k].map_stats[i][j].input_stats.l3.ip_pkts);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].output_stats.l3.ip_pkts), stats->line_stats[k].map_stats[i][j].output_stats.l3.ip_pkts);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].input_stats.l3.ip_bytes), stats->line_stats[k].map_stats[i][j].input_stats.l3.ip_bytes);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].output_stats.l3.ip_bytes), stats->line_stats[k].map_stats[i][j].output_stats.l3.ip_bytes);

				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].input_stats.l4.tcp_pkts), stats->line_stats[k].map_stats[i][j].input_stats.l4.tcp_pkts);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].output_stats.l4.tcp_pkts), stats->line_stats[k].map_stats[i][j].output_stats.l4.tcp_pkts);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].input_stats.l4.tcp_syn_pkts), stats->line_stats[k].map_stats[i][j].input_stats.l4.tcp_syn_pkts);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].output_stats.l4.tcp_syn_pkts), stats->line_stats[k].map_stats[i][j].output_stats.l4.tcp_syn_pkts);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].input_stats.l4.tcp_bytes), stats->line_stats[k].map_stats[i][j].input_stats.l4.tcp_bytes);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].output_stats.l4.tcp_bytes), stats->line_stats[k].map_stats[i][j].output_stats.l4.tcp_bytes);

				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].input_stats.l4.udp_pkts), stats->line_stats[k].map_stats[i][j].input_stats.l4.udp_pkts);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].output_stats.l4.udp_pkts), stats->line_stats[k].map_stats[i][j].output_stats.l4.udp_pkts);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].input_stats.l4.udp_bytes), stats->line_stats[k].map_stats[i][j].input_stats.l4.udp_bytes);
				rte_atomic64_add(&(udpi.stats.line_stats[k].map_stats[i][j].output_stats.l4.udp_bytes), stats->line_stats[k].map_stats[i][j].output_stats.l4.udp_bytes);
			}
		}
	}

}

void udpi_main_loop_ipv4_rx(void)
{
	uint32_t core_id = rte_lcore_id();
	
	struct udpi_core_params *core_params 
		= udpi_get_core_params(core_id);

	if((!core_params) 
		|| (core_params->core_type != UDPI_CORE_IPV4_RX))
	{
		rte_panic("Core %u misconfiguration\n", core_id);
		return;
	}

	RTE_LOG(INFO, USER1, 
		"Core %u is doing RX for port %u id %u\n", core_id, core_params->port_id, core_params->id);

	if (core_params->port_id >= udpi.n_ports)
	{
		rte_panic("Core %u misconfiguration port_id %u\n", core_id, core_params->port_id);
		return;
	}

	struct udpi_local_stats stats;
	struct rte_mbuf *pkts[udpi.bsz_hwq_rd];
	uint32_t j = 0;
	uint32_t n_mbufs = 0;
	
	memset(&stats, 0, sizeof(struct udpi_local_stats));
	uint64_t lcore_tsc_hz = rte_get_tsc_hz();
	uint64_t prev_tsc = rte_get_tsc_cycles();
	uint64_t cur_tsc = prev_tsc, last_tsc = 0, interval = 0, diff_tsc = 0;
	char is_busy = false;
	uint64_t timer_precision = lcore_tsc_hz;
	uint32_t port_id = core_params->port_id;
	uint32_t queue_id = core_params->id;
	uint8_t packet_id = 0;

	while (1)
	{
		last_tsc = cur_tsc;
		cur_tsc = rte_get_tsc_cycles();
		interval = cur_tsc - last_tsc;
		if (is_busy)
		{
			core_params->core_handle_cycles += interval;
		}
		core_params->core_total_cycles += interval;
		
		is_busy = false;

		diff_tsc = cur_tsc - prev_tsc;
		if (unlikely(diff_tsc >= timer_precision)) {
			is_busy = true;
			prev_tsc = cur_tsc;
			udpi_update_allstats(&stats);
			memset(&stats, 0, sizeof(struct udpi_local_stats));
		}

		n_mbufs = rte_eth_rx_burst(port_id, queue_id, pkts, udpi.bsz_hwq_rd);//获取包
		if(n_mbufs < 1)
		{
			continue;
		}
			
		is_busy = true;

		for(j = 0; j < n_mbufs; j++)
		{
			uint32_t hashed = 0;
			int dropped;
			uint8_t packet = 0;			
			
			dropped = udpi_pkt_metadata_fill(pkts[j], &hashed, &packet, &stats);//处理包
			if (dropped < 0)
				rte_pktmbuf_free(pkts[j]);
			else
			{
				int ret;
				if (packet)
				{
					packet_id = hashed % udpi.n_packets;
					struct rte_mbuf *mbuf = rte_pktmbuf_clone(pkts[j], udpi.pool[queue_id]);
					if (mbuf)
					{
						if ((ret = rte_ring_enqueue(udpi.packet_ring[packet_id], (void*)mbuf)) != 0)
						{
	 						rte_atomic64_inc(&udpi.overflow.packet_packet);
							rte_pktmbuf_free(mbuf);
						}
					}
					else
	 					rte_atomic64_inc(&udpi.overflow.packet_mem);
				}

				hashed %= udpi.n_workers;
				if ((ret = rte_ring_enqueue(udpi.rings[hashed], (void*)pkts[j])) != 0)
				{
	 				rte_atomic64_inc(&udpi.overflow.packet);
					rte_pktmbuf_free(pkts[j]);
				}
			}
		}
	}
	
	return;
}


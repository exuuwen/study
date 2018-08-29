/*-
 *
 * Copyright(c) UCloud. All rights reserved.
 * All rights reserved.
 *
 */

#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <rte_malloc.h>
#include <rte_lcore.h>
#include <rte_port.h>
#include <rte_cycles.h>

#include "main.h"
#include "stats.h"
#include "socket.h"

#include "ucloud.pb.h"
#include "unetanalysis.177000.178000.pb.h"


static ::ucloud::UMessage proto_req[UDPI_MAX_DUMPERS];
static char *proto_buf[UDPI_MAX_DUMPERS];

/**
 *  * 产生一个新消息
 *   * @param pMessage 输入输出参数
 *    * @return 正常返回*pMessage指针；NULL表示错误
 *     **/
static void *NewMessage(void *pMessage, 
    unsigned iFlowNo,                                       // 流水号
    const char *pSessionNo,                                 // 会话号
    int iMessageType,                                       // 消息类型
    int iWorkerIndex,                                       // 子进程ID
    bool bTintFlag,                                         // 染色标志
    unsigned iSourceEntity,                                 // 源实体号
    unsigned iDestEntity,                                   // 目的实体号
    const char *pCallPurpose,                               // 调用目的
    const char *pAccessToken,                               // 验证信息
    const char *pReserved                                   // 保留字段
    ) 
{  
    ::ucloud::UMessage *pInternalMessage = (pMessage)?((::ucloud::UMessage *)pMessage):(new ::ucloud::UMessage());

    ::ucloud::Head *pHead = pInternalMessage->mutable_head();
    pHead->set_version(1);
    pHead->set_magic_flag(0x12340987);
    pHead->set_random_num(random());
    pHead->set_flow_no(iFlowNo);
    pHead->set_session_no(pSessionNo);
    pHead->set_message_type(iMessageType);
    pHead->set_worker_index(iWorkerIndex);
    pHead->set_tint_flag(bTintFlag);
    pHead->set_source_entity(iSourceEntity);
    pHead->set_dest_entity(iDestEntity);
	if ( pCallPurpose ) {
        pHead->set_call_purpose(pCallPurpose);
    }
    if ( pAccessToken ) {
        pHead->set_access_token(pAccessToken);
    }
    if ( pReserved ) {
        pHead->set_reserved(pReserved);
    }

    return pInternalMessage;
}

static int EncodeMessage(const void *pMessage, char **pData, unsigned iSize) 
{
    const ::google::protobuf::Message *pMsg = (const ::google::protobuf::Message *)pMessage;
    int iRet = pMsg->ByteSize();
    if ( *pData && iSize < (iRet + sizeof(unsigned)) ) {
        return -100;
    }
    if ( !(*pData) ) {
        *pData = new char[iRet + sizeof(unsigned)];
    }

    pMsg->SerializeToArray(*pData + sizeof(unsigned), iRet);
    *(unsigned *)(*pData) = htonl(iRet);
    return iRet + sizeof(unsigned);
}

static uint32_t ByteSize(const void *pMessage)
{
        const ucloud::UMessage *pMsg = (const ucloud::UMessage *)pMessage;
       
        return pMsg->ByteSize();
}

static void udpi_map_eipstats_get(::ucloud::unetanalysis::StatsData *msg, struct eip_entry *entry, uint8_t item, char *eip, uint8_t region, uint8_t isp, time_t t)
{
 	msg->set_uuid(eip);
	msg->set_item_id(item);
	msg->set_region_id(region);
	msg->set_isp_id(isp);
	msg->set_is_regional(1);
	msg->set_value(0);
	msg->set_time(t);

	char key[2];
	memset(key, 0, sizeof(key));
	sprintf(key, "0");
	msg->set_no_regional_key(key, sizeof(key));

	switch (item) {
	case 0:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l3.ip_pkts);
		break;
	case 1:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l3.ip_bytes);
		break;
	case 2:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l4.tcp_pkts);
		break;
	case 3:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l4.tcp_bytes);
		break;
	case 4:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l4.udp_pkts);
		break;
	case 5:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l4.udp_bytes);
		break;
	case 6:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l7.ssh);
		break;
	case 7:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l7.ssh_bytes);
		break;
	case 8:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l7.http);
		break;
	case 9:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l7.http_bytes);
		break;
	case 10:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l7.https);
		break;
	case 11:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l7.https_bytes);
		break;
	case 12:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l7.others);
		break;
	case 13:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l7.others_bytes);
		break;
	case 14:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l4.tcp_retransmit_pkts);
		break;
	case 15:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l4.tcp_retransmit_bytes);
		break;
	case 16:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l3.ip_pkts);
		break;
	case 17:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l3.ip_bytes);
		break;
	case 18:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l4.tcp_pkts);
		break;
	case 19:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l4.tcp_bytes);
		break;
	case 20:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l4.udp_pkts);
		break;
	case 21:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l4.udp_bytes);
		break;
	case 22:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l7.ssh);
		break;
	case 23:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l7.ssh_bytes);
		break;
	case 24:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l7.http);
		break;
	case 25:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l7.http_bytes);
		break;
	case 26:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l7.https);
		break;
	case 27:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l7.https_bytes);
		break;
	case 28:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l7.others);
		break;
	case 29:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l7.others_bytes);
		break;
	case 30:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l4.tcp_retransmit_pkts);
		break;
	case 31:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l4.tcp_retransmit_bytes);
		break;
	case 32:
		msg->set_value(entry->stats.map_stats[region][isp].flows_stats.flow_entries);
		break;
	case 33:
		msg->set_value(entry->stats.map_stats[region][isp].flows_stats.closed_flow_entries);
		break;
	case 34:
		msg->set_value(entry->stats.map_stats[region][isp].flows_stats.tcp_flow_entries);
		break;
	case 35:
		msg->set_value(entry->stats.map_stats[region][isp].flows_stats.tcp_closed_flow_entries);
		break;
	case 36:
		msg->set_value(entry->stats.map_stats[region][isp].flows_stats.tcp_rst_flow_entries);
		break;
	case 37:
		msg->set_value(entry->stats.map_stats[region][isp].flows_stats.udp_flow_entries);
		break;
	case 38:
		msg->set_value(entry->stats.map_stats[region][isp].flows_stats.udp_closed_flow_entries);
		break;
	case 39:
		msg->set_value(entry->stats.map_stats[region][isp].delay_stats.tcp_delay_flows);
		break;
	case 40:
		msg->set_value(entry->stats.map_stats[region][isp].delay_stats.tcp_delay_ms);
		break;
	case 41:
		msg->set_value(entry->stats.map_stats[region][isp].input_stats.l4.tcp_syn_pkts);
		break;
	case 42:
		msg->set_value(entry->stats.map_stats[region][isp].output_stats.l4.tcp_syn_pkts);
		break;
	}
}

static void udpi_push_map_eipstats(struct eip_entry* entry, time_t t, uint8_t region, uint8_t isp, uint32_t dumper_id)
{
	uint32_t addr;
	uint8_t k;

	addr = entry->eip.eip;
	char ip[16];
	memset(ip, 0, sizeof(ip));

	unsigned char *p = (unsigned char*)&addr;
	sprintf(ip, "%u.%u.%u.%u", p[3], p[2], p[1], p[0]);

	proto_req[dumper_id].clear_body();
	::ucloud::unetanalysis::RecordStatsRequest* uir = proto_req[dumper_id].mutable_body()->MutableExtension(::ucloud::unetanalysis::record_stats_request);
	uir->set_dpdk_id(udpi.dpdk_id);
	
	for (k=0; k<UDPI_EIP_MAP_STATS_ITEMS; k++)
	{
		::ucloud::unetanalysis::StatsData* msg = uir->add_stats_data_list();
		udpi_map_eipstats_get(msg, entry, k, ip, region, isp, t);
	}

	uint32_t length = ByteSize(&proto_req[dumper_id]) + sizeof(uint32_t);
	int32_t size;

	proto_buf[dumper_id] = new char[length];
         
	size = EncodeMessage(&proto_req[dumper_id], &proto_buf[dumper_id], length);
	if (size <= 0) 
	{
		printf("Message encode fail\n");
		return;
 	}

	if (udpi.dumper_fds[dumper_id] > 0)
		udpi_socket_dumper_write(dumper_id, (uint8_t *)proto_buf[dumper_id], length);

	delete proto_buf[dumper_id];
}

static void udpi_whole_eipstats_get(::ucloud::unetanalysis::StatsData *msg, struct eip_entry *entry, uint8_t item, char *eip, time_t t, uint32_t k)
{
	msg->set_uuid(eip);
	msg->set_item_id(item);
	msg->set_region_id(0);
	msg->set_isp_id(0);
	msg->set_is_regional(1);
	msg->set_value(0);
	msg->set_time(t);
	
	char key[11];
	memset(key, 0, sizeof(key));
	sprintf(key, "0");

	switch (item) {
	case 0:
		msg->set_value(entry->stats.input_stats.l3.ip_pkts);
		break;
	case 1:
		msg->set_value(entry->stats.input_stats.l3.ip_bytes);
		break;
	case 2:
		msg->set_value(entry->stats.input_stats.l4.tcp_pkts);
		break;
	case 3:
		msg->set_value(entry->stats.input_stats.l4.tcp_bytes);
		break;
	case 4:
		msg->set_value(entry->stats.input_stats.l4.udp_pkts);
		break;
	case 5:
		msg->set_value(entry->stats.input_stats.l4.udp_bytes);
		break;
	case 6:
		msg->set_value(entry->stats.input_stats.l7.ssh);
		break;
	case 7:
		msg->set_value(entry->stats.input_stats.l7.ssh_bytes);
		break;
	case 8:
		msg->set_value(entry->stats.input_stats.l7.http);
		break;
	case 9:
		msg->set_value(entry->stats.input_stats.l7.http_bytes);
		break;
	case 10:
		msg->set_value(entry->stats.input_stats.l7.https);
		break;
	case 11:
		msg->set_value(entry->stats.input_stats.l7.https_bytes);
		break;
	case 12:
		msg->set_value(entry->stats.input_stats.l7.others);
		break;
	case 13:
		msg->set_value(entry->stats.input_stats.l7.others_bytes);
		break;
	case 14:
		msg->set_value(entry->stats.input_stats.l4.tcp_retransmit_pkts);
		break;
	case 15:
		msg->set_value(entry->stats.input_stats.l4.tcp_retransmit_bytes);
		break;
	case 16:
		msg->set_value(entry->stats.output_stats.l3.ip_pkts);
		break;
	case 17:
		msg->set_value(entry->stats.output_stats.l3.ip_bytes);
		break;
	case 18:
		msg->set_value(entry->stats.output_stats.l4.tcp_pkts);
		break;
	case 19:
		msg->set_value(entry->stats.output_stats.l4.tcp_bytes);
		break;
	case 20:
		msg->set_value(entry->stats.output_stats.l4.udp_pkts);
		break;
	case 21:
		msg->set_value(entry->stats.output_stats.l4.udp_bytes);
		break;
	case 22:
		msg->set_value(entry->stats.output_stats.l7.ssh);
		break;
	case 23:
		msg->set_value(entry->stats.output_stats.l7.ssh_bytes);
		break;
	case 24:
		msg->set_value(entry->stats.output_stats.l7.http);
		break;
	case 25:
		msg->set_value(entry->stats.output_stats.l7.http_bytes);
		break;
	case 26:
		msg->set_value(entry->stats.output_stats.l7.https);
		break;
	case 27:
		msg->set_value(entry->stats.output_stats.l7.https_bytes);
		break;
	case 28:
		msg->set_value(entry->stats.output_stats.l7.others);
		break;
	case 29:
		msg->set_value(entry->stats.output_stats.l7.others_bytes);
		break;
	case 30:
		msg->set_value(entry->stats.output_stats.l4.tcp_retransmit_pkts);
		break;
	case 31:
		msg->set_value(entry->stats.output_stats.l4.tcp_retransmit_bytes);
		break;
	case 32:
		msg->set_value(entry->stats.flows_stats.flow_entries);
		break;
	case 33:
		msg->set_value(entry->stats.flows_stats.closed_flow_entries);
		break;
	case 34:
		msg->set_value(entry->stats.flows_stats.tcp_flow_entries);
		break;
	case 35:
		msg->set_value(entry->stats.flows_stats.tcp_closed_flow_entries);
		break;
	case 36:
		msg->set_value(entry->stats.flows_stats.tcp_rst_flow_entries);
		break;
	case 37:
		msg->set_value(entry->stats.flows_stats.udp_flow_entries);
		break;
	case 38:
		msg->set_value(entry->stats.flows_stats.udp_closed_flow_entries);
		break;
	case 39:
		msg->set_value(entry->stats.delay_stats.tcp_delay_flows);
		break;
	case 40:
		msg->set_value(entry->stats.delay_stats.tcp_delay_ms);
		break;
	case 41:
		msg->set_value(entry->stats.input_stats.l4.tcp_syn_pkts);
		break;
	case 42:
		msg->set_value(entry->stats.output_stats.l4.tcp_syn_pkts);
		break;
	case 43:
		msg->set_value(entry->stats.http_stats.http_delay_flows);
		break;
	case 44:
		msg->set_value(entry->stats.http_stats.http_delay_ms);
		break;
	case 45:
		msg->set_value(entry->stats.http_stats.http_return[0]);
 		break;
	case 46:
		msg->set_value(entry->stats.http_stats.http_return[1]);
		break;
	case 47:
		msg->set_value(entry->stats.http_stats.http_return[2]);
		break;
	case 48:
		msg->set_value(entry->stats.http_stats.http_return[3]);
		break;
	case 49:
		msg->set_value(entry->stats.http_stats.http_return[4]);
		break;
	case 50:
		sprintf(key, "%u", k);
		msg->set_is_regional(0);
		msg->set_value(entry->stats.port_stats->port_list[k]);	
		msg->set_no_regional_key(key, sizeof(key));
		return;
	case 51:
		sprintf(key, "%u", k);
		msg->set_is_regional(0);
		msg->set_value(entry->stats.http_stats.http_code[k]);	
		msg->set_no_regional_key(key, sizeof(key));
		return;
	case 52:
		msg->set_value((entry->stats.input_comp_stats[0].ip_bytes));
		break;
	case 53:
		msg->set_value((entry->stats.output_comp_stats[0].ip_bytes));
		break;
	case 54:
		msg->set_value((entry->stats.input_comp_stats[1].ip_bytes));
		break;
	case 55:
		msg->set_value((entry->stats.output_comp_stats[1].ip_bytes));
		break;
	case 56:
		msg->set_value((entry->stats.input_comp_stats[2].ip_bytes));
		break;
	case 57:
		msg->set_value((entry->stats.output_comp_stats[2].ip_bytes));
		break;
	}
	
	msg->set_no_regional_key(key, sizeof(key));
}

static void udpi_push_whole_eipstats(struct eip_entry *entry, time_t t, uint32_t dumper_id)
{
	uint32_t addr;
	uint8_t k;
	uint32_t j;

	addr = entry->eip.eip;
	char ip[16];
	memset(ip, 0, sizeof(ip));

	unsigned char *p = (unsigned char*)&addr;
	sprintf(ip, "%u.%u.%u.%u", p[3], p[2], p[1], p[0]);

		
	proto_req[dumper_id].clear_body();
	::ucloud::unetanalysis::RecordStatsRequest* uir = proto_req[dumper_id].mutable_body()->MutableExtension(::ucloud::unetanalysis::record_stats_request);
	uir->set_dpdk_id(udpi.dpdk_id);

	for (k=0; k<UDPI_EIP_STATS_ITEMS-2; k++)
	{
		::ucloud::unetanalysis::StatsData* msg = uir->add_stats_data_list();
		udpi_whole_eipstats_get(msg, entry, k, ip, t, 0);
	}

	for (j=1; j<65536; j++)
	{
		if (entry->stats.port_stats->port_list_seen[j])
		{
			::ucloud::unetanalysis::StatsData* msg = uir->add_stats_data_list();
			udpi_whole_eipstats_get(msg, entry, UDPI_EIP_STATS_ITEMS - 2, ip, t, j);
			entry->stats.port_stats->port_list_seen[j] = 0;
		}
	}

	for (j=100; j<600; j++)
	{
		if (entry->stats.http_stats.http_code_seen[j])
		{
			::ucloud::unetanalysis::StatsData* msg = uir->add_stats_data_list();
			udpi_whole_eipstats_get(msg, entry, UDPI_EIP_STATS_ITEMS - 1, ip, t, j);
			entry->stats.http_stats.http_code_seen[j] = 0;
		}
	}

	for (k=52; k<58; k++)
	{
		::ucloud::unetanalysis::StatsData* msg = uir->add_stats_data_list();
		udpi_whole_eipstats_get(msg, entry, k, ip, t, 0);
	}

	uint32_t length = ByteSize(&proto_req[dumper_id]) + sizeof(uint32_t);
	int32_t size;
         
	proto_buf[dumper_id] = new char[length];

	size = EncodeMessage(&proto_req[dumper_id], &proto_buf[dumper_id], length);
	if (size <= 0) 
	{
		printf("Message encode fail\n");
		return;
	}

	if (udpi.dumper_fds[dumper_id] > 0)
		udpi_socket_dumper_write(dumper_id, (uint8_t*)proto_buf[dumper_id], length);

	delete proto_buf[dumper_id];
}


static void udpi_push_eipstats(uint32_t worker_id, time_t t, uint32_t dumper_id)
{
	struct eip_entry *entry = udpi.hash[worker_id].eip_entry;
	uint32_t i;

	for(i=0; i<udpi.eip_params.entries; i++)
	{
		if (entry->eip.eip && entry->stats.seen)
		{
			entry->stats.seen = 0;
			udpi_push_whole_eipstats(entry, t, dumper_id);
			uint8_t k, j;
			for (k=1; k<UDPI_REGION_STATS; k++)
				for (j=1; j<UDPI_OPERATOR_STATS; j++)
				{
					if (entry->stats.map_stats[k][j].seen)
					{
						entry->stats.map_stats[k][j].seen = 0;
						udpi_push_map_eipstats(entry, t, k, j, dumper_id);
					}
				}
		}

		entry ++;
	}
}

static void udpi_push_all_eip(time_t t, uint32_t dumper_id)
{
	int n = udpi.n_workers/udpi.n_dumpers;
	int i;

	for (i=0; i<n; i++)
	{
		udpi_push_eipstats(n*dumper_id + i, t, dumper_id);
	}
}
static void udpi_line_mapstats_get(::ucloud::unetanalysis::StatsData *msg, uint8_t item, uint8_t region, uint8_t isp, time_t t,uint8_t line_num)
{
	char uuid[128];
	memset(uuid, 0, 128);
	sprintf(uuid, "line_%u", line_num);
	msg->set_uuid(uuid);
	msg->set_item_id(item);
	msg->set_region_id(region);
	msg->set_isp_id(isp);
	msg->set_is_regional(1);
	msg->set_value(0);
	msg->set_time(t);

	char key[2];
	memset(key, 0, sizeof(key));
	sprintf(key, "0");
	msg->set_no_regional_key(key, sizeof(key));

	switch (item) {
	case 0:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].input_stats.l3.ip_pkts));
		break;
	case 1:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].input_stats.l3.ip_bytes));
		break;
	case 2:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].input_stats.l4.tcp_pkts));
		break;
	case 3:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].input_stats.l4.tcp_bytes));
		break;
	case 4:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].input_stats.l4.udp_pkts));
		break;
	case 5:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].input_stats.l4.udp_bytes));
		break;
	case 14:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].input_stats.l4.tcp_retransmit_pkts));
		break;
	case 15:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].input_stats.l4.tcp_retransmit_bytes));
		break;
	case 16:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].output_stats.l3.ip_pkts));
		break;
	case 17:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].output_stats.l3.ip_bytes));
		break;
	case 18:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].output_stats.l4.tcp_pkts));
		break;
	case 19:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].output_stats.l4.tcp_bytes));
		break;
	case 20:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].output_stats.l4.udp_pkts));
		break;
	case 21:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].output_stats.l4.udp_bytes));
		break;
	case 30:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].output_stats.l4.tcp_retransmit_pkts));
		break;
	case 31:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].output_stats.l4.tcp_retransmit_bytes));
		break;
	case 39:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].delay_stats.tcp_delay_flows));
		break;
	case 40:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].map_stats[region][isp].delay_stats.tcp_delay_ms));
		break;

	}
}
static void udpi_mapstats_get(::ucloud::unetanalysis::StatsData *msg, uint8_t item, uint8_t region, uint8_t isp, time_t t)
{
	msg->set_uuid("all");
	msg->set_item_id(item);
	msg->set_region_id(region);
	msg->set_isp_id(isp);
	msg->set_is_regional(1);
	msg->set_value(0);
	msg->set_time(t);

	char key[2];
	memset(key, 0, sizeof(key));
	sprintf(key, "0");
	msg->set_no_regional_key(key, sizeof(key));

	switch (item) {
	case 0:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l3.ip_pkts));
		break;
	case 1:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l3.ip_bytes));
		break;
	case 2:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l4.tcp_pkts));
		break;
	case 3:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l4.tcp_bytes));
		break;
	case 4:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l4.udp_pkts));
		break;
	case 5:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l4.udp_bytes));
		break;
	case 6:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l7.ssh));
		break;
	case 7:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l7.ssh_bytes));
		break;
	case 8:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l7.http));
		break;
	case 9:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l7.http_bytes));
		break;
	case 10:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l7.https));
		break;
	case 11:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l7.https_bytes));
		break;
	case 12:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l7.others));
		break;
	case 13:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l7.others_bytes));
		break;
	case 14:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l4.tcp_retransmit_pkts));
		break;
	case 15:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l4.tcp_retransmit_bytes));
		break;
	case 16:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l3.ip_pkts));
		break;
	case 17:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l3.ip_bytes));
		break;
	case 18:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l4.tcp_pkts));
		break;
	case 19:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l4.tcp_bytes));
		break;
	case 20:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l4.udp_pkts));
		break;
	case 21:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l4.udp_bytes));
		break;
	case 22:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l7.ssh));
		break;
	case 23:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l7.ssh_bytes));
		break;
	case 24:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l7.http));
		break;
	case 25:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l7.http_bytes));
		break;
	case 26:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l7.https));
		break;
	case 27:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l7.https_bytes));
		break;
	case 28:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l7.others));
		break;
	case 29:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l7.others_bytes));
		break;
	case 30:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l4.tcp_retransmit_pkts));
		break;
	case 31:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l4.tcp_retransmit_bytes));
		break;
	case 32:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].flows_stats.flow_entries));
		break;
	case 33:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].flows_stats.closed_flow_entries));
		break;
	case 34:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].flows_stats.tcp_flow_entries));
		break;
	case 35:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].flows_stats.tcp_closed_flow_entries));
		break;
	case 36:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].flows_stats.tcp_rst_flow_entries));
		break;
	case 37:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].flows_stats.udp_flow_entries));
		break;
	case 38:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].flows_stats.udp_closed_flow_entries));
		break;
	case 39:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].delay_stats.tcp_delay_flows));
		break;
	case 40:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].delay_stats.tcp_delay_ms));
		break;
	case 41:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].input_stats.l4.tcp_syn_pkts));
		break;
	case 42:
		msg->set_value(rte_atomic64_read(&udpi.stats.map_stats[region][isp].output_stats.l4.tcp_syn_pkts));
		break;
	}
}

static void udpi_push_map_stats(time_t t, uint8_t region, uint8_t isp, uint32_t dumper_id)
{
	uint8_t k;

	proto_req[dumper_id].clear_body();
	::ucloud::unetanalysis::RecordStatsRequest* uir = proto_req[dumper_id].mutable_body()->MutableExtension(::ucloud::unetanalysis::record_stats_request);
	uir->set_dpdk_id(udpi.dpdk_id);

	for (k=0; k<UDPI_MAP_STATS_ITEMS; k++)
	{
		::ucloud::unetanalysis::StatsData* msg = uir->add_stats_data_list();
		udpi_mapstats_get(msg, k, region, isp, t);
	}

	uint32_t length = ByteSize(&proto_req[dumper_id]) + sizeof(uint32_t);
	int32_t size;
         
	proto_buf[dumper_id] = new char[length];

	size = EncodeMessage(&proto_req[dumper_id], &proto_buf[dumper_id], length);
	if (size <= 0) 
	{
		printf("Message encode fail\n");
		return;
 	}

	if (udpi.dumper_fds[dumper_id] > 0)
		udpi_socket_dumper_write(dumper_id, (uint8_t *)proto_buf[dumper_id], length);

	delete proto_buf[dumper_id];
}
static void udpi_push_line_map_stats(time_t t, uint8_t region, uint8_t isp, uint32_t dumper_id,uint8_t line_num)
{
	uint8_t k;

	proto_req[dumper_id].clear_body();
	::ucloud::unetanalysis::RecordStatsRequest* uir = proto_req[dumper_id].mutable_body()->MutableExtension(::ucloud::unetanalysis::record_stats_request);
	uir->set_dpdk_id(udpi.dpdk_id);

	for (k=0; k<UDPI_MAP_STATS_ITEMS; k++)
	{
		::ucloud::unetanalysis::StatsData* msg = uir->add_stats_data_list();
		udpi_line_mapstats_get(msg, k, region, isp, t ,line_num);
	}

	uint32_t length = ByteSize(&proto_req[dumper_id]) + sizeof(uint32_t);
	int32_t size;
         
	proto_buf[dumper_id] = new char[length];

	size = EncodeMessage(&proto_req[dumper_id], &proto_buf[dumper_id], length);
	if (size <= 0) 
	{
		printf("Message encode fail\n");
		return;
 	}

	if (udpi.dumper_fds[dumper_id] > 0)
		udpi_socket_dumper_write(dumper_id, (uint8_t *)proto_buf[dumper_id], length);

	delete proto_buf[dumper_id];
}

static void udpi_whole_line_stats_get(::ucloud::unetanalysis::StatsData *msg, uint8_t item, time_t t,uint8_t line_num)
{
	char uuid[128];
	memset(uuid, 0, 128);
	sprintf(uuid, "line_%u", line_num);
	msg->set_uuid(uuid);
	msg->set_item_id(item);
	msg->set_region_id(0);
	msg->set_isp_id(0);
	msg->set_is_regional(1);
	msg->set_value(0);
	msg->set_time(t);

	char key[2];
	memset(key, 0, sizeof(key));
	sprintf(key, "0");
	msg->set_no_regional_key(key, sizeof(key));

	switch (item) {
	case 0:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].input_stats.l3.ip_pkts));
		break;
	case 1:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].input_stats.l3.ip_bytes));
		break;
	case 2:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].input_stats.l4.tcp_pkts));
		break;
	case 3:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].input_stats.l4.tcp_bytes));
		break;
	case 4:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].input_stats.l4.udp_pkts));
		break;
	case 5:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].input_stats.l4.udp_bytes));
		break;
	case 14:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].input_stats.l4.tcp_retransmit_pkts));
		break;
	case 15:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].input_stats.l4.tcp_retransmit_bytes));
		break;
	case 16:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].output_stats.l3.ip_pkts));
		break;
	case 17:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].output_stats.l3.ip_bytes));
		break;
	case 18:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].output_stats.l4.tcp_pkts));
		break;
	case 19:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].output_stats.l4.tcp_bytes));
		break;
	case 20:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].output_stats.l4.udp_pkts));
		break;
	case 21:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].output_stats.l4.udp_bytes));
		break;
	case 30:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].output_stats.l4.tcp_retransmit_pkts));
		break;
	case 31:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].output_stats.l4.tcp_retransmit_bytes));
		break;
	case 39:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].delay_stats.tcp_delay_flows));
		break;
	case 40:
		msg->set_value(rte_atomic64_read(&udpi.stats.line_stats[line_num].delay_stats.tcp_delay_ms));
		break;

	}
}
static void udpi_wholestats_get(::ucloud::unetanalysis::StatsData *msg, uint8_t item, time_t t)
{
	msg->set_uuid("all");
	msg->set_item_id(item);
	msg->set_region_id(0);
	msg->set_isp_id(0);
	msg->set_is_regional(1);
	msg->set_value(0);
	msg->set_time(t);

	char key[2];
	memset(key, 0, sizeof(key));
	sprintf(key, "0");
	msg->set_no_regional_key(key, sizeof(key));

	switch (item) {
	case 0:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l3.ip_pkts));
		break;
	case 1:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l3.ip_bytes));
		break;
	case 2:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l4.tcp_pkts));
		break;
	case 3:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l4.tcp_bytes));
		break;
	case 4:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l4.udp_pkts));
		break;
	case 5:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l4.udp_bytes));
		break;
	case 6:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l7.ssh));
		break;
	case 7:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l7.ssh_bytes));
		break;
	case 8:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l7.http));
		break;
	case 9:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l7.http_bytes));
		break;
	case 10:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l7.https));
		break;
	case 11:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l7.https_bytes));
		break;
	case 12:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l7.others));
		break;
	case 13:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l7.others_bytes));
		break;
	case 14:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l4.tcp_retransmit_pkts));
		break;
	case 15:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l4.tcp_retransmit_bytes));
		break;
	case 16:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l3.ip_pkts));
		break;
	case 17:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l3.ip_bytes));
		break;
	case 18:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l4.tcp_pkts));
		break;
	case 19:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l4.tcp_bytes));
		break;
	case 20:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l4.udp_pkts));
		break;
	case 21:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l4.udp_bytes));
		break;
	case 22:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l7.ssh));
		break;
	case 23:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l7.ssh_bytes));
		break;
	case 24:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l7.http));
		break;
	case 25:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l7.http_bytes));
		break;
	case 26:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l7.https));
		break;
	case 27:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l7.https_bytes));
		break;
	case 28:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l7.others));
		break;
	case 29:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l7.others_bytes));
		break;
	case 30:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l4.tcp_retransmit_pkts));
		break;
	case 31:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l4.tcp_retransmit_bytes));
		break;
	case 32:
		msg->set_value(rte_atomic64_read(&udpi.stats.flows_stats.flow_entries));
		break;
	case 33:
		msg->set_value(rte_atomic64_read(&udpi.stats.flows_stats.closed_flow_entries));
		break;
	case 34:
		msg->set_value(rte_atomic64_read(&udpi.stats.flows_stats.tcp_flow_entries));
		break;
	case 35:
		msg->set_value(rte_atomic64_read(&udpi.stats.flows_stats.tcp_closed_flow_entries));
		break;
	case 36:
		msg->set_value(rte_atomic64_read(&udpi.stats.flows_stats.tcp_rst_flow_entries));
		break;
	case 37:
		msg->set_value(rte_atomic64_read(&udpi.stats.flows_stats.udp_flow_entries));
		break;
	case 38:
		msg->set_value(rte_atomic64_read(&udpi.stats.flows_stats.udp_closed_flow_entries));
		break;
	case 39:
		msg->set_value(rte_atomic64_read(&udpi.stats.delay_stats.tcp_delay_flows));
		break;
	case 40:
		msg->set_value(rte_atomic64_read(&udpi.stats.delay_stats.tcp_delay_ms));
		break;
	case 41:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_stats.l4.tcp_syn_pkts));
		break;
	case 42:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_stats.l4.tcp_syn_pkts));
		break;
	case 52:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_comp_stats[0].ip_bytes));
		break;
	case 53:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_comp_stats[0].ip_bytes));
		break;
	case 54:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_comp_stats[1].ip_bytes));
		break;
	case 55:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_comp_stats[1].ip_bytes));
		break;
	case 56:
		msg->set_value(rte_atomic64_read(&udpi.stats.input_comp_stats[2].ip_bytes));
		break;
	case 57:
		msg->set_value(rte_atomic64_read(&udpi.stats.output_comp_stats[2].ip_bytes));
		break;
	}
}


static void udpi_push_whole_stats(time_t t, uint32_t dumper_id)
{
	uint8_t k;

	proto_req[dumper_id].clear_body();
	::ucloud::unetanalysis::RecordStatsRequest* uir = proto_req[dumper_id].mutable_body()->MutableExtension(::ucloud::unetanalysis::record_stats_request);
	uir->set_dpdk_id(udpi.dpdk_id);

	for (k=0; k<UDPI_STATS_ITEMS; k++)
	{
		::ucloud::unetanalysis::StatsData* msg = uir->add_stats_data_list();
		udpi_wholestats_get(msg, k, t);
	}

	for (k=52; k<58; k++)
	{
		::ucloud::unetanalysis::StatsData* msg = uir->add_stats_data_list();
		udpi_wholestats_get(msg, k, t);
	}

	uint32_t length = ByteSize(&proto_req[dumper_id]) + sizeof(uint32_t);
	int32_t size;
	proto_buf[dumper_id] = new char[length];
         
	size = EncodeMessage(&proto_req[dumper_id], &proto_buf[dumper_id], length);
	if (size <= 0) 
	{
		printf("Message encode fail\n");
		return;
	}

	if (udpi.dumper_fds[dumper_id] > 0)
		udpi_socket_dumper_write(dumper_id, (uint8_t*)proto_buf[dumper_id], length);

	delete proto_buf[dumper_id];
}
static void udpi_push_whole_line_stats(time_t t, uint32_t dumper_id,uint8_t line_num)
{
	uint8_t k;

	proto_req[dumper_id].clear_body();
	::ucloud::unetanalysis::RecordStatsRequest* uir = proto_req[dumper_id].mutable_body()->MutableExtension(::ucloud::unetanalysis::record_stats_request);
	uir->set_dpdk_id(udpi.dpdk_id);

	for (k=0; k<UDPI_STATS_ITEMS; k++)
	{
		::ucloud::unetanalysis::StatsData* msg = uir->add_stats_data_list();
		udpi_whole_line_stats_get(msg, k, t,line_num);
	}
	/*
	for (k=52; k<58; k++)
	{
		::ucloud::unetanalysis::StatsData* msg = uir->add_stats_data_list();
		udpi_whole_line_stats_get(msg, k, t,line_num);
	}
	*/
	uint32_t length = ByteSize(&proto_req[dumper_id]) + sizeof(uint32_t);
	int32_t size;
	proto_buf[dumper_id] = new char[length];
         
	size = EncodeMessage(&proto_req[dumper_id], &proto_buf[dumper_id], length);
	if (size <= 0) 
	{
		printf("Message encode fail\n");
		return;
	}

	if (udpi.dumper_fds[dumper_id] > 0)
		udpi_socket_dumper_write(dumper_id, (uint8_t*)proto_buf[dumper_id], length);

	delete proto_buf[dumper_id];
}

static void udpi_push_stats(time_t t, uint32_t dumper_id)
{
	uint8_t i, j ,k;

	udpi_push_whole_stats(t, dumper_id);
	for (i=1; i<UDPI_REGION_STATS; i++)
	{
		for (j=1; j<UDPI_OPERATOR_STATS; j++)
		{
			udpi_push_map_stats(t, i, j, dumper_id);
		}
	}
		
	for (k=0; k<UDPI_LINE_STATS; k++)
	{
		udpi_push_whole_line_stats(t,dumper_id,k);
		for (i=1; i<UDPI_REGION_STATS; i++)
		{
		for(j=1; j<UDPI_OPERATOR_STATS; j++)
		{
			udpi_push_line_map_stats(t, i, j, dumper_id,k);
		}
		}
	}	
}

static void udpi_init_protobuf(int i)
{
	NewMessage(&proto_req[i], 1, "f843c120-93e9-11e5-b18b-ac220b037c17", ::ucloud::unetanalysis::RECORD_STATS_REQUEST, 0, false, 0, 0, NULL, NULL, NULL);
}

static ::ucloud::UMessage proto_req_iplist;
static void udpi_startup_msg(uint32_t dumper_id)
{
	NewMessage(&proto_req_iplist, 1, "f843c120-93e9-11e5-b18b-ac220b037c17", ::ucloud::unetanalysis::PUSH_IPS_FILE_REQUEST, 0, false, 0, 0, NULL, NULL, NULL);
	::ucloud::unetanalysis::PushIpsFileRequest* uir = proto_req_iplist.mutable_body()->MutableExtension(::ucloud::unetanalysis::push_ips_file_request);
	uir->set_name(" ");

	uint32_t length = ByteSize(&proto_req_iplist) + sizeof(uint32_t);
	int32_t size;

	proto_buf[dumper_id] = new char[length];
         
	size = EncodeMessage(&proto_req_iplist, &proto_buf[dumper_id], length);
	if (size <= 0) 
	{
		printf("Message encode fail\n");
		return;
 	}

	if (udpi.dumper_fds[dumper_id] > 0)
		udpi_socket_dumper_write(dumper_id, (uint8_t *)proto_buf[dumper_id], length);

	delete proto_buf[dumper_id];
}

void udpi_main_loop_stats(void)
{
	uint32_t core_id = rte_lcore_id();
	
	struct udpi_core_params *core_params 
		= udpi_get_core_params(core_id);

	if((!core_params) 
		|| (core_params->core_type != UDPI_CORE_STATS))
	{
		rte_panic("Core %u misconfiguration\n", core_id);
		return;
	}

	uint32_t dumper_id = core_params->id;

	RTE_LOG(INFO, USER1, "Core %u is doing STATS with dumper id %u\n", core_id, dumper_id);

	uint64_t lcore_tsc_hz, old_cycles, new_cycles;
	lcore_tsc_hz = rte_get_timer_hz();
	old_cycles = rte_get_timer_cycles();
	time_t t;

	udpi_socket_dumper_create(dumper_id);
	
	udpi_init_protobuf(dumper_id);

	if (dumper_id == 0)
		udpi_startup_msg(dumper_id);
	
	while (1)
	{
		if (udpi.dumper_fds[dumper_id] > 0)
			udpi_socket_dumper_read(dumper_id);
		else 
			udpi_socket_dumper_create(dumper_id);

		if (dumper_id == (udpi.n_dumpers - 1))
		{
			new_cycles = rte_get_timer_cycles();    
			if (new_cycles - old_cycles > lcore_tsc_hz*10)
			{
				time(&t);
				rte_mb();
				uint32_t i;
				for (i=0; i<udpi.n_dumpers-1; i++)
				{
					void *msg;
					struct udpi_msg_req *req;			

					/* Allocate message buffer */
					msg = (void *)rte_ctrlmbuf_alloc(udpi.msg_pool);
					if (msg == NULL)
						rte_panic("Unable to allocate new message\n");

					/* Fill request message */
					req = (struct udpi_msg_req *)rte_ctrlmbuf_data((struct rte_mbuf *)msg);
					memset(req, 0, sizeof(struct udpi_msg_req));

					req->type = UDPI_MSG_ADD_STATS;
					req->add_stats.t = t;
					
					int ret = rte_ring_sp_enqueue(udpi.stats_rings[i], msg);
					if (ret != 0)
					{
						printf("enqueue stats ring %u fail %d\n", i, ret);
						printf("\n\tdumper_ring[%u] count: %u, free count: %u,  full %u, empty %u\n", i, rte_ring_count(udpi.stats_rings[i]),  rte_ring_free_count(udpi.stats_rings[i]), rte_ring_full(udpi.stats_rings[i]), rte_ring_empty(udpi.stats_rings[i]));
					}
				}

				uint64_t t_old, t_new;
				t_old = rte_get_timer_cycles();
				udpi_push_stats(t, dumper_id);
				udpi_push_all_eip(t, dumper_id);
				t_new = rte_get_timer_cycles();
				if (unlikely(udpi.debug))
					printf("time:%lu for dumper id %u when ts %lu\n", (t_new - t_old)/lcore_tsc_hz, dumper_id, t);

				old_cycles = new_cycles;
			}
		}
		else
		{
			void *msg;
			int ret;
			struct udpi_msg_req *req;

			/* Read request message */
			ret = rte_ring_sc_dequeue(udpi.stats_rings[dumper_id], &msg);
			if (ret != 0)
				continue;

			req = (struct udpi_msg_req *)rte_ctrlmbuf_data((struct rte_mbuf *)msg);
			switch (req->type) {
				case UDPI_MSG_ADD_STATS:
				{
					time_t t = req->add_stats.t;
					uint64_t t_old, t_new;
					t_old = rte_get_timer_cycles();
					udpi_push_all_eip(t, dumper_id);
					t_new = rte_get_timer_cycles();
					if (unlikely(udpi.debug))
						printf("time:%lu for dumper id %u when ts %lu\n", (t_new - t_old)/lcore_tsc_hz, dumper_id, t);
					break;
				}
				default:
				{
					break;
				}
			}

			/* Free message buffer */
			rte_ctrlmbuf_free((struct rte_mbuf *)msg);
		}

	}

	return;
}


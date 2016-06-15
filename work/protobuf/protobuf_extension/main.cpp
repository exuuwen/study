#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <net/ethernet.h>
#include <poll.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "ucloud.pb.h"
#include "unetanalysis.177000.178000.pb.h"

/**
 *  * 产生一个新消息
 *   * @param pMessage 输入输出参数
 *    * @return 正常返回*pMessage指针；NULL表示错误
 *     **/
void *NewMessage(void *pMessage, 
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

/*
    static bool bSRand = false;
    if ( !bSRand ) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        srandom(tv.tv_usec);
        bSRand = true;
    }    
*/
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

int EncodeMessage(const void *pMessage, char **pData, unsigned iSize) 
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

uint32_t ByteSize(const void *pMessage)
{
        const ucloud::UMessage *pMsg = (const ucloud::UMessage *)pMessage;
       
        return pMsg->ByteSize();
}




int main()
{

    ::ucloud::UMessage req;
    NewMessage(&req, 1, "f843c120-93e9-11e5-b18b-ac220b037c17", ::ucloud::unetanalysis::RECORD_STATS_REQUEST, 0, false, 0, 0, NULL, NULL, NULL);
    ::ucloud::unetanalysis::RecordStatsRequest* uir = req.mutable_body()->MutableExtension(::ucloud::unetanalysis::record_stats_request);

	for (int i = 0; i < 10; i++) 
	{ 
        ::ucloud::unetanalysis::StatsData* sdto = uir->add_stats_data_list();
    
        sdto->set_uuid("all");
        sdto->set_item_id(i);
        sdto->set_region_id(100);
        sdto->set_isp_id(100);
        sdto->set_value(i+1000);
        sdto->set_time(0x12345678);
    } 

	uint32_t length = ByteSize(&req) + sizeof(uint32_t);
   	char* buffer = new char[length];                                                                                                       
    int32_t size;
         
     size = EncodeMessage(&req, &buffer, length);
    if (size <= 0) {
               delete [] buffer;
               printf("Message send to manager server encode failed.");
                return -1;
       }

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {
        printf("StreamSocket Create socket failed\n");
        return -1;
    }

	struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("172.19.1.96");
    server_addr.sin_port = htons(6505);


    struct sockaddr_in addr;
    socklen_t addr_len;

    addr_len = sizeof(addr);
    int ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1)
    {
        perror("connect server fail:");
        ::close(sock);
        return -1;
    }

    printf("connect success\n");


	ret = ::write(sock, buffer, length);
	   if (ret < 0)
        {
            printf("write() failed. len = %d, ret = %d, errno = %d:%s\n", length, ret, errno, strerror(errno));
            return -1;
        }


	delete [] buffer;


	sleep(10);

	close(sock);	



	
}

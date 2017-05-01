// =====================================================================================
// 
//       Filename:  vip_proxy_api.cpp
// 
//    Description:  粉钻信息API
// 
//        Version:  1.0
//        Created:  2010年09月08日 10时36分17秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <arpa/inet.h>

#include "vip_proxy_api.h"
#include "vip_proxy_proto.h"
#include "comm/ini_file/ini_file.h"
#include "comm/safe_tcp_client/safe_tcp_client.h"

using namespace snslib;

CVipProxyApi::CVipProxyApi() : m_safe_tcp_client(NULL){
	memset(m_err_msg, 0x0, sizeof(m_err_msg));
	m_safe_tcp_client = new CSafeTcpClient;
}

CVipProxyApi::~CVipProxyApi(){
	if(m_safe_tcp_client){
		delete m_safe_tcp_client;
		m_safe_tcp_client = NULL;
	}
}

const char* CVipProxyApi::GetErrMsg(){
	m_err_msg[sizeof(m_err_msg)-1] = '\0';
	return m_err_msg;
}

int CVipProxyApi::Init(const char *conf) {
	// 初始化TCP客户端
	int rv = m_safe_tcp_client->Init(conf);
	if(rv != 0){
		snprintf(m_err_msg, sizeof(m_err_msg), "invalid config file %s", conf);
		return -1;
	}

	return 0;
}

int CVipProxyApi::GetVipData(unsigned int uin, VipData &vip_data) {
	CmdGetVipDataReq request;
	request.dwUin = htonl(uin);

	char buffer[10 * sizeof(CmdGetVipDataRsp)];
	memset(buffer, 0x0, sizeof(buffer));
	unsigned int buffer_size = sizeof(buffer);

	int rv = m_safe_tcp_client->SendAndRecv(&request, sizeof(request), buffer, &buffer_size);
	if(rv != 0){
		snprintf(m_err_msg, sizeof(m_err_msg), "SendAndRecv failed, rv=%d, msg=%s",
				rv, m_safe_tcp_client->GetErrMsg());
		return -1;
	}

	if(buffer_size < sizeof(CmdGetVipDataRsp)){
		snprintf(m_err_msg, sizeof(m_err_msg), "invalid response length=%u, expected=%u",
				buffer_size, sizeof(CmdGetVipDataRsp));
		return -2;
	}

	const CmdGetVipDataRsp * response = reinterpret_cast<const CmdGetVipDataRsp *>(buffer);

	int result = ntohl(response->dwResult);
	if(result != 0){
		snprintf(m_err_msg, sizeof(m_err_msg), "Internal error, response return %d", result);
		return result;
	}
	
	vip_data.growth = ntohl(response->growth);
	vip_data.days_left_normal = ntohs(response->days_left_normal);
	vip_data.days_left_medium = ntohs(response->days_left_medium);
	vip_data.days_left_fast = ntohs(response->days_left_fast);
	vip_data.days_left_rapid = ntohs(response->days_left_rapid);
	vip_data.vip_flag = response->vip_flag;
	vip_data.year_flag = response->year_flag;
	vip_data.level = ntohs(response->level);
	vip_data.growth_rate = ntohs(response->growth_rate);

	return 0;
}


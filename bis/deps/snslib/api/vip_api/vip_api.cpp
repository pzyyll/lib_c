// =====================================================================================
// 
//       Filename:  vip_api.cpp
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

#include "vip_api.h"
#include "vip_def.h"
#include "vip_ttc.h"
#include "comm/ini_file/ini_file.h"
#include <ttcapi.h>

using namespace snslib;
using namespace snslib::vip;

CVipApi::CVipApi() : m_vip_ttc(NULL){
	memset(m_err_msg, 0x0, sizeof(m_err_msg));
	m_vip_ttc = new CVipTTC;
}

CVipApi::~CVipApi(){
	if(m_vip_ttc){
		delete m_vip_ttc;
		m_vip_ttc = NULL;
	}
}

const char* CVipApi::GetErrMsg(){
	m_err_msg[sizeof(m_err_msg)-1] = '\0';
	return m_err_msg;
}

int CVipApi::Init(const char *conf) {
	// 打开配置文件
	CIniFile objIni(conf);
	if(!objIni.IsValid()){
		snprintf(m_err_msg, sizeof(m_err_msg) - 1, "invalid config file %s", conf);
		return E_BAD_CONFIG_FILE;
	}

	// TTC配置文件
	char ttc_conf[256];
	memset(ttc_conf, 0x0, sizeof(ttc_conf));

	// TTC配置方式
	int ttc_conf_type = 0;
	objIni.GetInt("TTC", "TTC_CONF_TYPE", 0, &ttc_conf_type);

	if(ttc_conf_type == 1){
		objIni.GetString("TTC", "TTC_CONF", "", ttc_conf, sizeof(ttc_conf));
	}
	else if(ttc_conf_type == 2){
		strncpy(ttc_conf, conf, sizeof(ttc_conf));
	}
	else{
		snprintf(m_err_msg, sizeof(m_err_msg) - 1, "bad config TTC::TTC_CONF_TYPE %d", ttc_conf_type);
		return E_BAD_CONFIG_FILE;
	}

	int rv = m_vip_ttc->Initialize(ttc_conf);
	if(rv != 0){
		strncpy(m_err_msg, m_vip_ttc->GetErrMsg(), sizeof(m_err_msg));
		return E_TTC_INIT;
	}

	return E_SUCCESS;
}

int CVipApi::GetVipData(unsigned int uin, VipData &vip_data) {
	if(!m_vip_ttc){
		snprintf(m_err_msg, sizeof(m_err_msg), "uninitialized vip_ttc");
		return E_TTC_INVALID;
	}

	memset(&vip_data, 0x0, sizeof(vip_data));

	CVipTTC::VipData ttc_data;
	memset(&ttc_data, 0x0, sizeof(ttc_data));

	int rv = m_vip_ttc->GetVipData(uin, ttc_data);
	if(rv == E_TTC_NO_REC){
		vip_data.vip_flag = 0;
		return E_SUCCESS;
	}
	else if(rv != E_SUCCESS){
		snprintf(m_err_msg, sizeof(m_err_msg) - 1, m_vip_ttc->GetErrMsg());
		return rv;
	}

	vip_data.growth = ttc_data.growth;
	vip_data.days_left_normal = ttc_data.days_left_normal;
	vip_data.days_left_medium = ttc_data.days_left_medium;
	vip_data.days_left_fast = ttc_data.days_left_fast;
	vip_data.days_left_rapid = ttc_data.days_left_rapid;
	// 计算其他数据
	vip_data.vip_flag = CVipTTC::GetVipFlag(ttc_data);
	vip_data.year_flag = CVipTTC::GetYearFlag(ttc_data);
	vip_data.level = CVipTTC::GetLevel(ttc_data);
	vip_data.growth_rate = CVipTTC::GetGrowthRate(ttc_data);

	return E_SUCCESS;
}


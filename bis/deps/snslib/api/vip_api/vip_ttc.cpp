// =====================================================================================
// 
//       Filename:  vip_ttc.cpp
// 
//    Description:  VIP TTC
// 
//        Version:  1.0
//        Created:  2010年09月15日 10时15分54秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#include "vip_ttc.h"
#include "vip_def.h"
#include "comm/ini_file/ini_file.h"
#include "comm/log/pet_log.h"
#include <ttcapi.h>
#include <string>

using namespace snslib;
using namespace snslib::vip;

// 构造函数
CVipTTC::CVipTTC(){
	memset(m_err_msg, 0x0, sizeof(m_err_msg));
}

// 析构函数
CVipTTC::~CVipTTC(){
	Finalize();
}

// 读取配置文件，初始化TTC实例
int CVipTTC::Initialize(const char *ttc_conf){
	if(!m_ttc_conf.empty() || !m_ttc_server.empty()){
		Finalize();
	}

	// 读取TTC配置
	CIniFile objTTCIni(ttc_conf);
	if(!objTTCIni.IsValid()){
		snprintf(m_err_msg, sizeof(m_err_msg), "invalid ttc config file %s", ttc_conf);
		return E_BAD_CONFIG_FILE;
	}

	int dbcount = 0;
	objTTCIni.GetInt("DBRANGECOUNT", "dbcount", 0, &dbcount);
	if(dbcount == 0){
		snprintf(m_err_msg, sizeof(m_err_msg),
				"bad config DBRANGECOUNT::dbcount %d|%s", dbcount, ttc_conf);
		return E_BAD_CONFIG_FILE;
	}

	m_ttc_conf.resize(dbcount);

	for(int i = 0; i < dbcount; ++i){
		char section[128];
		snprintf(section, sizeof(section), "db%d", i);

		objTTCIni.GetInt(section, "uin_range_start", 0, &m_ttc_conf[i].uin_range_start);
		objTTCIni.GetInt(section, "uin_range_end", 0, &m_ttc_conf[i].uin_range_end);
		objTTCIni.GetString(section, "ttc_addr_vip", "",
				m_ttc_conf[i].addr, sizeof(m_ttc_conf[i].addr) - 1);
		objTTCIni.GetString(section, "ttc_table_name_vip_info", "",
				m_ttc_conf[i].table_name, sizeof(m_ttc_conf[i].table_name) - 1);
	}

	// 初始化TTC::Server
	m_ttc_server.resize(dbcount);
	for(size_t i = 0; i < m_ttc_server.size(); ++i){
		m_ttc_server[i] = NULL;
	}

	for(int i = 0; i < dbcount; ++i){
		m_ttc_server[i] = new TTC::Server;

		m_ttc_server[i]->SetTimeout(3);

		int rv = m_ttc_server[i]->IntKey();
		if(rv != 0){
			snprintf(m_err_msg, sizeof(m_err_msg),
					"TTC::Server::IntKey failed, rv=%d, msg=%s",
					rv, m_ttc_server[i]->ErrorMessage());
			Finalize();
			return E_TTC_INIT;
		}

		rv = m_ttc_server[i]->SetAddress(m_ttc_conf[i].addr);
		if(rv != 0){
			snprintf(m_err_msg, sizeof(m_err_msg),
					"TTC::Server::SetAddress failed, rv=%d, msg=%s",
					rv, m_ttc_server[i]->ErrorMessage());
			Finalize();
			return E_TTC_INIT;
		}

		rv = m_ttc_server[i]->SetTableName(m_ttc_conf[i].table_name);
		if(rv != 0){
			snprintf(m_err_msg, sizeof(m_err_msg),
					"TTC::Server::SetTableName failed, rv=%d, msg=%s",
					rv, m_ttc_server[i]->ErrorMessage());
			Finalize();
			return E_TTC_INIT;
		}
	}

	return E_SUCCESS;
}

// 释放TTC实例
void CVipTTC::Finalize(){
	for(size_t i = 0; i < m_ttc_server.size(); ++i){
		if(m_ttc_server[i]){
			delete m_ttc_server[i];
			m_ttc_server[i] = NULL;
		}
	}
	m_ttc_server.clear();
	m_ttc_conf.clear();
}

// 获取TTC实例
TTC::Server* CVipTTC::GetTTCServer(unsigned int uin){
	for(size_t i = 0; i < m_ttc_conf.size(); ++i){
		int mod = uin % 100;
		// 注意左闭右开
		if(mod >= m_ttc_conf[i].uin_range_start &&
				mod < m_ttc_conf[i].uin_range_end){
			return m_ttc_server[i];
		}
	}
	return NULL;
}

// 如果没有该用户数据，则创建一条该用户数据
int CVipTTC::TouchVipData(unsigned int uin){
	TTC::Server *ttc_server = GetTTCServer(uin);
	if(!ttc_server){
		snprintf(m_err_msg, sizeof(m_err_msg), "invalid ttc server, uin=%u", uin);
		return E_TTC_INVALID;
	}

	TTC::InsertRequest stInsertReq(ttc_server);
	int rv = stInsertReq.SetKey(uin);
	if(rv != 0){
		snprintf(m_err_msg, sizeof(m_err_msg),
				"TTC::InsertRequest::SetKey failed, uin=%u, rv=%d, msg=%s",
				uin, rv, ttc_server->ErrorMessage());
		return E_TTC_TOUCH;
	}

	TTC::Result stResult; 
	stInsertReq.Execute(stResult);

	return E_SUCCESS;
}

int CVipTTC::GetVipData(unsigned int uin, VipData &vip_data){
	memset(&vip_data, 0x0, sizeof(vip_data));

	// 拉取DB数据
	TTC::Server *ttc_server = GetTTCServer(uin);
	if(!ttc_server){
		snprintf(m_err_msg, sizeof(m_err_msg), "invalid ttc server, uin=%u", uin);
		return E_TTC_INVALID;
	}

	TTC::GetRequest stGetReq(ttc_server);

	int rv = stGetReq.SetKey(uin);
	if(rv != 0){
		snprintf(m_err_msg, sizeof(m_err_msg),
				"TTC::GetRequest::SetKey failed, uin=%u, rv=%d, msg=%s",
				uin, rv, ttc_server->ErrorMessage());
		return E_TTC_GET;
	}

	rv = 0;
	if(0 == rv) rv = stGetReq.Need("GROWTH");
	if(0 == rv)	rv = stGetReq.Need("DAYS_LEFT_NORMAL");
	if(0 == rv)	rv = stGetReq.Need("DAYS_LEFT_MEDIUM");
	if(0 == rv)	rv = stGetReq.Need("DAYS_LEFT_FAST");
	if(0 == rv)	rv = stGetReq.Need("DAYS_LEFT_RAPID");
	if(0 == rv)	rv = stGetReq.Need("PROC_TIMESTAMP");
	if(0 == rv)	rv = stGetReq.Need("CHECK_AND_SET");
	if(0 == rv) rv = stGetReq.Need("FLAG1");

	if(0 != rv){
		snprintf(m_err_msg, sizeof(m_err_msg),
				"TTC::GetRequest::Need failed, rv=%d, msg=%s",
				rv, ttc_server->ErrorMessage());
		return E_TTC_GET;
	}

	TTC::Result stResult; 
	rv = stGetReq.Execute(stResult);
	if(rv != 0){
		snprintf(m_err_msg, sizeof(m_err_msg),
				"TTC::GetRequest::Execute failed, rv=%d, error_from=%s, msg=%s",
				rv, stResult.ErrorFrom(), stResult.ErrorMessage());
		return E_TTC_GET;
	}

	if(stResult.NumRows() <= 0){
		snprintf(m_err_msg, sizeof(m_err_msg),
				"TTC::Result::NumRows == 0");
		return E_TTC_NO_REC;
	}

	rv = stResult.FetchRow();
	if(rv != 0){
		snprintf(m_err_msg, sizeof(m_err_msg),
				"TTC::Result::FetchRow failed, rv=%d", rv);
		return E_TTC_GET;
	}

	vip_data.growth = stResult.IntValue("GROWTH");
	vip_data.days_left_normal = stResult.IntValue("DAYS_LEFT_NORMAL");
	vip_data.days_left_medium = stResult.IntValue("DAYS_LEFT_MEDIUM");
	vip_data.days_left_fast = stResult.IntValue("DAYS_LEFT_FAST");
	vip_data.days_left_rapid = stResult.IntValue("DAYS_LEFT_RAPID");
	vip_data.proc_timestamp = stResult.IntValue("PROC_TIMESTAMP");
	vip_data.check_and_set = stResult.IntValue("CHECK_AND_SET");
	vip_data.non_prepaid_flag = stResult.IntValue("FLAG1");

	return E_SUCCESS;
}

int CVipTTC::SetVipData(unsigned int uin, const VipData &vip_data){
	// 先TOUCH一下
	int rv = TouchVipData(uin);
	if(rv != 0){
		return rv;
	}

	TTC::Server *ttc_server = GetTTCServer(uin);
	if(!ttc_server){
		snprintf(m_err_msg, sizeof(m_err_msg), "invalid ttc server, uin=%u", uin);
		return E_TTC_INVALID;
	}

	TTC::UpdateRequest stUpdateReq(ttc_server);

	rv = stUpdateReq.SetKey(uin);
	if(rv != 0){
		snprintf(m_err_msg, sizeof(m_err_msg),
				"TTC::GetRequest::SetKey failed, uin=%u, rv=%d, msg=%s",
				uin, rv, ttc_server->ErrorMessage());
		return E_TTC_GET;
	}

	int rv1 = 0, rv2 = 0;
#define JUDGE_RV (0 == rv1 && 0 == rv2)

	if(JUDGE_RV && (vip_data.mask & ADD_GROWTH)){
		rv1 = stUpdateReq.Add("GROWTH", vip_data.growth);
		if(vip_data.growth < 0){
			rv2 = stUpdateReq.GE("GROWTH", -vip_data.growth);
		}
	}
	if(JUDGE_RV && (vip_data.mask & ADD_DAYS_LEFT_NORMAL)){
		rv1 = stUpdateReq.Add("DAYS_LEFT_NORMAL", vip_data.days_left_normal);
		if(vip_data.days_left_normal < 0){
			rv2 = stUpdateReq.GE("DAYS_LEFT_NORMAL", -vip_data.days_left_normal);
		}
	}
	if(JUDGE_RV && (vip_data.mask & ADD_DAYS_LEFT_MEDIUM)){
		rv1 = stUpdateReq.Add("DAYS_LEFT_MEDIUM", vip_data.days_left_medium);
		if(vip_data.days_left_medium < 0){
			rv2 = stUpdateReq.GE("DAYS_LEFT_MEDIUM", -vip_data.days_left_medium);
		}
	}
	if(JUDGE_RV && (vip_data.mask & ADD_DAYS_LEFT_FAST)){
		rv1 = stUpdateReq.Add("DAYS_LEFT_FAST", vip_data.days_left_fast);
		if(vip_data.days_left_fast < 0){
			rv2 = stUpdateReq.GE("DAYS_LEFT_FAST", -vip_data.days_left_fast);
		}
	}
	if(JUDGE_RV && (vip_data.mask & ADD_DAYS_LEFT_RAPID)){
		rv1 = stUpdateReq.Add("DAYS_LEFT_RAPID", vip_data.days_left_rapid);
		if(vip_data.days_left_rapid < 0){
			rv2 = stUpdateReq.GE("DAYS_LEFT_RAPID", -vip_data.days_left_rapid);
		}
	}
	if(JUDGE_RV && (vip_data.mask & SET_PROC_TIMESTAMP)){
		rv1 = stUpdateReq.Set("PROC_TIMESTAMP", vip_data.proc_timestamp);
	}
	if(JUDGE_RV && (vip_data.mask & CHECK_AND_SET)){
		rv1 = stUpdateReq.NE("CHECK_AND_SET", vip_data.check_and_set);
		rv2 = stUpdateReq.Set("CHECK_AND_SET", vip_data.check_and_set);
	}
	if(JUDGE_RV && (vip_data.mask & SET_NON_PREPAID_FLAG)){
		rv1 = stUpdateReq.Set("FLAG1", vip_data.non_prepaid_flag);
	}

	if(!JUDGE_RV){
		snprintf(m_err_msg, sizeof(m_err_msg),
				"TTC::GetRequest::Set/Add/NE/GE failed, rv1=%d, rv2=%d, msg=%s",
				rv1, rv2, ttc_server->ErrorMessage());
		return E_TTC_GET;
	}

	TTC::Result stResult; 
	rv = stUpdateReq.Execute(stResult);
	if(rv != 0){
		snprintf(m_err_msg, sizeof(m_err_msg),
				"TTC::GetRequest::Execute failed, rv=%d, error_from=%s, msg=%s",
				rv, stResult.ErrorFrom(), stResult.ErrorMessage());
		return E_TTC_GET;
	}

	if(stResult.AffectedRows() <= 0){
		snprintf(m_err_msg, sizeof(m_err_msg),
				"TTC::Result::AffectedRows == 0");
		return E_TTC_NO_AFFECTED;
	}

	PetLog(0, uin, PETLOG_INFO,
			"%x|%d|%d|%d|%d|%d|%u|%u|%hhd",
			vip_data.mask,
			vip_data.growth,
			vip_data.days_left_normal,
			vip_data.days_left_medium,
			vip_data.days_left_fast,
			vip_data.days_left_rapid,
			vip_data.proc_timestamp,
			vip_data.check_and_set,
			vip_data.non_prepaid_flag
			);

	return E_SUCCESS;
}

// 得到错误描述
const char* CVipTTC::GetErrMsg(){
	m_err_msg[sizeof(m_err_msg)-1] = '\0';
	return m_err_msg;
}

unsigned short CVipTTC::GetLevel(const VipData &vip_data){
	for(unsigned short level = sizeof(LevelEdge) / sizeof(LevelEdge[0]); level > 0; --level){
		if(static_cast<unsigned short>(vip_data.growth) >= LevelEdge[level]){
			return level;
		}
	}
	return 0;
}

unsigned short CVipTTC::GetGrowthRate(const VipData &vip_data){
	if(vip_data.days_left_rapid > 0)
		return GrowthRate[RAPID];
	else if(vip_data.days_left_fast > 0)
		return GrowthRate[FAST];
	else if(vip_data.days_left_medium > 0)
		return GrowthRate[MEDIUM];
	else if(vip_data.days_left_normal > 0)
		return GrowthRate[NORMAL];
	else if(vip_data.non_prepaid_flag != 0)
		return GrowthRate[NORMAL];
	else
		return 0;
}

unsigned char CVipTTC::GetYearFlag(const VipData &vip_data){
	return vip_data.days_left_rapid > 0 ? 1 : 0;
}

unsigned char CVipTTC::GetVipFlag(const VipData &vip_data){
	return (vip_data.days_left_rapid > 0 ||
			vip_data.days_left_fast > 0 ||
			vip_data.days_left_medium > 0 ||
			vip_data.days_left_normal > 0 ||
			vip_data.non_prepaid_flag != 0) ? 1 : 0;
}

unsigned short CVipTTC::GetLeftDays(const VipData &vip_data){
	return
		vip_data.days_left_rapid +
		vip_data.days_left_fast +
		vip_data.days_left_medium +
		vip_data.days_left_normal;
}

void CVipTTC::Adjust(VipData &vip_data, const VipData &update_data){
	if(update_data.mask & ADD_GROWTH){
		if(vip_data.growth + update_data.growth >= 0){
			vip_data.growth += update_data.growth;
		}
	}
	if(update_data.mask & ADD_DAYS_LEFT_NORMAL){
		if(vip_data.days_left_normal + update_data.days_left_normal >= 0){
			vip_data.days_left_normal += update_data.days_left_normal;
		}
	}
	if(update_data.mask & ADD_DAYS_LEFT_MEDIUM){
		if(vip_data.days_left_medium + update_data.days_left_medium >= 0){
			vip_data.days_left_medium += update_data.days_left_medium;
		}
	}
	if(update_data.mask & ADD_DAYS_LEFT_FAST){
		if(vip_data.days_left_fast + update_data.days_left_fast >= 0){
			vip_data.days_left_fast += update_data.days_left_fast;
		}
	}
	if(update_data.mask & ADD_DAYS_LEFT_RAPID){
		if(vip_data.days_left_rapid + update_data.days_left_rapid >= 0){
			vip_data.days_left_rapid += update_data.days_left_rapid;
		}
	}
	if(update_data.mask & SET_PROC_TIMESTAMP){
		vip_data.proc_timestamp = update_data.proc_timestamp;
	}
	if(update_data.mask & CHECK_AND_SET){
		vip_data.check_and_set = update_data.check_and_set;
	}
	if(update_data.mask & SET_NON_PREPAID_FLAG){
		vip_data.non_prepaid_flag = update_data.non_prepaid_flag;
	}
}


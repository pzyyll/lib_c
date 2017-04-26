// =====================================================================================
// 
//       Filename:  vip_tool.cpp
// 
//    Description:  工具
// 
//        Version:  1.0
//        Created:  2010年11月01日 15时10分17秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#include "vip_tool.h"
#include "comm/ini_file/ini_file.h"
#include <mysql.h>
#include <sstream>

using namespace petlib;

// 全局配置
int db_num = 0;
std::vector<DB_CONF> db_conf_vec;
char web_api_conf[128];
char adopt_api_conf[128];
char vip_ttc_conf[128];
char oidb_api_conf[128];

int sync_oidb = 1;
int sync_penguin = 1;
int effective_only = 0;

// 全局对象
CAdoptAPI adopt_api;
CVipTTC vip_ttc;
COIDBProxyAPI oidb_api;

// 检查是否是过去的日期
bool IsPastDay(time_t then){
	time_t now = time(NULL);

	tm tm_now = *localtime(&now);
	tm tm_then = *localtime(&then);

	if(tm_then.tm_year < tm_now.tm_year){
		return true;
	}
	else if(tm_then.tm_year == tm_now.tm_year &&
			tm_then.tm_yday < tm_now.tm_yday){
		return true;
	}
	else{
		return false;
	}
}

int OIDBModifyLevel(unsigned int uiUin, unsigned short ushLevel){
	if(sync_oidb == 0){
		return 0;
	}

	if(ushLevel > 7){
		PetLog(0, uiUin, PETLOG_ERR, "level exceed limit, level=%u", ushLevel);
		return 1;
	}

	int rv = oidb_api.SetRichFlagLevel(uiUin, OIDB_RICHFLAGLEVEL_FZ,
			static_cast<unsigned char>(ushLevel));
	if(rv != 0){
		PetLog(0, uiUin, PETLOG_ERR,
				"SetRichFlagLevel failed, rv=%d, msg=%s",
				rv, oidb_api.GetErrMsg());
		return 2;
	}

	PetLog(0, uiUin, PETLOG_TRACE, "SetRichFlagLevel|%d|%u",
			OIDB_RICHFLAGLEVEL_FZ, ushLevel);

	return 0;
}

int OIDBModifyYearFlag(unsigned int uiUin, char cYearFlag){
	if(sync_oidb == 0){
		return 0;
	}

	if(cYearFlag != 0){
		cYearFlag = 1;
	}

	int rv = oidb_api.SetMssFlag(uiUin,	OIDB_MSSTYPE_PETVIP_YEAR, cYearFlag);
	if(rv != 0){
		PetLog(0, uiUin, PETLOG_ERR,
				"SetMssFlag failed, rv=%d, msg=%s",
				rv, oidb_api.GetErrMsg());
		return 1;
	}

	PetLog(0, uiUin, PETLOG_TRACE, "SetMssFlag|%d|%d",
			OIDB_MSSTYPE_PETVIP_YEAR, cYearFlag);

	return 0;
}

int Notify(unsigned int uiUin,
		std::vector<MsgCentreMsg> &vMsg,
		UpdatePetAttrReq &stUpdatePetAttrReq){
	if(sync_penguin == 0){
		return 0;
	}

	int rv = 0;

	if(vMsg.empty() && stUpdatePetAttrReq.ullUpdateMask == 0){
		return 0;
	}

	// 查领养
	SAdoptGetResp stAdoptRsp;
	rv = adopt_api.Get(uiUin, stAdoptRsp);
	if(rv != 0){
		PetLog(0, uiUin, PETLOG_ERR, "CAdoptAPI::Get failed, rv=%d, msg=%s",
				rv, adopt_api.GetErrMsg());
		return 1;
	}

	// 通知消息中心
	if(!vMsg.empty()){
		for(int i = 0; i < stAdoptRsp.byPetNum; ++i){
			CWebApi web_api;
			rv = web_api.Initialize(web_api_conf);
			if(rv != 0){
				PetLog(0, uiUin, PETLOG_ERR,
						"CWebApi::Initialize failed, conf=%s, rv=%d, msg=%s",
						web_api_conf, rv, web_api.GetErrMsg());
				return 2;
			}

			rv = web_api.SendMsgCentreInfo(stAdoptRsp.aullPetID[i], vMsg);
			if(rv != 0){
				PetLog(0, uiUin, PETLOG_ERR,
						"CWebApi::SendMsgCentreInfo failed, rv=%d, msg=%s",
						rv, web_api.GetErrMsg());
				return 3;
			}
		}
	}

	// 通知feedsvr
	if(stUpdatePetAttrReq.ullUpdateMask != 0){
		for(int i = 0; i < stAdoptRsp.byPetNum; ++i){
			CWebApi web_api;
			rv = web_api.Initialize(web_api_conf);
			if(rv != 0){
				PetLog(0, uiUin, PETLOG_ERR,
						"CWebApi::Initialize failed, conf=%s, rv=%d, msg=%s",
						web_api_conf, rv, web_api.GetErrMsg());
				return 4;
			}

			rv = web_api.UpdatePetAttr(stAdoptRsp.aullPetID[i], stUpdatePetAttrReq);
			if(rv != 0 && rv != FEEDSVR_PET_OFFLINE){
				PetLog(0, uiUin, PETLOG_ERR,
						"CWebApi::UpdatePetAttr failed, rv=%d, msg=%s",
						rv, web_api.GetErrMsg());
				return 5;
			}
		}
	}

	return 0;
}

int LoadConf(const char *conf){
	// 打开配置文件
	CIniFile objIni(conf);
	if(!objIni.IsValid()){
		PetLog(0, 0, PETLOG_ERR, "invalid config file %s", conf)
		printf("invalid config file %s\n", conf);
		return 1;
	}

	// 读取WEBAPI配置
	objIni.GetString("global", "web_api_conf", "",
			web_api_conf, sizeof(web_api_conf));
	if(web_api_conf[0] == '\0'){
		PetLog(0, 0, PETLOG_ERR, "invalid config global:web_api_conf");
		return 2;
	}

	// 读取AdoptApi配置
	objIni.GetString("global", "adopt_api_conf", "",
			adopt_api_conf, sizeof(adopt_api_conf));
	if(adopt_api_conf[0] == '\0'){
		PetLog(0, 0, PETLOG_ERR, "invalid config global:adopt_api_conf");
		return 2;
	}

	// 读取VipTTC配置
	objIni.GetString("global", "vip_ttc_conf", "", vip_ttc_conf, sizeof(vip_ttc_conf));
	if(vip_ttc_conf[0] == '\0'){
		PetLog(0, 0, PETLOG_ERR, "invalid config global::vip_ttc_conf");
		return 2;
	}

	// 读取OIDB配置
	objIni.GetInt("global", "sync_oidb", 1, &sync_oidb);
	objIni.GetString("global", "oidb_api_conf", "", oidb_api_conf, sizeof(oidb_api_conf));
	if(oidb_api_conf[0] == '\0'){
		PetLog(0, 0, PETLOG_ERR, "invalid config global::oidb_api_conf");
		return 2;
	}

	// 同步策略
	objIni.GetInt("global", "sync_penguin", 1, &sync_penguin);

	// 读取DUMP策略
	objIni.GetInt("global", "effective_only", 1, &effective_only);

	// 读取DB配置
	objIni.GetInt("mysql", "db_num", 0, &db_num);
	if(db_num <= 0){
		PetLog(0, 0, PETLOG_ERR, "invalid config mysql::db_num %d", db_num);
		printf("invalid config mysql::db_num %d\n", db_num);
		return 2;
	}

	db_conf_vec.resize(db_num);
	for(size_t i = 0; i < db_conf_vec.size(); ++i){
		char section[128];
		snprintf(section, sizeof(section), "db%d", i);

		DB_CONF &conf = db_conf_vec[i];

		objIni.GetString(section, "addr", "", conf.addr, sizeof(conf.addr) - 1);
		objIni.GetInt(section, "port", -1, &conf.port);
		objIni.GetString(section, "user", "", conf.user, sizeof(conf.user) - 1);
		objIni.GetString(section, "passwd", "", conf.passwd, sizeof(conf.passwd) - 1);
		objIni.GetString(section, "db_name", "", conf.db_name, sizeof(conf.db_name) - 1);
		objIni.GetInt(section, "db_suffix_start", -1, &conf.db_suffix_start);
		objIni.GetInt(section, "db_suffix_end", -1, &conf.db_suffix_end);
		objIni.GetString(section, "table_name", "", conf.table_name, sizeof(conf.table_name) - 1);

		if(		conf.addr[0] == '\0' ||
				conf.port < 0 ||
				conf.user[0] == '\0' ||
				conf.passwd[0] == '\0' ||
				conf.db_name[0] == '\0' ||
				conf.db_suffix_start < 0 ||
				conf.db_suffix_end < 0 ||
				conf.db_suffix_end < conf.db_suffix_start ||
				conf.table_name[0] == '\0'){
			PetLog(0, 0, PETLOG_ERR, "config section %s error|%s|%d|%s|%s|%s|%d|%d|%s",
					section,
					conf.addr,
					conf.port,
					conf.user,
					conf.passwd,
					conf.db_name,
					conf.db_suffix_start,
					conf.db_suffix_end,
					conf.table_name);
			return 2;
		}

		PetLog(0, 0, PETLOG_TRACE, "config section %s|%s|%d|%s|%s|%s|%d|%d|%s",
				section,
				conf.addr,
				conf.port,
				conf.user,
				conf.passwd,
				conf.db_name,
				conf.db_suffix_start,
				conf.db_suffix_end,
				conf.table_name);
	}

	return LoadConfCustom(conf);
}

int InitApi(){
	int rv = 0;

	// 初始化领养接口
	rv = adopt_api.Init(adopt_api_conf);
	if(rv != 0){
		PetLog(0, 0, PETLOG_ERR, "CAdoptAPI::Init failed, conf=%s, rv=%d, msg=%s",
				adopt_api_conf, rv, adopt_api.GetErrMsg());
		return 1;
	}

	// 初始化VIPTTC
	rv = vip_ttc.Initialize(vip_ttc_conf);
	if(rv != 0){
		PetLog(0, 0, PETLOG_ERR,
				"CVipTTC::Initialize failed, conf=%s, rv=%d, msg=%s",
				vip_ttc_conf, rv, vip_ttc.GetErrMsg());
		return 2;
	}

	if(sync_oidb){
		rv = oidb_api.Init(oidb_api_conf, "vip_state_tool");
		if(rv != 0){
			PetLog(0, 0, PETLOG_ERR,
					"COIDBProxyAPI::Init failed, conf=%s, rv=%d, msg=%s",
					oidb_api_conf, rv, oidb_api.GetErrMsg());
			return 3;
		}
	}

	return InitApiCustom();
}

int DumpUin(const DB_CONF &conf, std::vector<unsigned int> &uin_vec){
	for(int db_suffix = conf.db_suffix_start;
			db_suffix <= conf.db_suffix_end; ++db_suffix){

		PetLog(0, 0, PETLOG_TRACE, "process db %u", db_suffix);

		// 初始化句柄
		MYSQL *mysql = mysql_init(NULL);
		if(mysql == NULL){
			PetLog(0, 0, PETLOG_ERR, "mysql_init failed|%d|%s",
					mysql_errno(mysql), mysql_error(mysql));
			continue;
		}

		// set options
		my_bool reconn = true;
		int rv = mysql_options(mysql, MYSQL_OPT_RECONNECT, (const char*)&reconn);
		if(rv != 0){
			PetLog(0, 0, PETLOG_ERR, "mysql_options failed|%d|%s",
					mysql_errno(mysql), mysql_error(mysql));
			mysql_close(mysql);
			mysql = NULL;
			continue;
		}

		// connect
		char db_full_name[128];
		snprintf(db_full_name, sizeof(db_full_name), "%s%d", conf.db_name, db_suffix);

		if(NULL == mysql_real_connect(mysql, conf.addr, conf.user, conf.passwd,
					db_full_name, conf.port, NULL, 0)){
			PetLog(0, 0, PETLOG_ERR, "mysql_real_connect failed|%d|%s",
					mysql_errno(mysql), mysql_error(mysql));
			mysql_close(mysql);
			mysql = NULL;
			continue;
		}

		PetLog(0, 0, PETLOG_TRACE, "mysql_real_connect %s", db_full_name);

		// 执行select
		std::ostringstream oss;
		oss << "SELECT `UIN` FROM `" << conf.table_name << "` ";
		if(effective_only){
			oss << "WHERE `GROWTH` > 0 or "
				<< "`DAYS_LEFT_NORMAL` > 0 or "
				<< "`DAYS_LEFT_MEDIUM` > 0 or "
				<< "`DAYS_LEFT_FAST` > 0 or "
				<< "`DAYS_LEFT_RAPID` > 0 or "
				<< "`FLAG1` > 0";
		}

		PetLog(0, 0, PETLOG_TRACE, "mysql query %s", oss.str().c_str());

		rv = mysql_real_query(mysql, oss.str().c_str(), oss.str().size());
		if(rv != 0){
			PetLog(0, 0, PETLOG_ERR, "mysql_real_query failed|%d|%s|%s",
					mysql_errno(mysql), mysql_error(mysql), oss.str().c_str());
			mysql_close(mysql);
			mysql = NULL;
			continue;
		}

		// 处理select结果
		MYSQL_RES *result = mysql_store_result(mysql);
		if(result == NULL){
			PetLog(0, 0, PETLOG_ERR, "mysql_store_result failed|%d|%s",
					mysql_errno(mysql), mysql_error(mysql));
			mysql_close(mysql);
			mysql = NULL;
			continue;
		}

		unsigned int rnum = mysql_num_rows(result);
		PetLog(0, 0, PETLOG_TRACE, "db %d, result row num %u", db_suffix, rnum);

		// 处理每个UIN
		MYSQL_ROW row = NULL;
		while((row = mysql_fetch_row(result)) != NULL){
			unsigned int uin = strtoul(row[0], NULL, 10);
			if(uin < 10000){
				PetLog(0, 0, PETLOG_ERR, "invalid uin %s", row[0]);
				continue;
			}

			uin_vec.push_back(uin);
		}

		PetLog(0, 0, PETLOG_TRACE, "process db %u end", db_suffix);

		mysql_close(mysql);
		mysql = NULL;
	}

	return 0;
}


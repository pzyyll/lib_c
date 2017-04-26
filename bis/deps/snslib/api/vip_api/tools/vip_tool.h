// =====================================================================================
// 
//       Filename:  vip_tool.h
// 
//    Description:	工具
// 
//        Version:  1.0
//        Created:  2010年11月01日 15时08分04秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#pragma once

#include <vector>
#include "vip_ttc.h"
#include "api/web_api/webapi.h"
#include "api/adopt_api/adopt_api.h"
#include "api/oidb_api/oidb_api.h"

// DB配置项
struct DB_CONF{
	DB_CONF() { memset(this, 0x0, sizeof(*this)); }

	char addr[128];
	int port;
	char user[128];
	char passwd[128];
	char db_name[128];
	int db_suffix_start;
	int db_suffix_end;
	char table_name[128];
};

// 全局配置
extern int db_num;
extern std::vector<DB_CONF> db_conf_vec;
extern char web_api_conf[128];
extern char adopt_api_conf[128];
extern char vip_ttc_conf[128];
extern char oidb_api_conf[128];

// 是否同步到OIDB（测试环境不需要同步）
extern int sync_oidb;
// 是否同步到企鹅系统
extern int sync_penguin;
// 只处理有效用户
extern int effective_only;

// 全局对象
extern petlib::CAdoptAPI adopt_api;
extern petlib::CVipTTC vip_ttc;
extern petlib::COIDBProxyAPI oidb_api;

// 检查是否是过去的日期
extern bool IsPastDay(time_t then);

// 更改OIDB等级
extern int OIDBModifyLevel(unsigned int uiUin, unsigned short ushLevel);

// 更改OIDB年费标记
extern int OIDBModifyYearFlag(unsigned int uiUin, char cYearFlag);

// 通知消息中心和feedsvr
extern int Notify(unsigned int uiUin,
		std::vector<petlib::MsgCentreMsg> &vMsg,
		petlib::UpdatePetAttrReq &stUpdatePetAttrReq);

// 读取配置文件
extern int LoadConf(const char *conf);
extern int LoadConfCustom(const char *conf);

// 初始化接口
extern int InitApi();
extern int InitApiCustom();

// Dump数据库UIN
extern int DumpUin(const DB_CONF &conf, std::vector<unsigned int> &uin_vec);

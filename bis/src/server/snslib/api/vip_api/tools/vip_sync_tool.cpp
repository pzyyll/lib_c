// =====================================================================================
// 
//       Filename:  vip_state_tool.cpp
// 
//    Description:  粉钻状态工具
// 
//        Version:  1.0
//        Created:  2010年09月17日 11时44分43秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#include "vip_tool.h"
#include "vip_ttc.h"
#include "vip_def.h"
#include "comm/log/pet_log.h"
#include "api/include/pet_log_def.h"
#include "comm/ini_file/ini_file.h"
#include "api/include/msg_centre_protocol.h"
#include "api/web_api/webapi.h"
#include "api/adopt_api/adopt_api.h"
#include "api/oidb_api/oidb_api.h"
#include <mysql.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <sys/time.h>
#include "c4b_api.h"

using namespace petlib;
using namespace petlib::vip;

// 开启c4b同步
int c4b_enable = 1;
// 是否用c4b的数据校正本地数据
int c4b_fix = 1;
// 是否同步消息中心
int sync_msg_center = 1;
// 是否同步session
int sync_session = 1;
// 是否带离线标记
int sync_session_offline = 1;
// 进程数量
int PROC_NUM = 20;

// c4b 相关
static C4B::CSearcher searcher;

static char sIp[64];
static char sSlaveIp[64];
static char sDefaultC4BIp[64];
static int iDefaultC4BPort = 0;

int LoadConfCustom(const char *conf){
	CIniFile objIni(conf);
	if(!objIni.IsValid()){
		PetLog(0, 0, PETLOG_ERR, "invalid config file %s", conf)
		printf("invalid config file %s\n", conf);
		return 1;
	}

	objIni.GetInt("c4b", "enable", 1, &c4b_enable);
	if(c4b_enable){
		objIni.GetInt("c4b", "fix", 1, &c4b_fix);

		objIni.GetString("c4b", "Ip", "", sIp, sizeof(sIp));
		objIni.GetString("c4b", "SlaveIp", "", sSlaveIp, sizeof(sSlaveIp));
		objIni.GetString("c4b", "DefaultC4BIp", "", sDefaultC4BIp, sizeof(sDefaultC4BIp));
		objIni.GetInt("c4b", "DefaultC4BPort", 0, &iDefaultC4BPort);

		if(		sIp[0] == '\0' ||
				sSlaveIp[0] == '\0' ||
				sDefaultC4BIp[0] == '\0' ||
				iDefaultC4BPort == 0){

			PetLog(0, 0, PETLOG_ERR, "invalid c4b config, "
					"Ip=%s, SlaveIp=%s, DefaultC4BIp=%s, DefaultC4BPort=%d",
					sIp, sSlaveIp, sDefaultC4BIp, iDefaultC4BPort);
			return 2;
		}
	}

	objIni.GetInt("sync_tool", "sync_msg_center", 0, &sync_msg_center);
	objIni.GetInt("sync_tool", "sync_session", 1, &sync_session);
	objIni.GetInt("sync_tool", "sync_session_offline", 1, &sync_session_offline);
	objIni.GetInt("sync_tool", "proc_num", 20, &PROC_NUM);

	return 0;
}

int InitApiCustom(){
	if(c4b_enable == 0){
		return 0;
	}

	// 初始化C4B
	int rv = searcher.initServer(sIp, sSlaveIp, sDefaultC4BIp, iDefaultC4BPort);
	if(rv != 0){
		PetLog(0, 0, PETLOG_ERR,
				"C4B::CSearcher::initServer failed, rv=%d, msg=%s",
				rv, searcher.getLastError());
		return 1;
	}

	return 0;
}

int CheckAndFix(unsigned int uin, CVipTTC::VipData &ttc_data, const C4B::BossItem *bossItem){

	CVipTTC::VipData update_data;

	if(!bossItem){
		// 不是粉钻用户
		if(CVipTTC::GetVipFlag(ttc_data) == 1){
			PetLog(LOG_VIP_C4B_MISMATCH, uin, PETLOG_WARN,
					"ttc vip not in c4b");
		}

		if(ttc_data.days_left_normal){
			update_data.days_left_normal = -ttc_data.days_left_normal;
			update_data.mask |= CVipTTC::ADD_DAYS_LEFT_NORMAL;
		}
		if(ttc_data.days_left_medium){
			update_data.days_left_medium = -ttc_data.days_left_medium;
			update_data.mask |= CVipTTC::ADD_DAYS_LEFT_MEDIUM;
		}
		if(ttc_data.days_left_fast){
			update_data.days_left_fast = -ttc_data.days_left_fast;
			update_data.mask |= CVipTTC::ADD_DAYS_LEFT_FAST;
		}
		if(ttc_data.days_left_rapid){
			update_data.days_left_rapid = -ttc_data.days_left_rapid;
			update_data.mask |= CVipTTC::ADD_DAYS_LEFT_RAPID;
		}
		if(ttc_data.non_prepaid_flag != 0){
			update_data.non_prepaid_flag = 0;
			update_data.mask |= CVipTTC::SET_NON_PREPAID_FLAG;
		}
	}
	else if(bossItem->m_payWay == 1 || bossItem->m_payWay == 3){
		// 包月用户
		if(ttc_data.non_prepaid_flag != bossItem->m_payWay){
			PetLog(LOG_VIP_C4B_MISMATCH, uin, PETLOG_WARN,
					"pay_way mismatch|%d|%d",
					ttc_data.non_prepaid_flag, bossItem->m_payWay);

			update_data.non_prepaid_flag = bossItem->m_payWay;
			update_data.mask |= CVipTTC::SET_NON_PREPAID_FLAG;
		}
	}
	else /*if(bossItem->m_payWay == 0)*/{

		// 预付费用户
		if(ttc_data.non_prepaid_flag != 0){
			// 如果有包月标记，去掉
			PetLog(LOG_VIP_C4B_MISMATCH, uin, PETLOG_WARN,
					"prepaid user has non-prepaid flag|%d",
					ttc_data.non_prepaid_flag);

			update_data.non_prepaid_flag = 0;
			update_data.mask |= CVipTTC::SET_NON_PREPAID_FLAG;
		}

		// 校验关闭时间
		const time_t now_time = ::time(NULL);
		unsigned int close_time = bossItem->m_iCloseTime;

		unsigned int c4b_days_left = 0;

		if(close_time > (unsigned int)now_time){
			tm now_tm = *localtime(&now_time);

			now_tm.tm_hour = 0;
			now_tm.tm_min = 0;
			now_tm.tm_sec = 0;

			time_t now_t = mktime(&now_tm);

			c4b_days_left = ( close_time - now_t ) / ( 3600 * 24 ) + 1;
		}

		const unsigned int ttc_days_left = CVipTTC::GetLeftDays(ttc_data);

		if(ttc_days_left != c4b_days_left){
			// 到期时间不匹配
			PetLog(LOG_VIP_C4B_MISMATCH, uin, PETLOG_WARN,
					"days left mismatch|%u|%d",
					ttc_days_left, c4b_days_left);

			if(ttc_days_left < c4b_days_left){
				// 本地少了，加给他中速
				update_data.days_left_medium = c4b_days_left - ttc_days_left;
				update_data.mask |= CVipTTC::ADD_DAYS_LEFT_MEDIUM;
			}
			else if(ttc_days_left > c4b_days_left){
				// 本地多了，从小的开始减
				int sub_days = ttc_days_left - c4b_days_left;

				if(sub_days > 0 && ttc_data.days_left_normal > 0){
					int days = std::min(sub_days, ttc_data.days_left_normal);
					update_data.days_left_normal = -days;
					update_data.mask |= CVipTTC::ADD_DAYS_LEFT_NORMAL;
					sub_days -= days;
				}
				if(sub_days > 0 && ttc_data.days_left_medium > 0){
					int days = std::min(sub_days, ttc_data.days_left_medium);
					update_data.days_left_medium = -days;
					update_data.mask |= CVipTTC::ADD_DAYS_LEFT_MEDIUM;
					sub_days -= days;
				}
				if(sub_days > 0 && ttc_data.days_left_fast > 0){
					int days = std::min(sub_days, ttc_data.days_left_fast);
					update_data.days_left_fast = -days;
					update_data.mask |= CVipTTC::ADD_DAYS_LEFT_FAST;
					sub_days -= days;
				}
				if(sub_days > 0 && ttc_data.days_left_rapid > 0){
					int days = std::min(sub_days, ttc_data.days_left_rapid);
					update_data.days_left_rapid = -days;
					update_data.mask |= CVipTTC::ADD_DAYS_LEFT_RAPID;
					sub_days -= days;
				}
			}
		}
	}

	// 需要修复
	if(c4b_fix && update_data.mask != 0){
		int rv = vip_ttc.SetVipData(uin, update_data);
		if(rv != 0){
			PetLog(0, uin, PETLOG_ERR, "CVipTTC::SetVipData failed|%d|%s",
					rv, vip_ttc.GetErrMsg());
			return 1;
		}

		PetLog(LOG_VIP_C4B_MISMATCH, uin, PETLOG_INFO,
				"Fix|%d|%d|%d|%d|%d|%d",
				update_data.mask,
				update_data.days_left_normal,
				update_data.days_left_medium,
				update_data.days_left_fast,
				update_data.days_left_rapid,
				update_data.non_prepaid_flag);

		CVipTTC::Adjust(ttc_data, update_data);
	}

	return 0;
}

int HandleUin(unsigned int uin){
	// 获取VIP数据
	CVipTTC::VipData ttc_data;
	int rv = vip_ttc.GetVipData(uin, ttc_data);
	if(rv != 0 && rv != E_TTC_NO_REC){
		PetLog(0, uin, PETLOG_ERR, "CVipTTC::GetVipData failed|%d|%s",
				rv, vip_ttc.GetErrMsg());
		return 1;
	}

	// 跟数平同步
	if(c4b_enable){
		// 获取C4B数据
		C4B::BossItem bossItem[16];
		memset(bossItem, 0x0, sizeof(bossItem));
		unsigned int iItemNum = 16;
		char sUserNum[32];
		snprintf(sUserNum, sizeof(sUserNum), "%u", uin);

		rv = searcher.getByUserNumServiveType(
				C4B::SERVICE_CLASS_QQ,
				sUserNum,
				"PETVIP",
				bossItem,
				iItemNum);
		if(rv != 0){
			PetLog(0, uin, PETLOG_ERR,
					"C4B::CSearcher::getByUserNumServiveType(1) failed, rv=%d, msg=%s",
					rv, searcher.getLastError());
			return 2;
		}

		if(iItemNum == 0){	// PETVIP用户
			iItemNum = 16;
			rv = searcher.getByUserNumServiveType(
					C4B::SERVICE_CLASS_QQ,
					sUserNum,
					"QQPETWX",
					bossItem,
					iItemNum);
			if(rv != 0){
				PetLog(0, uin, PETLOG_ERR,
						"C4B::CSearcher::getByUserNumServiveType(2) failed, rv=%d, msg=%s",
						rv, searcher.getLastError());
				return 3;
			}
		}

		if(iItemNum == 0){
			rv = CheckAndFix(uin, ttc_data, NULL);
		}
		else{
			rv = CheckAndFix(uin, ttc_data, &bossItem[0]);
		}

		if(rv != 0){
			return 4;
		}
	}

	PetLog(LOG_VIP_SYNC_PENGUIN, uin, PETLOG_INFO,
			"sync|%d|%d|%d|%d|%d|%d",
			ttc_data.growth,
			ttc_data.days_left_normal,
			ttc_data.days_left_medium,
			ttc_data.days_left_fast,
			ttc_data.days_left_rapid,
			ttc_data.proc_timestamp );

	// 通知消息中心和feedsvr的结构
	std::vector<MsgCentreMsg> vMsg;
	UpdatePetAttrReq stUpdatePetAttrReq;
	memset(&stUpdatePetAttrReq, 0x0, sizeof(stUpdatePetAttrReq));

	if(sync_msg_center){
		MsgCentreMsg msg;

		memset(&msg, 0x0, sizeof(msg));
		msg.ushMsgType = MCID_MODIFY_VIPFLAG;
		msg.ushMsgValLen = 1;
		msg.szMsgVal[0] = CVipTTC::GetVipFlag(ttc_data);
		vMsg.push_back(msg);

		memset(&msg, 0x0, sizeof(msg));
		msg.ushMsgType = MCID_MODIFY_VIPLEVEL;
		msg.ushMsgValLen = 2;
		*reinterpret_cast<unsigned short*>(msg.szMsgVal) =
			CVipTTC::GetLevel(ttc_data);
		vMsg.push_back(msg);

		memset(&msg, 0x0, sizeof(msg));
		msg.ushMsgType = MCID_MODIFY_VIPYEARFLAG;
		msg.ushMsgValLen = 1;
		msg.szMsgVal[0] = CVipTTC::GetYearFlag(ttc_data);
		vMsg.push_back(msg);
	}

	if(sync_session){
		stUpdatePetAttrReq.cVipFlag = CVipTTC::GetVipFlag(ttc_data);
		stUpdatePetAttrReq.ullUpdateMask |= UPDATE_VIP_FLAG;

		stUpdatePetAttrReq.ushVipLevel = CVipTTC::GetLevel(ttc_data);
		stUpdatePetAttrReq.ullUpdateMask |= UPDATE_VIP_LEVEL;

		stUpdatePetAttrReq.ucVipYearFlag = CVipTTC::GetYearFlag(ttc_data);
		stUpdatePetAttrReq.ullUpdateMask |= UPDATE_VIP_YEAR_FLAG;

		stUpdatePetAttrReq.ushOffLineFlag = sync_session_offline > 0 ? 1 : 0;
	}

	OIDBModifyLevel(uin, CVipTTC::GetLevel(ttc_data));
	OIDBModifyYearFlag(uin, CVipTTC::GetYearFlag(ttc_data));

	Notify(uin, vMsg, stUpdatePetAttrReq);

	return 0;
}

int main(int argc, const char *argv[]){
	// open log
	OpenPetLog("vip_sync_tool");

	// check param
	if(argc < 2){
		PetLog(0, 0, PETLOG_ERR, "bad argument number %d", argc)
		printf("usage: %s config [uin]\n", argv[0]);
		return 1;
	}

	// load config file
	int rv = LoadConf(argv[1]);
	if(rv != 0){
		return 2;
	}

	// handle single uin ?
	if(argc >= 3){
		int rv = 0;

		unsigned int uin = strtoul(argv[2], NULL, 10);
		if(uin >= 10000){
			rv = InitApi();
			if(rv != 0){
				return 1;
			}

			return HandleUin(uin);
		}
		else if(uin == 0){
			std::ifstream file(argv[2]);
			if(!file){
				PetLog(0, 0, PETLOG_ERR, "invalid file %s", argv[2]);
				printf("invalid file %s\n", argv[2]);
				return 1;
			}

			std::vector<unsigned int> uin_vec;
			while(file >> uin){
				uin_vec.push_back(uin);
			}

			for(int idx = 0; idx < PROC_NUM; ++idx){
				pid_t pid = fork();
				if(pid == 0){
					rv = InitApi();
					if(rv != 0){
						return 1;
					}

					for(size_t i = 0; i * PROC_NUM + idx < uin_vec.size(); ++i){
						rv = HandleUin(uin_vec[i * PROC_NUM + idx]);
					}
					return rv;
				}
			}

			return 0;
		}
		else{
			PetLog(0, 0, PETLOG_ERR, "invalid config %s", argv[2]);
			printf("invalid config %s\n", argv[2]);
			return 1;
		}

		return rv;
	}

	std::vector<unsigned int> uin_vec;

	for(int i = 0; i < db_num; ++i){
		// dump all uin from db
		const DB_CONF &conf = db_conf_vec[i];
		rv = DumpUin(conf, uin_vec);
		if(rv != 0){
			PetLog(0, 0, PETLOG_ERR, "DumpUin failed|%d", rv);
			return 1;
		}
	}

	for(int idx = 0; idx < PROC_NUM; ++idx){
		pid_t pid = fork();
		if(pid == 0){
			rv = InitApi();
			if(rv != 0){
				return 1;
			}

			for(size_t i = 0; i * PROC_NUM + idx < uin_vec.size(); ++i){
				rv = HandleUin(uin_vec[i * PROC_NUM + idx]);
			}
			return rv;
		}
	}

	return 0;
}


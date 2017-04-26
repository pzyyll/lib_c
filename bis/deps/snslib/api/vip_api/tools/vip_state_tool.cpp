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
#include <sys/wait.h>

using namespace petlib;
using namespace petlib::vip;

int check_timestamp = 1;
int enable_sub = 1;

int LoadConfCustom(const char *conf){
	CIniFile objIni(conf);
	if(!objIni.IsValid()){
		PetLog(0, 0, PETLOG_ERR, "invalid config file %s", conf)
		printf("invalid config file %s\n", conf);
		return 1;
	}

	objIni.GetInt("state_tool", "check_timestamp", 1, &check_timestamp);
	objIni.GetInt("state_tool", "enable_sub", 1, &enable_sub);

	return 0;
}

int InitApiCustom(){
	return 0;
}

static int HandleUin(unsigned int uin){
	// 获取数据
	CVipTTC::VipData ttc_data;
	int rv = vip_ttc.GetVipData(uin, ttc_data);
	if(rv != 0){
		PetLog(0, uin, PETLOG_ERR, "CVipTTC::GetVipData(1) failed|%d|%s",
				rv, vip_ttc.GetErrMsg());
		return 1;
	}

	// 今天处理过了
	if(check_timestamp && !IsPastDay(ttc_data.proc_timestamp)){
		PetLog(0, uin, PETLOG_TRACE, "handled today|%u",
				ttc_data.proc_timestamp);
		return 0;
	}

	// TTC更新结构
	CVipTTC::VipData update_data;
	memset(&update_data, 0x0, sizeof(update_data));

	// 过期粉钻，每天扣15点
	if(0 == CVipTTC::GetVipFlag(ttc_data)){
		if(ttc_data.growth > 0){
			if(enable_sub){
				update_data.growth = -(std::min(ttc_data.growth, 15));
				update_data.mask |= CVipTTC::ADD_GROWTH;
			}
		}
		else if(ttc_data.growth < 0){
			PetLog(0, uin, PETLOG_ERR, "growth(%d) < 0, reset", ttc_data.growth);
			update_data.growth = -update_data.growth;
			update_data.mask |= CVipTTC::ADD_GROWTH;
		}
	}
	// 正常粉钻
	else{
		if(ttc_data.days_left_rapid > 0){
			update_data.growth = GrowthRate[RAPID];
			update_data.mask |= CVipTTC::ADD_GROWTH;
			update_data.days_left_rapid = -1;
			update_data.mask |= CVipTTC::ADD_DAYS_LEFT_RAPID;
		}
		else if(ttc_data.days_left_fast > 0){
			update_data.growth = GrowthRate[FAST];
			update_data.mask |= CVipTTC::ADD_GROWTH;
			update_data.days_left_fast = -1;
			update_data.mask |= CVipTTC::ADD_DAYS_LEFT_FAST;
		}
		else if(ttc_data.days_left_medium > 0){
			update_data.growth = GrowthRate[MEDIUM];
			update_data.mask |= CVipTTC::ADD_GROWTH;
			update_data.days_left_medium = -1;
			update_data.mask |= CVipTTC::ADD_DAYS_LEFT_MEDIUM;
		}
		else if(ttc_data.days_left_normal > 0){
			update_data.growth = GrowthRate[NORMAL];
			update_data.mask |= CVipTTC::ADD_GROWTH;
			update_data.days_left_normal = -1;
			update_data.mask |= CVipTTC::ADD_DAYS_LEFT_NORMAL;
		}
		else if(ttc_data.non_prepaid_flag != 0){
			update_data.growth = GrowthRate[NORMAL];
			update_data.mask |= CVipTTC::ADD_GROWTH;
		}
		else{
			PetLog(0, uin, PETLOG_ERR, "vip out of date(why here?)");
			return 1;
		}
	}

	// 设置时间戳
	update_data.proc_timestamp = ::time(NULL);
	update_data.mask |= CVipTTC::SET_PROC_TIMESTAMP;

	// 更新数据
	rv = vip_ttc.SetVipData(uin, update_data);
	if(rv != 0){
		PetLog(0, uin, PETLOG_ERR,
				"CVipTTC::SetVipData failed|%d|%s",
				rv, vip_ttc.GetErrMsg());
		return 1;
	}

	// 获取新后的数据
	CVipTTC::VipData ttc_data_new = ttc_data;
	CVipTTC::Adjust(ttc_data_new, update_data);

	PetLog(LOG_VIP_STATE_PROC, uin, PETLOG_INFO,
			"|Old|%d|%d|%d|%d|%d|%d"
			"|Update|%x|%d|%d|%d|%d|%d|%d"
			"|New|%d|%d|%d|%d|%d|%d",
			ttc_data.growth,
			ttc_data.days_left_normal,
			ttc_data.days_left_medium,
			ttc_data.days_left_fast,
			ttc_data.days_left_rapid,
			ttc_data.proc_timestamp,

			update_data.mask,
			update_data.growth,
			update_data.days_left_normal,
			update_data.days_left_medium,
			update_data.days_left_fast,
			update_data.days_left_rapid,
			update_data.proc_timestamp,

			ttc_data_new.growth,
			ttc_data_new.days_left_normal,
			ttc_data_new.days_left_medium,
			ttc_data_new.days_left_fast,
			ttc_data_new.days_left_rapid,
			ttc_data_new.proc_timestamp	);

	// 通知消息中心和feedsvr的结构
	UpdatePetAttrReq stUpdatePetAttrReq;
	memset(&stUpdatePetAttrReq, 0x0, sizeof(stUpdatePetAttrReq));

	// 粉钻标记位改变
	if(CVipTTC::GetVipFlag(ttc_data) != CVipTTC::GetVipFlag(ttc_data_new)){
		stUpdatePetAttrReq.ushOffLineFlag = 1;
		stUpdatePetAttrReq.ushVipLevel = CVipTTC::GetVipFlag(ttc_data_new);
		stUpdatePetAttrReq.ullUpdateMask |= UPDATE_VIP_FLAG;
	}

	// 粉钻升级
	// 2011-1-21 粉钻等级全量同步到OIDB，FEEDSVR，MSG_CENTER
	// 有些活动赠送粉钻成长值会导致等级无法同步到这些地方
	stUpdatePetAttrReq.ushOffLineFlag = 0;
	stUpdatePetAttrReq.ushVipLevel = CVipTTC::GetLevel(ttc_data_new);
	stUpdatePetAttrReq.ullUpdateMask |= UPDATE_VIP_LEVEL;

	MsgCentreMsg stMsgCentreMsg;
	stMsgCentreMsg.ushMsgType = MCID_MODIFY_VIPLEVEL;
	stMsgCentreMsg.ushMsgValLen = 2;
	*(reinterpret_cast<unsigned short *>(stMsgCentreMsg.szMsgVal)) =
		CVipTTC::GetLevel(ttc_data_new);
	std::vector<MsgCentreMsg> vMsg;
	vMsg.push_back(stMsgCentreMsg);
	
	OIDBModifyLevel(uin, CVipTTC::GetLevel(ttc_data_new));

	// 年份标记位改变
	if(CVipTTC::GetYearFlag(ttc_data) != CVipTTC::GetYearFlag(ttc_data_new)){
		stUpdatePetAttrReq.ushOffLineFlag = 1;
		stUpdatePetAttrReq.ucVipYearFlag = CVipTTC::GetYearFlag(ttc_data_new);;
		stUpdatePetAttrReq.ullUpdateMask |= UPDATE_VIP_YEAR_FLAG;

		OIDBModifyYearFlag(uin, CVipTTC::GetYearFlag(ttc_data_new));
	}

	Notify(uin, vMsg, stUpdatePetAttrReq);

	return 0;
}

int main(int argc, const char *argv[]){
	// open log
	OpenPetLog("vip_state_tool");

	// check param
	if(argc < 2){
		PetLog(0, 0, PETLOG_ERR, "bad argument number %d", argc)
		printf("usage: %s config\n", argv[0]);
		return 1;
	}

	// load config file
	int rv = LoadConf(argv[1]);
	if(rv != 0){
		return 1;
	}

	// handle single uin ?
	if(argc >= 3){
		// init all api
		rv = InitApi();
		if(rv != 0){
			PetLog(0, 0, PETLOG_ERR, "Init failed, rv=%d", rv);
			return 1;
		}

		unsigned int uin = strtoul(argv[2], NULL, 10);
		if(uin < 10000){
			PetLog(0, 0, PETLOG_ERR, "invalid uin %s", argv[2]);
			printf("invalid uin %s\n", argv[2]);
			return 1;
		}

		return HandleUin(uin);
	}

	// fork & process

	PetLog(0, 0, PETLOG_INFO, "========== BEGIN ==========");

	for(int i = 0; i < db_num; ++i){
		pid_t pid = fork();
		if(pid < 0){
			PetLog(0, 0, PETLOG_ERR, "fork failed");
			return 1;
		}
		else if(pid == 0){
			printf("process(%d,%d) started\n", i, getpid());

			// init all api
			rv = InitApi();
			if(rv != 0){
				PetLog(0, 0, PETLOG_ERR, "Init failed, rv=%d", rv);
				return 1;
			}

			const DB_CONF &conf = db_conf_vec[i];
			// dump uin from db
			std::vector<unsigned int> uin_vec;
			rv = DumpUin(conf, uin_vec);
			if(rv != 0){
				PetLog(0, 0, PETLOG_ERR, "DumpUin failed|%d|%d", i, rv);
				return 1;
			}

			// handle all uin
			for(size_t i = 0; i < uin_vec.size(); ++i){
				HandleUin(uin_vec[i]);
			}

			return 0;
		}
	}

	for(int i = 0; i < db_num; ++i){
		int status;
		pid_t pid = wait(&status);
		printf("process(%d,%d) ended with %d\n", i, pid, status);
	}

	PetLog(0, 0, PETLOG_INFO, "========== END ==========");

	return 0;
}


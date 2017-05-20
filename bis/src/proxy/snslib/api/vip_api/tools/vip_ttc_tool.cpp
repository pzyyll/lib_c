// =====================================================================================
// 
//       Filename:  vip_ttc_tool.cpp
// 
//    Description:  vip_ttc工具
// 
//        Version:  1.0
//        Created:  2010年09月21日 16时40分17秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#include "../vip_ttc.h"
#include "../vip_def.h"

using namespace petlib;

void usage(int argc, const char *argv[]){
	printf("Usage: %s config uin get/set/add [key value]\n", argv[0]);
	printf("Set: proc_timestamp, check_and_set, non_prepaid_flag\n");
	printf("Add: growth, days_left_normal, days_left_medium, days_left_fast, days_left_rapid\n");
}

int main(int argc, const char *argv[]){
	if(argc < 4){
		usage(argc, argv);
		return 1;
	}

	const char *config = argv[1];
	unsigned int uin = strtoul(argv[2], NULL, 10);
	const char *oper = argv[3];

	CVipTTC vip_ttc;
	
	int rv = vip_ttc.Initialize(config);
	if(rv != 0){
		printf("CVipTTC::Initialize failed, rv=%d, msg=%s\n", rv, vip_ttc.GetErrMsg());
		return 1;
	}

	if(strcmp(oper, "get") == 0){
		CVipTTC::VipData vip_data;
		int rv = vip_ttc.GetVipData(uin, vip_data);
		if(rv == vip::E_TTC_NO_REC){
			printf("uin(%u) not in db\n", uin);
			return 1;
		}
		else if(rv != 0){
			printf("CVipTTC::GetVipData failed, rv=%d, msg=%s\n", rv, vip_ttc.GetErrMsg());
			return 1;
		}
		
		printf("growth					= %d\n", vip_data.growth);
		printf("days_left_normal		= %d\n", vip_data.days_left_normal);
		printf("days_left_medium		= %d\n", vip_data.days_left_medium);
		printf("days_left_fast			= %d\n", vip_data.days_left_fast);
		printf("days_left_rapid			= %d\n", vip_data.days_left_rapid);
		time_t proc_timestamp = vip_data.proc_timestamp;
		printf("proc_timestamp			= %d %s", vip_data.proc_timestamp, ctime(&proc_timestamp));
		printf("check_and_set			= %d\n", vip_data.check_and_set);
		printf("non_prepaid_flag		= %x\n", vip_data.non_prepaid_flag);
	}
	else if(strcmp(oper, "set") == 0){
		CVipTTC::VipData vip_data;
		memset(&vip_data, 0x0, sizeof(vip_data));

		for(int i = 4; i + 1 < argc; i += 2){
			if(strcmp(argv[i], "proc_timestamp") == 0){
				vip_data.proc_timestamp = strtoul(argv[i+1], NULL, 10);
				vip_data.mask |= CVipTTC::SET_PROC_TIMESTAMP;
			}
			else if(strcmp(argv[i], "check_and_set") == 0){
				vip_data.check_and_set = strtoul(argv[i+1], NULL, 10);
				vip_data.mask |= CVipTTC::CHECK_AND_SET;
			}
			else if(strcmp(argv[i], "non_prepaid_flag") == 0){
				vip_data.non_prepaid_flag = strtol(argv[i+1], NULL, 16);
				vip_data.mask |= CVipTTC::SET_NON_PREPAID_FLAG;
			}
			else{
				usage(argc, argv);
				return 1;
			}
		}
		if(vip_data.mask != 0){
			int rv = vip_ttc.SetVipData(uin, vip_data);
			if(rv != 0){
				printf("CVipTTC::SetVipData failed, rv=%d, msg=%s\n",
						rv, vip_ttc.GetErrMsg());
				return 1;
			}
			else {
				printf("SUCCESS\n");
			}
		}
	}
	else if(strcmp(oper, "add") == 0){
		CVipTTC::VipData vip_data;
		memset(&vip_data, 0x0, sizeof(vip_data));

		for(int i = 4; i + 1 < argc; i += 2){
			if(strcmp(argv[i], "growth") == 0){
				vip_data.growth = strtoul(argv[i+1], NULL, 0);
				vip_data.mask |= CVipTTC::ADD_GROWTH;
				printf("add growth + %d\n", vip_data.growth);
			}
			else if(strcmp(argv[i], "days_left_normal") == 0){
				vip_data.days_left_normal = strtoul(argv[i+1], NULL, 0);
				vip_data.mask |= CVipTTC::ADD_DAYS_LEFT_NORMAL;
				printf("add days_left_normal + %d\n", vip_data.days_left_normal);
			}
			else if(strcmp(argv[i], "days_left_medium") == 0){
				vip_data.days_left_medium = strtoul(argv[i+1], NULL, 0);
				vip_data.mask |= CVipTTC::ADD_DAYS_LEFT_MEDIUM;
				printf("add days_left_medium + %d\n", vip_data.days_left_medium);
			}
			else if(strcmp(argv[i], "days_left_fast") == 0){
				vip_data.days_left_fast = strtoul(argv[i+1], NULL, 0);
				vip_data.mask |= CVipTTC::ADD_DAYS_LEFT_FAST;
				printf("add days_left_fast + %d\n", vip_data.days_left_fast);
			}
			else if(strcmp(argv[i], "days_left_rapid") == 0){
				vip_data.days_left_rapid = strtoul(argv[i+1], NULL, 0);
				vip_data.mask |= CVipTTC::ADD_DAYS_LEFT_RAPID;
				printf("add days_left_rapid + %d\n", vip_data.days_left_rapid);
			}
			else{
				usage(argc, argv);
				return 1;
			}
		}

		if(vip_data.mask != 0){
			int rv = vip_ttc.SetVipData(uin, vip_data);
			if(rv != 0){
				printf("CVipTTC::SetVipData failed, rv=%d, msg=%s\n",
						rv, vip_ttc.GetErrMsg());
				return 1;
			}
			else {
				printf("SUCCESS\n");
			}
		}
	}
	else{
		usage(argc, argv);
		return 1;
	}

	return 0;
}


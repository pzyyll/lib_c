// =====================================================================================
// 
//       Filename:  vip_tool.cpp
// 
//    Description:  工具
// 
//        Version:  1.0
//        Created:  2010年09月10日 14时47分36秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#include "vip_api.h"
#include "vip_def.h"
#include <iostream>

using namespace std;
using namespace petlib;
using namespace petlib::vip;

int main(int argc, const char *argv[]){
	if(argc < 3){
		cout << "Usage: " << argv[0] << " conf uin" << endl;
		return 1;
	}

	const char *conf = argv[1];
	unsigned int uin = strtoul(argv[2], NULL, 10);

	cout << uin << endl;

	CVipApi api;

	int ecode;

	ecode = api.Init(conf);
	if(E_SUCCESS != ecode){
		cout << "CVipApi::Init failed, " << ecode << " " << api.GetErrMsg() << endl;
		return 1;
	}

	CVipApi::VipData data;
	ecode = api.GetVipData(uin, data);
	if(E_SUCCESS != ecode){
		cout << "CVipApi::GetVipData failed, " << ecode << " " << api.GetErrMsg() << endl;
		return 1;
	}

	cout <<
		"growth = "				<< data.growth << endl <<
		"days_left_normal = "	<< data.days_left_normal << endl <<
		"days_left_medium = "	<< data.days_left_medium << endl <<
		"days_left_fast = "		<< data.days_left_fast << endl <<
		"days_left_rapid = "	<< data.days_left_rapid << endl <<
		"vip_flag = "			<< static_cast<int>(data.vip_flag) << endl <<
		"year_flag = "			<< static_cast<int>(data.year_flag) << endl <<
		"level = "				<< data.level << endl <<
		"growth_rate = "		<< data.growth_rate << endl <<
		endl;

	return 0;
}


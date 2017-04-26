// =====================================================================================
// 
//       Filename:  c4b.cpp
// 
//    Description:  拉取粉钻过期
// 
//        Version:  1.0
//        Created:  2010年11月02日 14时10分32秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#include "c4b_api.h"
#include <iostream>

using namespace std;

int main(int argc, const char *argv[])
{
	if(argc < 2){
		cout << "Usage: c4b uin" << endl;
		return 1;
	}

	C4B::CSearcher searcher;

	int rv = searcher.initServer("172.16.61.73", "172.23.20.48", "172.23.20.95", 11001);
	if(rv != 0){
		cout << "init server failed" << endl;
		return 1;
	}

	C4B::BossItem bossItem[16];
	memset(bossItem, 0x0, sizeof(bossItem));
	unsigned int iItemNum = 16;

	rv = searcher.getByUserNumServiveType(
			C4B::SERVICE_CLASS_QQ,
			argv[1],
			"PETVIP",
			bossItem,
			iItemNum);
	if(rv != 0){
		cout << "get failed" << endl;
		return 1;
	}

	if(iItemNum == 0){
		iItemNum = 16;
		rv = searcher.getByUserNumServiveType(
				C4B::SERVICE_CLASS_QQ,
				argv[1],
				"QQPETWX",
				bossItem,
				iItemNum);
		if(rv != 0){
			cout << "get failed" << endl;
			return 1;
		}
	}

	if(iItemNum == 1){
		tm open_tm, close_tm;
		C4B::BossItem::my_localtime_r(&bossItem[0].m_iOpenTime, &open_tm);
		C4B::BossItem::my_localtime_r(&bossItem[0].m_iCloseTime, &close_tm);

		cout << bossItem[0].m_serviceClass << endl;
		cout << bossItem[0].m_sUserNum << endl;
		cout << bossItem[0].m_payWay << endl;
		cout << bossItem[0].m_sPayNum << endl;
		cout << bossItem[0].m_sServiceType << endl;
		cout << bossItem[0].m_iOpenTime << "\t" << open_tm.tm_year + 1900 << " " <<
			open_tm.tm_mon + 1 << " " << open_tm.tm_mday << endl;
		cout << bossItem[0].m_iCloseTime << "\t" << close_tm.tm_year + 1900 << " " <<
			close_tm.tm_mon + 1 << " " << close_tm.tm_mday << endl;
		cout << bossItem[0].m_cState << endl;
		cout << endl;

		time_t now_time = ::time(NULL);
		unsigned int c4b_close_time = bossItem[0].m_iCloseTime;

		unsigned int c4b_days_left = 0;

		if(c4b_close_time > (unsigned int)now_time){
			tm now_tm = *localtime(&now_time);

			now_tm.tm_hour = 0;
			now_tm.tm_min = 0;
			now_tm.tm_sec = 0;

			time_t now_t = mktime(&now_tm);

			c4b_days_left = ( c4b_close_time - now_t ) / ( 3600 * 24 ) + 1;
		}
		cout << "days left:" << c4b_days_left << endl;
	}

	return 0;
}

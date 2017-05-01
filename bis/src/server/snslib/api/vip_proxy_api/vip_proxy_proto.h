// =====================================================================================
// 
//       Filename:  vip_proxy_proto.h
// 
//    Description:  协议
// 
//        Version:  1.0
//        Created:  2010年11月30日 10时21分14秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#pragma once

namespace snslib{

#pragma pack(1)

struct CmdGetVipDataReq{
	unsigned int dwUin;
};

struct CmdGetVipDataRsp{
	unsigned int dwUin;
	int dwResult;

	struct{
		unsigned int		growth;				// 成长值
		unsigned short		days_left_normal;	// 普通成长
		unsigned short		days_left_medium;	// 中速成长
		unsigned short		days_left_fast;		// 快速成长
		unsigned short		days_left_rapid;	// 极速成长

		unsigned char		vip_flag;			// 粉钻标记 1是 0不是
		unsigned char		year_flag;			// 年费标记
		unsigned short		level;				// 等级
		unsigned short		growth_rate;		// 成长速度
	};
};

#pragma pack()

}

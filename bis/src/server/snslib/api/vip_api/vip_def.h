// =====================================================================================
// 
//       Filename:  vip_def.h
// 
//    Description:  vip定义
// 
//        Version:  1.0
//        Created:  2010年09月15日 15时44分57秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#pragma once

namespace snslib{
namespace vip{

/*
 * 返回码枚举
 */
enum{
	E_SUCCESS = 0,				// 执行成功
	E_BAD_CONFIG_FILE,			// 配置文件错误
	E_TTC_INIT,					// TTC初始化错误
	E_TTC_INVALID,				// 无效的TTC对象
	E_TTC_GET,					// TTC获取时失败
	E_TTC_SET,					// TTC设置时失败
	E_TTC_NO_REC,				// TTC查询无数据
	E_TTC_TOUCH,				// 创建初始数据时失败
	E_TTC_NO_AFFECTED,			// TT设置时没有更改任何数据
};

/* 
 * 等级枚举
 */
enum{ LV0 = 0, LV1 = 1, LV2 = 2, LV3 = 3, LV4 = 4, LV5 = 5, LV6 = 6, LV7 = 7, LV_MAX };

/* 
 * 成长速度枚举
 */
enum{ NONE = 0, NORMAL = 1, MEDIUM = 2, FAST = 3, RAPID = 4 };

/* 
 * ----------------------------------------------------
 * |粉钻等级| LV1 | LV2 | LV3 | LV4 | LV5 | LV6 | LV7 |
 * ----------------------------------------------------
 * | 成长值 |  0  | 300 | 900 |2400 |4500 |7200 |14400|
 * ----------------------------------------------------
 */
const unsigned int LevelEdge[LV_MAX] = {0, 0, 300, 900, 2400, 4500, 7200, 14400};

/*
 * --------------------------------------
 * |成长速度| 普通 | 中速 | 高速 | 急速 |
 * --------------------------------------
 * | 点/天  |  10  |  12  |  13  |  15  |
 * --------------------------------------
 */
const unsigned int GrowthRate[LV_MAX] = {0, 10, 12, 13, 15};

/*
 * 粉钻等级对应SNSGAME特权喂养次数
 * --------------------------------------
 * |等级| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 * --------------------------------------
 * |次数| 0 | 1 | 1 | 2 | 3 | 4 | 5 | 8 |
 * --------------------------------------
 */
const unsigned int SNSTimes[LV_MAX] = {1, 1, 1, 2, 3, 4, 5, 8};

/*
 * 粉钻等级对应状态特权次数
 * 次数详见CPP
 */
extern unsigned short StatusTimes(unsigned int status, unsigned short vip_level);

/*
 * 粉钻等级对应特权用餐、清洁、看病次数
 * --------------------------------------
 * |等级| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 * --------------------------------------
 * |次数| 0 | 1 | 2 | 3 | 3 | 4 | 4 | 5 |
 * --------------------------------------
 */
const unsigned int FeedTimes[LV_MAX] = {1, 1, 2, 3, 3, 4, 4, 5};
const unsigned int CleanTimes[LV_MAX] = {1, 1, 2, 3, 3, 4, 4, 5};
const unsigned int CureTimes[LV_MAX] = {1, 1, 2, 3, 3, 4, 4, 5};

}
}

// =====================================================================================
// 
//       Filename:  vip_ttc.h
// 
//    Description:  VIP TTC操作API
// 
//        Version:  1.0
//        Created:  2010年09月15日 10时08分42秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#pragma once

#include <vector>

namespace TencentTableCache{
	class Server;
}

namespace snslib{

class CVipTTC{
	public:
		// VIP原始数据
		// GetVipData时候提取的数据为DB原始数据
		// SetVipData时候growth、days_left_xxx字段为增/减差量，
		// TTC会执行Add/Sub操作以保证操作原子性，不要设置成绝对量!!!
		struct VipData{
			VipData() { memset(this, 0x0, sizeof(*this)); }

			int					mask;				// 更新掩码

			int					growth;				// 成长值
			int					days_left_normal;	// 普通成长
			int					days_left_medium;	// 中速成长
			int					days_left_fast;		// 快速成长
			int					days_left_rapid;	// 极速成长
			unsigned int		proc_timestamp;		// 上次处理时间
			unsigned int		check_and_set;		// 唯一标识
			char				non_prepaid_flag;	// 包月标记(0预付费,1手机,3宽带)
		};

		// 更新掩码
		enum{
			ADD_GROWTH = 1 << 0,
			ADD_DAYS_LEFT_NORMAL = 1 << 1,
			ADD_DAYS_LEFT_MEDIUM = 1 << 2,
			ADD_DAYS_LEFT_FAST = 1 << 3,
			ADD_DAYS_LEFT_RAPID = 1 << 4,
			SET_PROC_TIMESTAMP = 1 << 5,
			CHECK_AND_SET = 1 << 6,
			SET_NON_PREPAID_FLAG = 1 << 7,
		};

	public:
		// 构造函数
		CVipTTC();

		// 析构函数
		~CVipTTC();

		// 读取配置文件，初始化TTC实例
		int Initialize(const char *conf);

		// 释放TTC实例
		void Finalize();

		// 获取数据
		int GetVipData(unsigned int uin, VipData &vip_data);

		// 设置数据
		int SetVipData(unsigned int uin, const VipData &vip_data);

		// 得到错误描述
		const char* GetErrMsg();

	private:
		// 获取TTC实例
		TencentTableCache::Server* GetTTCServer(unsigned int uin);

		// 如果没有该用户数据，则创建一条该用户数据
		int TouchVipData(unsigned int uin);

	public:
		// 提供一些静态方法统一计算逻辑
		// 计算等级
		static unsigned short GetLevel(const VipData &vip_data);
		// 计算成长速度
		static unsigned short GetGrowthRate(const VipData &vip_data);
		// 计算年费标记
		static unsigned char GetYearFlag(const VipData &vip_data);
		// 计算粉钻标记
		static unsigned char GetVipFlag(const VipData &vip_data);
		// 计算剩余粉钻天数
		static unsigned short GetLeftDays(const VipData &vip_data);
		// 获取调整过后的数据
		static void Adjust(VipData &vip_data, const VipData &update_data);

	private:
		// TTC配置文件项
		struct TTC_CONF{
			// [40, 45) 注意区间开闭
			int uin_range_start;
			int uin_range_end;
			// xxx.xxx.xxx.xxx:xxxxx
			char addr[32];
			char table_name[128];
		};

	private:
		char m_err_msg[1024];
		std::vector<TTC_CONF> m_ttc_conf;
		std::vector<TencentTableCache::Server *> m_ttc_server;
};

}

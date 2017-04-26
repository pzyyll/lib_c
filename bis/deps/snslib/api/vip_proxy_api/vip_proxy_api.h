// =====================================================================================
// 
//       Filename:  vip_proxy_api.h
// 
//    Description:  粉钻信息API
// 
//        Version:  1.0
//        Created:  2010年09月08日 10时07分51秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#pragma once

namespace snslib{

class CSafeTcpClient;

class CVipProxyApi{
	public:
		struct VipData{
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

	public:
		/*
		 * 构造函数
		 */
		CVipProxyApi();

		/*
		 * 析构函数
		 */
		~CVipProxyApi();

		/*
		 * 初始化API
		 * param:	[IN] conf	配置文件
		 */
		int Init(const char *conf);

		/*
		 * 获取粉钻数据
		 * param:	[IN]	uin			用户UIN
		 * param:	[OUT]	vip_data	返回数据
		 * return:	返回错误码，详见上面的enum
		 */
		int GetVipData(unsigned int uin, VipData &vip_data);

		/*
		 * 获取最后一次错误信息
		 * return:	错误描述
		 */
		const char* GetErrMsg();

	private:
		/*
		 * 错误消息
		 */
		char m_err_msg[1024];

		/*
		 * 安全TCP客户端
		 */
		CSafeTcpClient *m_safe_tcp_client;
};

}// end of namespace snslib


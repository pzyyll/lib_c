// =====================================================================================
// 
//       Filename:  sns_def.h
// 
//    Description:  一些常量的定义
// 
//        Version:  1.0
//        Created:  2011年04月08日 15时16分33秒
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
// 
// =====================================================================================

#pragma once

namespace snslib
{

	/*
	 * 定义一些通用的字符串长度。在使用这些变量定义字符串的时候，都在这个长度基础上+1
	 * 统一使用如下方式，例如：
	 * char szIP[MAX_IP_LEN + 1];
	 * strncpy(szIP, szTemp, MAX_IP_LEN);
	 */
	const int MAX_IP_LEN            = 15;       // IP地址最大长度
	const int MAX_NAME_LEN          = 64;       // 模块名、配置段名、配置变量名
	const int MAX_FILE_PATH_LEN     = 1024;     // 路径或者带路径的文件名最大长度
	const int MAX_ERR_MSG_LEN       = 256;      // 一般错误信息字符串最大长度
	const int MAX_LINE_LEN          = 1024;     // 读取文件中一行的buff

}

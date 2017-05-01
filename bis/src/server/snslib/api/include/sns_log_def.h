// =====================================================================================
//
//       Filename:  sns_log_def.h
//
//    Description:  日志相关定义
//
//        Version:  1.0
//        Created:  08/10/2010 05:31:00 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  MichaelZhao (Zhao Guangyu), michaelzhao@tencent.com
//        Company:  TENCENT
//
// =====================================================================================

#pragma once

namespace snslib{

// 用匿名ENUM可以有效防止ID重复
//
// 业务使用如下号段规则
// APPID_000 - APPID_999

enum{

// PCL 1000-1999
LOG_PCL_STATISTICS			= 1001,		// 统计日志
LOG_PCL_CONN				= 1011,		// 新连接
LOG_PCL_TERMINATE			= 1012,		// 连接关闭
LOG_PCL_DATA_RECV			= 1021,		// 收到数据
LOG_PCL_DATA_SEND			= 1022,		// 发送数据
LOG_PCL_DATA_BUFFER_SEND	= 1023,		// 缓存发送数据
LOG_PCL_CMD_RECV			= 1031,		// 收到命令
LOG_PCL_CMD_SEND			= 1032,		// 发送命令
LOG_PCL_CMD_BUS_SEND		= 1041,		// BUS发送命令
LOG_PCL_CMD_BUS_RECV		= 1042,		// BUS接收命令

LOG_PAY_INFO                = 10001,    // ?§??
LOG_PAY_GOODS               = 10002,    // ???·????
LOG_PAY_PROVIDE             = 10003,    // ·￠??????

// LOCK
LOG_LOCK_GET				= 2000,		// 读取锁
LOG_LOCK_SET				= 2001, 	// 设置锁
LOG_LOCK_DEL				= 2002,		// 删除锁
LOG_LOCK_TIMEOUT			= 2010,		// 锁超时清除

// FLAG
LOG_FLAG_GET				= 2100,		// 获取标志位
LOG_FLAG_SET				= 2101,		// 设置标志位
LOG_FLAG_DEL				= 2102,		// 删除标志位

// KONGFU CACHE
LOG_KF_CACHE_GET            = 1031300,  // 加载数据
LOG_KF_CACHE_SET            = 1031301,  // 更新数据
LOG_KF_CACHE_NEW            = 1031302,  // 创建角色
LOG_KF_CACHE_DEL            = 1031303,  // 删除角色
LOG_KF_CACHE_DB             = 1031304,  // 从DB中加载

// KONGFU Gateway
LOG_KF_GW_GET_ADOPT_REQ     = 1031000,
LOG_KF_GW_GET_ADOPT_RSP     = 1031001,
LOG_KF_GW_GET_PET_REQ       = 1031002,
LOG_KF_GW_GET_PET_RSP       = 1031003,
LOG_KF_GW_UPDATE_PET_REQ    = 1031004,
LOG_KF_GW_UPDATE_PET_RSP    = 1031005,
LOG_KF_GW_MSG_CENTRE        = 1031006,

// KONGFU Dir
LOG_KF_DIR_REQ              = 1030900,
LOG_KF_DIR_RSP              = 1030901,
LOG_KF_DIR_OL               = 1030902,

// KONGFU App
LOG_KF_APP_LOGIN            = 1030000,
LOG_KF_APP_LOGOUT           = 1030002,
LOG_KF_APP_HELLO            = 1030003,
LOG_KF_APP_BUY              = 1030004,
LOG_KF_APP_EXCHANGE         = 1030005,
LOG_KF_APP_USE              = 1030006,
LOG_KF_APP_OL               = 1030007,
LOG_KF_APP_LEARN            = 1030008,
LOG_KF_APP_START_FIGHT      = 1030009,
LOG_KF_APP_END_FIGHT        = 1030010,
LOG_KF_APP_START_TASK       = 1030011,
LOG_KF_APP_END_TASK         = 1030012,
LOG_KF_APP_TASK_PRIZE       = 1030013,
LOG_KF_APP_FIGHT_ACTION     = 1030014,
LOG_KF_APP_PET_YUANBAO      = 1030015,
LOG_KF_APP_PET_GROWTH       = 1030016,

// LOADER WRITER
LOG_LW_LOAD_DATA			= 2200,		// 加载数据
LOG_LW_WRITE_DATA			= 2201, 	// 回写数据
LOG_LW_DEL_DATA				= 2202,		// 删除数据

};// end of enum

}// end of namespace petlib


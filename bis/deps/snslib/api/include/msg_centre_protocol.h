#ifndef _MSG_CENTRE_PROTOCOL_H_
#define _MSG_CENTRE_PROTOCOL_H_

#include <map>
#include <vector>

#include "comm/util/pet_util.h"

namespace snslib
{
const int MC_MSG_MAX_VALLEN = 64;
const int MC_MSG_MAGIC_NUM = 0x4D534743;    //MSGC

//用于接口间传递的消息
typedef struct tagMsgCentreMsg
{
    unsigned short ushMsgType;
    unsigned short ushMsgValLen;
    char szMsgVal[MC_MSG_MAX_VALLEN];   //该字段数据为主机序，如果是简单数据类型，可以通过指针指向头部直接使用
}MsgCentreMsg;

enum
{
    //宠物属性变化
    MCID_MODIFY_PETNAME			= 1,	//宠物昵称
    MCID_MODIFY_QQNAME			= 2,	//QQ昵称
    MCID_MODIFY_VIPFLAG			= 3,	//粉钻标志位
    MCID_MODIFY_PETLEVEL		= 4,	//宠物等级
    MCID_MODIFY_PERMISSION		= 5,	//宠物权限
    MCID_MODIFY_TITLEID			= 6,	//宠物当前称号
    MCID_MODIFY_PETSTATUS		= 7,	//宠物当前状态
    MCID_MODIFY_AVATARVER		= 8,	//宠物的Avatar版本
    MCID_MODIFY_PETHEALTH		= 9,	//宠物的健康级别
    MCID_MODIFY_AVATARINFO		= 10,	//宠物的Avatar装扮方案
    MCID_MODIFY_LOVINGNESS		= 11,	//宠物爱心值
    MCID_MODIFY_ZONEID			= 12,	//修改大区ID
    MCID_MODIFY_AREAID			= 13,	//修改小区ID
    MCID_MODIFY_YUANBAO			= 14,	//修改用户元宝
    MCID_MODIFY_GROWTH			= 15,	//修改用户成长值
    MCID_MODIFY_ONLINETIME		= 16,	//修改用户在线时间
	MCID_MODIFY_VIPLEVEL		= 17,	//VIP等级变化
	MCID_MODIFY_VIPYEARFLAG		= 18,	//VIP年费标记变化

    //宠物属性同步
    MCID_SYNC_PETNAME			= 501,  //宠物昵称
    MCID_SYNC_QQNAME			= 502,	//QQ昵称
    MCID_SYNC_VIPFLAG			= 503,	//粉钻标志位
    MCID_SYNC_PETLEVEL			= 504,	//宠物等级
    MCID_SYNC_PERMISSION		= 505,	//宠物权限
    MCID_SYNC_TITLEID			= 506,	//宠物当前称号
    MCID_SYNC_PETSTATUS			= 507,  //宠物当前状态
    MCID_SYNC_AVATARVER			= 508,	//宠物的Avatar版本
    MCID_SYNC_VIPLEVEL          = 509,  //VIP等级

    //需要发送到每台Router的加载、删除、更新消息
    MCID_PETLOADNODE			= 1001,	//宠物节点在SESSION中加载，注意，该消息必须使用PetHeader命令字0x7702
    MCID_PETDELNODE				= 1002,	//宠物节点在SESSION中删除，注意，该消息必须使用PetHeader命令字0x7702
    MCID_PETREGIST				= 1003,	//宠物注册
    MCID_PETDISCARD				= 1004,	//宠物抛弃
    MCID_PETLOGIN				= 1005,	//宠物登陆  LOGIN时需要将所有宠物属性发送出去
    MCID_PETLOGOUT				= 1006,	//宠物退出

    //任务系统的各种ID
    MCID_FISH_GET               = 2001, //养鱼收获鱼 VAL:[鱼的种类:4Byte][收鱼的条数:4Byte]
    MCID_FARM_GET               = 2002, //农场收作物 VAL:[作物的种类:4Byte][收作物的个数:4Byte]
    MCID_NPC_TALK               = 2003, //完成NPC对话 VAL:[SCeneID:2Byte][NPCID:2Byte][NPC对话次数]
    MCID_NPC_GAMESUCC           = 2004, //完成小游戏VAL:[GAMEID:4Byte][GAMELEVEL:4Byte]
};

};

#endif

/*
 * webapi_kongfu.h
 *
 *  Created on: 2010-9-16
 *      Author: jiffychen
 */

#ifndef WEBAPI_KONGFU_H_
#define WEBAPI_KONGFU_H_

#include "webapi_base.h"
#include "api/proto/replay.pb.h"
#include "api/proto/feeds.pb.h"

#include <vector>
#include<string>

namespace snslib
{
typedef struct tagKongfuBasicInfo
{
	// 宠物id
	unsigned long long ullPetID;
	// 宠物等级
	unsigned uiLevel;
	// 宠物成长值
	unsigned uiGrowth;
	// 宠物元宝
	unsigned uiYuanBao;
	// 悟性值
	unsigned uiSavvy;
	// 气
	unsigned uiCurMP;
	// 耐
	unsigned uiCurVIT;
	// 是否被封号，非0表示被封号，本字段表示封号原因
	unsigned uiFrozen;
	// 解封时间（从1970/1/1开始的秒数）
	unsigned uiFrozenTo;
	// 基础HP
	unsigned uiBaseHP;
	// 基础MP
	unsigned uiBaseMP;
	// 基础攻击力
	unsigned uiBaseATK;
	// 基础防御力
	unsigned uiBaseDEF;
	// 基础速度
	unsigned uiBaseATI;
	// 基础灵力
	unsigned uiBaseMAG;
	// 游戏角色创建时间
	unsigned uiCreateTime;
	// 上次登录游戏的时间
	unsigned uiLastLogin;
} KongfuBasicInfo;

typedef struct tagGoodsItem
{
    unsigned uiItemID;
    // 表示道具的增量，> 0 表示增加，< 0 表示减少
    int iItemCount;
} GoodsItem;

typedef struct tagSkillInfo
{
	// 技能ID
	unsigned uiSkillID;
	// 技能新等级，为0表示技能将被删除
	int iLevel;
} SKillInfo;

typedef struct tagKongfuAttr
{
	// 基础hp最大值增量
	int iBaseHP;
	// 基础mp最大值增量
	int iBaseMP;
	// 基础攻击力值增量
	int iBaseATK;
	// 基础防御力值增量
	int iBaseDEF;
	// 基础速度值增量
	int iBaseATI;
	// 基础灵力值增量
	int iBaseMAG;
	// 耐力值增量
	int iCurVIT;
	// 魔法值增量
	int iCurMP;
} KongfuAttr;

typedef struct tagTypedFightSkill
{
	std::vector<unsigned> vActiveSkills;
	std::vector<unsigned> vPassiveSkills;
} TypedFightSkill;

typedef struct tagFightSkill
{
	TypedFightSkill stAttack;
	TypedFightSkill stDefense;
} FightSkill;

typedef struct tagPlayerInfo
{
	KongfuBasicInfo stBasicInfo;
	std::vector<snslib::GoodsItem> vGoodsItems;
	std::vector<snslib::SKillInfo> vSkillInfos;
	FightSkill stFightSkills;
} PlayerInfo;

class CWebApiKongfu : public CWebApiBase
{
public:
	CWebApiKongfu();
	~CWebApiKongfu();

	// 添加或减少道具
	int ModifyGoods(unsigned uiUin, std::vector<snslib::GoodsItem>& vItems, const char* pszDesc);

	// 封号, 解封时间为从当前时间+iFrozeHours(iFrozenHours < 0 可以解封)
	int Froze(unsigned uiUin, unsigned uiFrozeType, int iFrozenHours);

	// 解封
	int UnFroze(unsigned uiUin);

	// 升级或降级技能
	int ModifySkills(unsigned uiUin, std::vector<snslib::SKillInfo>& vSkills);

	// 修改属性
	int ModifyAttr(unsigned uiUin, snslib::KongfuAttr& stAttr);

	// 元宝只能加，不能减，要减，直接减企鹅的
	int AddYuanBao(unsigned uiUin, unsigned uiYuanBaoAdd, const char* pszDesc);

	// 悟性值可以加或减，isavvyadd为负表示减悟性值
	int AddSavvy(unsigned uiUin, unsigned iSavvyAdd, const char* pszDesc);

	// 成长值只能加，不能减，要减，直接减企鹅的
	int AddGrowth(unsigned uiUin, unsigned uiGrowthAdd, const char* pszDesc);

	// 获取角色信息
	int GetUserInfo(unsigned uiUin, PlayerInfo& stPlayerInfo);

	//2011-06-08 新增录像接口  ghostzheng

	//获取录像列表
	int GetReplayList( unsigned int uiUin/*用户id*/ , kongfupet::GetReplayListResponse& stReplayList/*返回的录像列表*/);

	//拉取录像详细信息
	int GetReplay(unsigned int uiUin/*用户id*/,unsigned int request_id/*录像id*/, unsigned int fight_time /*开始打斗时间*/ , kongfupet::GetReplayResponse& szReplay/*返回的录像*/);

	//获取类型为type 的feeds列表
	int GetFeedsList( unsigned int uiUin/*用户id*/ ,  kongfupet::KongfuFeedsData::KongfuFeedType type,kongfupet::GetFeedsListResponse& stFeedsList/*返回的类型为type 的feed列表*/);

	//获取乐斗2 Feeds + 录像列表
	int GetWebFeeds(unsigned int uiUin/*用户id*/ ,kongfupet::WebFeeds& stWebFeeds/*返回的feeds*/);

};
}

#endif /* WEBAPI_KONGFU_H_ */

	


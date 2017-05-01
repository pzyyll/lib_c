/*
 * webapi_xboy.h
 *
 *  Created on: 2010-12-8
 *      Author: smiyang
 */

#ifndef WEBAPI_XBOY_H_
#define WEBAPI_XBOY_H_

#include <string>
#include <sstream>
#include "webapi_base.h"

namespace snslib
{
typedef struct tagMoneyUnit
{
    int iMoneyType;       //金币的型号
    int iMoneyCount;      //金币的数量
} MoneyUnit;

typedef struct tagBaseInfo
{
    unsigned int uiUin;   //QQ
    int exp;              //经验值
    int lev;              //等级
    int ryz;              //荣誉值
    int hzs;              //黑钻石
} BaseInfo;

typedef struct tagTeamInfo
{
    unsigned int uiUin;   //QQ
    std::string name;     //战队名称
    int super_teamer_size;//高级探员人数
} TeamInfo;

typedef struct tagGoodsUnit
{
    int iGoodsID;          //道具ID
    int iGoodsCount;       //表示道具的增量，> 0 表示增加，< 0 表示减少
} GoodsUnit;

typedef struct tagHonorTitleUnit
{
    int iHonorID;          //荣誉勋章的ID
    long lTime;            //获得该勋章的时间
} HonorTitleUnit;

typedef struct tagFightUnit
{
    unsigned int iPlayerID;
    int iWonCount;
    int iLostCount;
    long lLastTime;
} FightUnit;

typedef struct tagRandomUnit
{
    unsigned int iPlayerID;
    int iPlayerLev;
} RandomUnit;

typedef struct tagSimpleInfoUnit
{
    unsigned int iPlayerID;
    int iLev;
    int iSize;
} SimpleInfo;

typedef struct tagBaseAttrUnit
{
    int iCurLife;
    int iCurEnergy;
    int iCurEndurance;
    int iCurRyz;
    int iCurExp;
    int iCurHzs;
} BaseAttrUnit;

typedef struct tagTaskUnit
{
    int task_id;
    int cur_step;
    int cur_state;
    int series_task_id;
    int series_task_level;
} TaskUnit;

typedef struct tagSeriesTaskUnit
{
    int series_task_id;
    int series_task_level;
} SeriesTaskUnit;

typedef struct tagDzdUnit
{
    unsigned int iPlayerID;
    long lCreateTime;
} DzdUnit;

typedef struct tagBossUnit
{
    int boss_id;
    int win_count;
    int lost_count;
    int last_won_time;
    int fighting;
    int cur_life;
    int last_ft_time;
} BossUnit;

class CWebApiXBoy : public CWebApiBase
{
public:
    CWebApiXBoy();
    ~CWebApiXBoy();

    /********************************业务受理要求的API接口********************************/
    //封号, 解封时间为从当前时间+iFrozeHours(iFrozenHours必须为正整数)
    //uiFrozeType为封号原因，为正整数
    int XBoyFroze(const unsigned uiUin, const unsigned uiFrozeType, const int iFrozenHours);

    //解封
    int XBoyUnFroze(const unsigned uiUin);

    //打印玩家的钱包
    int XBoyPrintPlayerMoneyBag(unsigned uiUin, std::vector<snslib::MoneyUnit>& vItems);

    //增量更新玩家的钱包
    int XBoyModifyPlayerMoneyBag(unsigned uiUin, std::vector<snslib::MoneyUnit>& vItems);

    //打印玩家的基本信息：QQ、经验值、等级、荣誉值、黑钻石
    int XBoyPrintPlayerBaseInfo(unsigned uiUin, snslib::BaseInfo &stBaseInfo);

    //修改玩家的基本信息：QQ、经验值(后台会根据经验自动更新玩家等级，等级置0)、荣誉值、黑钻石
    int XBoyModifyPlayerBaseInfo(unsigned uiUin, snslib::BaseInfo &stBaseInfo);

    //打印玩家的战队信息：战队名称、高级探员人数
    int XBoyPrintPlayerTeamInfo(unsigned uiUin, snslib::TeamInfo &stTeamInfo);

    //打印玩家的道具
    int XBoyPrintPlayerGoods(unsigned uiUin, std::vector<snslib::GoodsUnit>& vItems);

    //添加或减少道具
    int XBoyModifyPlayerGoods(unsigned uiUin, std::vector<snslib::GoodsUnit>& vItems);
    /*****************************业务受理要求的API接口**********************************/



    /**********************************测试使用的一些接口API*****************************/
    //玩家注册
    int XBoyRegister(unsigned uiUin);

    //打印玩家的基础属性：生命、能量、耐力值、荣誉值
    int XBoyPrintPlayerBaseAttr(unsigned uiUin, BaseAttrUnit& stBaseAttrUnit);

    //增量更改玩家的基础属性：生命、能量、耐力值、荣誉值,将不想更改的项置为0
    int XBoyModifyPlayerBaseAttr(unsigned uiUin, BaseAttrUnit& stBaseAttrUnit);

    //打印玩家的今日挑战列表
    int XBoyPrintTodayFightList(unsigned uiUin, std::vector<snslib::FightUnit>& vItems);

    //更改玩家的今日挑战列表-采用单元复写,time字段-1表示删除该元素,0表示采用原来时间，其它采用输入的时间
    int XBoyModifyTodayFightList(unsigned uiUin, std::vector<snslib::FightUnit>& vItems);

    //打印玩家的历史挑战列表
    int XBoyPrintHistoryBeFightList(unsigned uiUin, std::vector<snslib::FightUnit>& vItems);

    //更改玩家的历史挑战列表-采用单元复写,time字段-1表示删除该元素,0表示采用原来时间，其它采用输入的时间
    int XBoyModifyHistoryBeFightList(unsigned uiUin, std::vector<snslib::FightUnit>& vItems);

    //打印玩家的任务列表
    int XBoyPrintPlayerTaskList(unsigned uiUin, std::vector<snslib::TaskUnit>& vItems);

    //修改玩家的任务列表：step字段为-1表示要清楚任务列表中的该任务。若任务列表没有该任务，那么会增加该任务
    int XBoyModifyPlayerTaskList(unsigned uiUin, std::vector<snslib::TaskUnit>& vItems);

    //打印玩家的系列任务列表
    int XBoyPrintPlayerSeriesTaskList(unsigned uiUin, std::vector<snslib::SeriesTaskUnit>& vItems);

    //修改玩家的系列任务列表-level字段为-1代表删除操作。若系列任务列表中没有该任务，那么会增加该系列任务
    int XBoyModifyPlayerSeriesTaskList(unsigned uiUin, std::vector<snslib::SeriesTaskUnit>& vItems);

    //打印间谍个数
    int XBoyPrintPlayerDzdCount(unsigned uiUin, unsigned& uiCount);

    //间谍个数修改
    int XBoyModifyPlayerDzdCount(unsigned uiUin, unsigned uiCount);

    //打印间谍信息
    int XBoyPrintPlayerDzdInfo(unsigned uiUin, std::vector<snslib::DzdUnit>& vItems);

    //更改间谍信息-时间字段为-1标识删除该间谍。其它为复写（存在该id的间谍）或增加间谍，时间字段为0表示采用系统时间。
    int XBoyModifyPlayerDzdInfo(unsigned uiUin, std::vector<snslib::DzdUnit>& vItems);

    //打印BOSS背包
    int XBoyPrintPlayerBossBag(unsigned uiUin, std::vector<snslib::BossUnit>& vItems);

    //修改BOSS信息-时间字段为-1标识删除该BOSS。其它为复写（存在该id的BOSS）或增加BOSS，时间字段为0表示采用系统时间。
    int XBoyModifyPlayerBossBag(unsigned uiUin, std::vector<snslib::BossUnit>& vItems);

    //更新（第一次更新为添加）RandomSvr中的一条数据
    int XBoyRefreshRandomSvrPlayerInfo(unsigned uiUin, RandomUnit& Item);

    //为一个玩家随机出挑战对象
    int XBoyRandomFightPlayer(unsigned uiUin, RandomUnit& Item, std::vector<unsigned int>& vBlackList,
            std::vector<unsigned int>& vRandomList);

    //获取玩家的简单信息
    int XBoyPrintSimpleInfo(unsigned uiUin, snslib::SimpleInfo& stItems);

    //修改玩家的简单信息
    int XBoyModifySimpleInfo(unsigned uiUin, snslib::SimpleInfo& stItems);

    //获取玩家的挑战次数：胜利、失败场次
    int XBoyPrintFightCount(unsigned uiUin, int &iWonCount, int &iLostCount);

    //修改玩家的挑战次数：胜利、失败场次
    int XBoyModifyFightCount(unsigned uiUin, int iWonCount, int iLostCount);

    //打印玩家获得的勋章列表
    int XBoyPrintHonorTitle(unsigned uiUin, std::vector<HonorTitleUnit> &vHonorTitle);

    //修改玩家的勋章。time=-1表示删除该勋章，time=0标识增加勋章并将获得的时间置为系统时间，time=other标识采用输入作为获得时间
    int XBoyModifyHonorTitle(unsigned uiUin, int iHonorID, long lTime);

    //将玩家置0
    int XBoyResetPlayerInfo(unsigned int uiUin);
};
}
#endif /* WEBAPI_XBOY_H_ */

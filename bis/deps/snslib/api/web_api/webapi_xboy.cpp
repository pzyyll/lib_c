/*
 * webapi_xboy.cpp
 *
 * Created on: 11010-111-13
 * Author: smiyang
 */

#include "webapi_xboy.h"
#include "../../../server/xboy/proto/xboy_player.pb.h"
#include "../../../server/xboy/proto/xboy_random.pb.h"
#include "../../../server/xboy/proto/xboy_simpleinfo.pb.h"
#include "../../../server/xboy/proto/json.pb.h"

using namespace snslib;
using namespace std;

const unsigned short CMD_XBOY_HOME_UPDATE_PLAYER = 0x5100;
const unsigned short CMD_XBOY_GET_PLAYER_INFO = 0x5101;
const unsigned short CMD_XBOY_FIGHT_RANDOM_GET_LIST = 0x5106;
const unsigned short CMD_XBOY_FIGHT_RANDOM_REFRESH = 0x5107;
const unsigned short CMD_XBOY_GET_SIMPLE_PLAYER_INFO = 0x5104;
const unsigned short CMD_XBOY_SAVE_SIMPLE_PLAYER_INFO = 0x5105;
const unsigned short CMD_XBOY_CGI_PLAYER_REGISTER = 8100;
const unsigned short CMD_XBOY_CLIENT_USE_GOODS    = 8305;
const unsigned short CMD_XBOY_RESET_PLAYER_INFO = 0x5202;

CWebApiXBoy::CWebApiXBoy()
{

}

CWebApiXBoy::~CWebApiXBoy()
{

}

/********************************业务受理要求的API接口*******beg*********************/
int CWebApiXBoy::XBoyFroze(const unsigned uiUin, const unsigned uiFrozeType, const int iFrozenHours)
{
    XboyPlayerDataUpdate objRequest;
    objRequest.set_uin(uiUin);
    objRequest.mutable_player_data()->set_player_id(uiUin);
    objRequest.mutable_player_data()->mutable_froze_state()->set_froze_reason(uiFrozeType);
    objRequest.mutable_player_data()->mutable_froze_state()->set_unfroze_time(iFrozenHours*60*60);

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }

    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyUnFroze(const unsigned uiUin)
{
    XboyPlayerDataUpdate objRequest;
    objRequest.set_uin(uiUin);
    objRequest.mutable_player_data()->set_player_id(uiUin);
    objRequest.mutable_player_data()->mutable_froze_state()->set_froze_reason(0);

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }

    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintPlayerMoneyBag(unsigned uiUin, std::vector<snslib::MoneyUnit>& vItems)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    xboy_player stxboy_player = objResponse.player_data();
    int iCount = objResponse.player_data().xboy_money_size();
    for (int i = 0; i < iCount; i++)
    {
        MoneyUnit stMoneyUnit;
        stMoneyUnit.iMoneyType = objResponse.player_data().xboy_money(i).money_type();
        stMoneyUnit.iMoneyCount = objResponse.player_data().xboy_money(i).cash_money();
        vItems.push_back(stMoneyUnit);
    }

    return 0;
}

int CWebApiXBoy::XBoyModifyPlayerMoneyBag(unsigned uiUin, std::vector<snslib::MoneyUnit>& vItems)
{
    XboyPlayerDataUpdate objRequest;
    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    objRequest.set_uin(uiUin);
    pstxboy_player->set_player_id(uiUin);

    for (size_t i = 0; i < vItems.size(); i++)
    {
        XboyMoney *pstXboyMoney = pstxboy_player->add_xboy_money();
        pstXboyMoney->set_money_type(vItems[i].iMoneyType);
        pstXboyMoney->set_cash_money(vItems[i].iMoneyCount);
    }

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -3;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintPlayerBaseInfo(unsigned uiUin, snslib::BaseInfo &stBaseInfo)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    stBaseInfo.uiUin = objResponse.player_data().player_id();
    stBaseInfo.lev = objResponse.player_data().player_level();
    stBaseInfo.exp = objResponse.player_data().cur_experience();
    stBaseInfo.ryz = objResponse.player_data().honor_value();
    stBaseInfo.hzs = objResponse.player_data().hz_count();

    return 0;
}

int CWebApiXBoy::XBoyModifyPlayerBaseInfo(unsigned uiUin, snslib::BaseInfo &stBaseInfo)
{
    XboyPlayerDataUpdate objRequest;

    objRequest.set_uin(uiUin);
    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    pstxboy_player->set_player_id(uiUin);

    stBaseInfo.lev = 0;
    if (0 != stBaseInfo.exp)
    {
        pstxboy_player->set_cur_experience(stBaseInfo.exp);
    }

    if (0 != stBaseInfo.ryz)
    {
        pstxboy_player->set_honor_value(stBaseInfo.ryz);
    }

    if (0 != stBaseInfo.hzs)
    {
        pstxboy_player->set_hz_count(stBaseInfo.hzs);
    }

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintPlayerTeamInfo(unsigned uiUin, snslib::TeamInfo &stTeamInfo)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    stTeamInfo.uiUin = objResponse.uin();
    stTeamInfo.name = objResponse.player_data().team_name();

    int iSuperTeamerSize = 0;
    for (int i = 0; i < objResponse.player_data().dzd_list_size(); ++i)
    {
        if (objResponse.player_data().dzd_list(i).create_time() + 7*24*3600 > time(0))
        {
            ++iSuperTeamerSize;
        }
    }
    stTeamInfo.super_teamer_size = iSuperTeamerSize;

    return 0;
}

int CWebApiXBoy::XBoyPrintPlayerGoods(unsigned uiUin, std::vector<snslib::GoodsUnit>& vItems)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    xboy_player stxboy_player = objResponse.player_data();
    int iCount = objResponse.player_data().goods_bag_size();
    for (int i = 0; i < iCount; i++)
    {
        GoodsUnit stGoodsUnit;
        stGoodsUnit.iGoodsID = objResponse.player_data().goods_bag(i).goods_id();
        stGoodsUnit.iGoodsCount = objResponse.player_data().goods_bag(i).goods_count();
        vItems.push_back(stGoodsUnit);
    }

    return 0;
}

int CWebApiXBoy::XBoyModifyPlayerGoods(unsigned uiUin, std::vector<snslib::GoodsUnit>& vItems)
{
    XboyPlayerDataUpdate objRequest;
    printf("uin=%u\n", uiUin);
    objRequest.set_uin(uiUin);
    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    pstxboy_player->set_player_id(uiUin);

    for (size_t i = 0; i < vItems.size(); i++)
    {
        GoodsAndCount *pstGoodsAndCount = pstxboy_player->add_goods_bag();;
        pstGoodsAndCount->set_goods_id(vItems[i].iGoodsID);
        pstGoodsAndCount->set_goods_count(vItems[i].iGoodsCount);
    }

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}
/********************************业务受理要求的API接口********end*********************/

int CWebApiXBoy::XBoyRegister(unsigned uiUin)
{
    XboyPlayerDataGetRequest objRequest;//占位，没有什么用。
    RegisterInfo objResponse;

    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_CGI_PLAYER_REGISTER, 10, 0, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.head().error())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintPlayerBaseAttr(unsigned uiUin, BaseAttrUnit& stBaseAttrUnit)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    stBaseAttrUnit.iCurLife = objResponse.player_data().cur_life();
    stBaseAttrUnit.iCurEnergy = objResponse.player_data().cur_energy();
    stBaseAttrUnit.iCurEndurance = objResponse.player_data().cur_endurance();
    stBaseAttrUnit.iCurRyz = objResponse.player_data().honor_value();
    stBaseAttrUnit.iCurExp = objResponse.player_data().cur_experience();
    stBaseAttrUnit.iCurHzs = objResponse.player_data().hz_count();

    return 0;
}

int CWebApiXBoy::XBoyModifyPlayerBaseAttr(unsigned uiUin, BaseAttrUnit& stBaseAttrUnit)
{
    XboyPlayerDataUpdate objRequest;

    objRequest.set_uin(uiUin);
    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    pstxboy_player->set_player_id(uiUin);

    if (0 != stBaseAttrUnit.iCurLife)
    {
        pstxboy_player->set_cur_life(stBaseAttrUnit.iCurLife);
    }

    if (0 != stBaseAttrUnit.iCurEnergy)
    {
        pstxboy_player->set_cur_energy(stBaseAttrUnit.iCurEnergy);
    }

    if (0 != stBaseAttrUnit.iCurEndurance)
    {
        pstxboy_player->set_cur_endurance(stBaseAttrUnit.iCurEndurance);
    }

    if (0 != stBaseAttrUnit.iCurRyz)
    {
        pstxboy_player->set_honor_value(stBaseAttrUnit.iCurRyz);
    }

    if (0 != stBaseAttrUnit.iCurExp)
    {
        pstxboy_player->set_cur_experience(stBaseAttrUnit.iCurExp);
    }

    if (0 != stBaseAttrUnit.iCurHzs)
    {
        pstxboy_player->set_hz_count(stBaseAttrUnit.iCurHzs);
    }

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintTodayFightList(unsigned uiUin, std::vector<snslib::FightUnit>& vItems)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    xboy_player stxboy_player = objResponse.player_data();
    int iCount = objResponse.player_data().today_challenge_other_size();
    for (int i = 0; i < iCount; i++)
    {
        FightUnit stFightUnit;
        stFightUnit.iPlayerID = objResponse.player_data().today_challenge_other(i).player_id();
        stFightUnit.iWonCount = objResponse.player_data().today_challenge_other(i).win_count();
        stFightUnit.iLostCount = objResponse.player_data().today_challenge_other(i).lost_count();
        stFightUnit.lLastTime = objResponse.player_data().today_challenge_other(i).last_time();
        vItems.push_back(stFightUnit);
    }

    return 0;
}

int CWebApiXBoy::XBoyModifyTodayFightList(unsigned uiUin, std::vector<snslib::FightUnit>& vItems)
{
    XboyPlayerDataUpdate objRequest;

    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    objRequest.set_uin(uiUin);
    pstxboy_player->set_player_id(uiUin);

    for (size_t i = 0; i < vItems.size(); i++)
    {
        TeamAndCount *pstTeamAndCount = pstxboy_player->add_today_challenge_other();
        pstTeamAndCount->set_player_id(vItems[i].iPlayerID);
        pstTeamAndCount->set_win_count(vItems[i].iWonCount);
        pstTeamAndCount->set_lost_count(vItems[i].iLostCount);
        pstTeamAndCount->set_last_time(vItems[i].lLastTime);
    }

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintHistoryBeFightList(unsigned uiUin, std::vector<snslib::FightUnit>& vItems)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    xboy_player stxboy_player = objResponse.player_data();
    int iCount = objResponse.player_data().history_be_challenged_size();
    for (int i = 0; i < iCount; i++)
    {
        FightUnit stFightUnit;
        stFightUnit.iPlayerID = objResponse.player_data().history_be_challenged(i).player_id();
        stFightUnit.iWonCount = objResponse.player_data().history_be_challenged(i).win_count();
        stFightUnit.iLostCount = objResponse.player_data().history_be_challenged(i).lost_count();
        stFightUnit.lLastTime = objResponse.player_data().history_be_challenged(i).last_time();
        vItems.push_back(stFightUnit);
    }

    return 0;
}

int CWebApiXBoy::XBoyModifyHistoryBeFightList(unsigned uiUin, std::vector<snslib::FightUnit>& vItems)
{
    XboyPlayerDataUpdate objRequest;

    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    objRequest.set_uin(uiUin);
    pstxboy_player->set_player_id(uiUin);

    for (size_t i = 0; i < vItems.size(); i++)
    {
        TeamAndCount *pstTeamAndCount = pstxboy_player->add_history_be_challenged();
        pstTeamAndCount->set_player_id(vItems[i].iPlayerID);
        pstTeamAndCount->set_win_count(vItems[i].iWonCount);
        pstTeamAndCount->set_lost_count(vItems[i].iLostCount);
        pstTeamAndCount->set_last_time(vItems[i].lLastTime);
    }

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintPlayerTaskList(unsigned uiUin, std::vector<snslib::TaskUnit>& vItems)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    xboy_player stxboy_player = objResponse.player_data();
    int iCount = objResponse.player_data().task_bag_size();
    for (int i = 0; i < iCount; i++)
    {
        TaskUnit stTaskUnit;
        stTaskUnit.series_task_id= objResponse.player_data().task_bag(i).series_task_id();
        stTaskUnit.task_id = objResponse.player_data().task_bag(i).task_id();
        stTaskUnit.cur_state = objResponse.player_data().task_bag(i).cur_state();
        stTaskUnit.cur_step = objResponse.player_data().task_bag(i).cur_step();
        stTaskUnit.series_task_level = objResponse.player_data().task_bag(i).series_task_level();
        vItems.push_back(stTaskUnit);
    }

    return 0;
}

int CWebApiXBoy::XBoyModifyPlayerTaskList(unsigned uiUin, std::vector<snslib::TaskUnit>& vItems)
{
    XboyPlayerDataUpdate objRequest;
    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    objRequest.set_uin(uiUin);
    pstxboy_player->set_player_id(uiUin);

    for (size_t i = 0; i < vItems.size(); i++)
    {
        TaskAndCurStep *pstTaskAndCurStep = pstxboy_player->add_task_bag();
        pstTaskAndCurStep->set_task_id(vItems[i].task_id);
        pstTaskAndCurStep->set_cur_step(vItems[i].cur_step);
        pstTaskAndCurStep->set_cur_state(vItems[i].cur_state);
        pstTaskAndCurStep->set_series_task_id(vItems[i].series_task_id);
        pstTaskAndCurStep->set_series_task_level(vItems[i].series_task_level);
    }

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintPlayerSeriesTaskList(unsigned uiUin, std::vector<snslib::SeriesTaskUnit>& vItems)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    xboy_player stxboy_player = objResponse.player_data();
    int iCount = objResponse.player_data().series_task_bag_size();
    for (int i = 0; i < iCount; i++)
    {
        SeriesTaskUnit stSeriesTaskUnit;
        stSeriesTaskUnit.series_task_id= objResponse.player_data().series_task_bag(i).series_task_id();
        stSeriesTaskUnit.series_task_level = objResponse.player_data().series_task_bag(i).series_task_level();
        vItems.push_back(stSeriesTaskUnit);
    }

    return 0;
}

int CWebApiXBoy::XBoyModifyPlayerSeriesTaskList(unsigned uiUin, std::vector<snslib::SeriesTaskUnit>& vItems)
{
    XboyPlayerDataUpdate objRequest;
    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    objRequest.set_uin(uiUin);
    pstxboy_player->set_player_id(uiUin);

    for (size_t i = 0; i < vItems.size(); i++)
    {
        SeriesTaskState *pstSeriesTaskState = pstxboy_player->add_series_task_bag();
        pstSeriesTaskState->set_series_task_id(vItems[i].series_task_id);
        pstSeriesTaskState->set_series_task_level(vItems[i].series_task_level);
    }

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintPlayerDzdCount(unsigned uiUin, unsigned& uiCount)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    xboy_player stxboy_player = objResponse.player_data();
    uiCount = objResponse.player_data().dzd_count();

    return 0;
}

int CWebApiXBoy::XBoyModifyPlayerDzdCount(unsigned uiUin, unsigned uiCount)
{
    XboyPlayerDataUpdate objRequest;
    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    objRequest.set_uin(uiUin);
    pstxboy_player->set_player_id(uiUin);
    pstxboy_player->set_dzd_count(uiCount);

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintPlayerDzdInfo(unsigned uiUin, std::vector<snslib::DzdUnit>& vItems)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    xboy_player stxboy_player = objResponse.player_data();
    int iDzdCount = stxboy_player.dzd_list_size();

    for (int i = 0; i < iDzdCount; ++i)
    {
        DzdUnit stDzdUnit;
        stDzdUnit.iPlayerID = stxboy_player.dzd_list(i).player_id();
        stDzdUnit.lCreateTime = stxboy_player.dzd_list(i).create_time();
        vItems.push_back(stDzdUnit);
    }

    return 0;
}

int CWebApiXBoy::XBoyModifyPlayerDzdInfo(unsigned uiUin, std::vector<snslib::DzdUnit>& vItems)
{
    XboyPlayerDataUpdate objRequest;
    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    objRequest.set_uin(uiUin);
    pstxboy_player->set_player_id(uiUin);

    for (size_t i = 0; i < vItems.size(); ++i)
    {
        DzdInfo *pstDzdInfo = pstxboy_player->add_dzd_list();
        pstDzdInfo->set_player_id(vItems[i].iPlayerID);
        pstDzdInfo->set_create_time(vItems[i].lCreateTime);
    }

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintPlayerBossBag(unsigned uiUin, std::vector<snslib::BossUnit>& vItems)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    xboy_player stxboy_player = objResponse.player_data();
    int iBossCount = stxboy_player.boss_bag_size();

    for (int i = 0; i < iBossCount; ++i)
    {
        BossUnit stBossUnit;
        stBossUnit.boss_id = stxboy_player.boss_bag(i).boss_id();
        stBossUnit.win_count = stxboy_player.boss_bag(i).win_count();
        stBossUnit.lost_count = stxboy_player.boss_bag(i).lost_count();
        stBossUnit.last_ft_time = stxboy_player.boss_bag(i).last_won_time();
        stBossUnit.fighting = stxboy_player.boss_bag(i).fighting();
        stBossUnit.cur_life = stxboy_player.boss_bag(i).cur_life();
        stBossUnit.last_ft_time = stxboy_player.boss_bag(i).last_ft_time();
        vItems.push_back(stBossUnit);
    }

    return 0;
}

int CWebApiXBoy::XBoyModifyPlayerBossBag(unsigned uiUin, std::vector<snslib::BossUnit>& vItems)
{
    XboyPlayerDataUpdate objRequest;
    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    objRequest.set_uin(uiUin);
    pstxboy_player->set_player_id(uiUin);

    for (size_t i = 0; i < vItems.size(); ++i)
    {
        BossAndStep *pstBossAndStep = pstxboy_player->add_boss_bag();
        pstBossAndStep->set_boss_id(vItems[i].boss_id);
        pstBossAndStep->set_win_count(vItems[i].win_count);
        pstBossAndStep->set_lost_count(vItems[i].lost_count);
        pstBossAndStep->set_last_won_time(vItems[i].last_won_time);
        pstBossAndStep->set_fighting(vItems[i].fighting);
        pstBossAndStep->set_cur_life(vItems[i].cur_life);
        pstBossAndStep->set_last_ft_time(vItems[i].last_ft_time);
    }

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyRefreshRandomSvrPlayerInfo(unsigned uiUin, RandomUnit& Item)
{
    UpdatePlayerLevelRequest objRequest;

    if (Item.iPlayerLev == 0)
    {
        //等级输入0认为无效，不发包到randomsvr
        return 0;
    }
    else
    {
        objRequest.set_uin(Item.iPlayerID);
        objRequest.set_level(Item.iPlayerLev);
    }

    //发送消息
    int iRetVal;
    iRetVal = SendAndRecv(uiUin, CMD_XBOY_FIGHT_RANDOM_REFRESH, 10, 13, objRequest);

    if (iRetVal != 0)
    {
        return iRetVal;
    }

    return 0;
}

int CWebApiXBoy::XBoyRandomFightPlayer(unsigned uiUin, RandomUnit& Item, std::vector<unsigned int>& vBlackList,
        std::vector<unsigned int>& vRandomList)
{
    FetchRandOpponentRequest objRequest;
    FetchRandOpponentResponse objResponse;

    objRequest.set_uin(Item.iPlayerID);
    objRequest.set_level(Item.iPlayerLev);
    for (size_t i = 0; i < vBlackList.size(); ++i)
    {
        objRequest.add_exclude(vBlackList[i]);
    }

    //发送并接收返回信息
    int iRetVal;
    iRetVal = SendAndRecv(uiUin, CMD_XBOY_FIGHT_RANDOM_GET_LIST, 10, 13, objRequest, objResponse);

    if (iRetVal != 0)
    {
        return iRetVal;
    }

    for (int i = 0; i < objResponse.uins_size(); ++i)
    {
        vRandomList.push_back(objResponse.uins(i));
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintSimpleInfo(unsigned uiUin, snslib::SimpleInfo& stItems)
{
    GetPlayerSimpleInfo objRequest;
    PlayerSimpleInfo *pstPlayerSimpleInfo = objRequest.add_playerinfo();
    pstPlayerSimpleInfo->set_uin(uiUin);

    GetPlayerSimpleInfo objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_SIMPLE_PLAYER_INFO, 10, 12, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }

    if (1 != objResponse.playerinfo_size())
    {
        return -2;
    }

    if (0 != objResponse.playerinfo(0).ret())
    {
        return -3;
    }

    stItems.iPlayerID = objResponse.playerinfo(0).uin();
    stItems.iLev = objResponse.playerinfo(0).lev();
    stItems.iSize = objResponse.playerinfo(0).size();

    return 0;
}

int CWebApiXBoy::XBoyModifySimpleInfo(unsigned uiUin, snslib::SimpleInfo& stItems)
{
    SavePlayerSimpleInfo objRequest;
    PlayerSimpleInfo *pstPlayerSimpleInfo = objRequest.mutable_playerinfo();

    pstPlayerSimpleInfo->set_uin(stItems.iPlayerID);
    pstPlayerSimpleInfo->set_lev(stItems.iLev);
    pstPlayerSimpleInfo->set_size(stItems.iSize);

    SavePlayerSimpleInfo objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_SAVE_SIMPLE_PLAYER_INFO, 10, 12, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }

    pstPlayerSimpleInfo = objResponse.mutable_playerinfo();
    if (0 != pstPlayerSimpleInfo->ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintFightCount(unsigned uiUin, int &iWonCount, int &iLostCount)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    xboy_player stxboy_player = objResponse.player_data();
    iWonCount = stxboy_player.won_count();
    iLostCount = stxboy_player.lose_count();

    return 0;
}

int CWebApiXBoy::XBoyModifyFightCount(unsigned uiUin, int iWonCount, int iLostCount)
{
    XboyPlayerDataUpdate objRequest;
    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    objRequest.set_uin(uiUin);
    pstxboy_player->set_player_id(uiUin);

    if (0 != iWonCount)
    {
        pstxboy_player->set_won_count(iWonCount);
    }

    if (0 != iLostCount)
    {
        pstxboy_player->set_lose_count(iLostCount);
    }

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyPrintHonorTitle(unsigned uiUin, std::vector<HonorTitleUnit> &vHonorTitle)
{
    XboyPlayerDataGetRequest objRequest;
    objRequest.set_uin(uiUin);

    XboyPlayerDataGetResponse objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_GET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    xboy_player stxboy_player = objResponse.player_data();
    int iHonorBagSize = stxboy_player.honor_title_id_size();
    for (int i = 0; i < iHonorBagSize; ++i)
    {
        HonorTitleUnit stHonorTitleUnit;
        stHonorTitleUnit.iHonorID = stxboy_player.honor_title_id(i).title_id();
        stHonorTitleUnit.lTime = stxboy_player.honor_title_id(i).gain_time();
        vHonorTitle.push_back(stHonorTitleUnit);
    }

    return 0;
}

int CWebApiXBoy::XBoyModifyHonorTitle(unsigned uiUin, int iHonorID, long lTime)
{
    XboyPlayerDataUpdate objRequest;
    xboy_player *pstxboy_player = objRequest.mutable_player_data();
    objRequest.set_uin(uiUin);
    pstxboy_player->set_player_id(uiUin);

    HonorTitle *pstHonorTitle = pstxboy_player->add_honor_title_id();
    pstHonorTitle->set_title_id(iHonorID);
    pstHonorTitle->set_gain_time(lTime);

    XboyPlayerDataUpdate objResponse;
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_HOME_UPDATE_PLAYER, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

int CWebApiXBoy::XBoyResetPlayerInfo(unsigned int uiUin)
{
    XboyPlayerDataSaveRequest objRequest;
    XboyPlayerDataSaveResponse objResponse;

    objRequest.set_uin(uiUin);
    int iRetVal = SendAndRecv(uiUin, CMD_XBOY_RESET_PLAYER_INFO, 10, 11, objRequest, objResponse);
    if (iRetVal != 0)
    {
        return -1;
    }
    if (0 != objResponse.ret())
    {
        return -2;
    }

    return 0;
}

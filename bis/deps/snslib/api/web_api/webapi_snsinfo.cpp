#include <string.h>

#include "webapi_snsinfo.h"
#include "api/proto/sns_info.pb.h"

using namespace snslib;
using namespace std;
using namespace kongfupet;

const int WEBAPI_SNSINFO_MAX_UIN_NUM_PER_REQ = 100;

CWebApiSnsInfo::CWebApiSnsInfo()
{}

CWebApiSnsInfo::~CWebApiSnsInfo()
{}

int CWebApiSnsInfo::GetSnsInfo(std::vector<unsigned int> vuiUin, std::map<unsigned int, SnsInfo> &mSnsInfo)
{
    int iRetVal = 0;

    GetSnsInfoRequest objGetSnsInfoRequest;
    GetSnsInfoResponse objGetSnsInfoResponse;

    mSnsInfo.clear();

    for(unsigned int i=0; i<vuiUin.size(); i++)
    {
        objGetSnsInfoRequest.add_uin(vuiUin[i]);
        if (objGetSnsInfoRequest.uin_size() >= WEBAPI_SNSINFO_MAX_UIN_NUM_PER_REQ)
        {
            iRetVal = SendAndRecv(objGetSnsInfoRequest.uin(0), CMD_SNSINFO_GET, 103, 1, objGetSnsInfoRequest, objGetSnsInfoResponse);
            if (iRetVal != 0)
            {
                return iRetVal;
            }

            SnsInfo stSnsInfo;
            for(int j=0; j<objGetSnsInfoResponse.sns_info_size(); j++)
            {
                stSnsInfo.uiUin = objGetSnsInfoResponse.sns_info(j).uin();
                stSnsInfo.ullPetID = objGetSnsInfoResponse.sns_info(j).petid();
                stSnsInfo.uiYuanBao = objGetSnsInfoResponse.sns_info(j).yuanbao();
                stSnsInfo.uiGrowth = objGetSnsInfoResponse.sns_info(j).growth();
                stSnsInfo.uiOnlineTime = objGetSnsInfoResponse.sns_info(j).online_time();
                stSnsInfo.bOnlineFlag = objGetSnsInfoResponse.sns_info(j).online_flg();

                mSnsInfo.insert(pair<unsigned int, SnsInfo>(stSnsInfo.uiUin, stSnsInfo));
            }

            objGetSnsInfoRequest.clear_uin();
        }
    }

    if (objGetSnsInfoRequest.uin_size() > 0)
    {
        iRetVal = SendAndRecv(objGetSnsInfoRequest.uin(0), CMD_SNSINFO_GET, 103, 1, objGetSnsInfoRequest, objGetSnsInfoResponse);
        if (iRetVal != 0)
        {
            return iRetVal;
        }

        SnsInfo stSnsInfo;
        for(int j=0; j<objGetSnsInfoResponse.sns_info_size(); j++)
        {
            stSnsInfo.uiUin = objGetSnsInfoResponse.sns_info(j).uin();
            stSnsInfo.ullPetID = objGetSnsInfoResponse.sns_info(j).petid();
            stSnsInfo.uiYuanBao = objGetSnsInfoResponse.sns_info(j).yuanbao();
            stSnsInfo.uiGrowth = objGetSnsInfoResponse.sns_info(j).growth();
            stSnsInfo.uiOnlineTime = objGetSnsInfoResponse.sns_info(j).online_time();
            stSnsInfo.bOnlineFlag = objGetSnsInfoResponse.sns_info(j).online_flg();

            mSnsInfo.insert(pair<unsigned int, SnsInfo>(stSnsInfo.uiUin, stSnsInfo));
        }

        objGetSnsInfoRequest.clear_uin();
    }

    return 0;
}

int CWebApiSnsInfo::UpdateSnsInfo(std::vector<SnsInfo> &vSnsInfo)
{
    int iRetVal = 0;

    AddSnsInfoRequest objAddSnsInfoRequest;

    for(unsigned int i=0; i<vSnsInfo.size(); i++)
    {
        objAddSnsInfoRequest.add_sns_info();
        objAddSnsInfoRequest.mutable_sns_info(objAddSnsInfoRequest.sns_info_size()-1)->set_uin(vSnsInfo[i].uiUin);
        objAddSnsInfoRequest.mutable_sns_info(objAddSnsInfoRequest.sns_info_size()-1)->set_petid(vSnsInfo[i].ullPetID);
        objAddSnsInfoRequest.mutable_sns_info(objAddSnsInfoRequest.sns_info_size()-1)->set_yuanbao(vSnsInfo[i].uiYuanBao);
        objAddSnsInfoRequest.mutable_sns_info(objAddSnsInfoRequest.sns_info_size()-1)->set_growth(vSnsInfo[i].uiGrowth);
        objAddSnsInfoRequest.mutable_sns_info(objAddSnsInfoRequest.sns_info_size()-1)->set_online_time(vSnsInfo[i].uiOnlineTime);
        objAddSnsInfoRequest.mutable_sns_info(objAddSnsInfoRequest.sns_info_size()-1)->set_online_flg(vSnsInfo[i].bOnlineFlag);

        if (objAddSnsInfoRequest.sns_info_size() >= WEBAPI_SNSINFO_MAX_UIN_NUM_PER_REQ)
        {
            iRetVal = SendAndRecv(objAddSnsInfoRequest.sns_info(0).uin(), CMD_SNSINFO_PUT, 103, 1, objAddSnsInfoRequest);
            if (iRetVal != 0)
            {
                return iRetVal;
            }

            objAddSnsInfoRequest.clear_sns_info();
        }
    }

    if (objAddSnsInfoRequest.sns_info_size() > 0)
    {
        iRetVal = SendAndRecv(objAddSnsInfoRequest.sns_info(0).uin(), CMD_SNSINFO_PUT, 103, 1, objAddSnsInfoRequest);
        if (iRetVal != 0)
        {
            return iRetVal;
        }

        objAddSnsInfoRequest.clear_sns_info();
    }

    return 0;
}

int CWebApiSnsInfo::DelSnsInfo(std::vector<unsigned int> vuiUin)
{

    int iRetVal = 0;

    DelSnsInfoRequest objDelSnsInfoRequest;

    for(unsigned int i=0; i<vuiUin.size(); i++)
    {
        objDelSnsInfoRequest.add_uin(vuiUin[i]);

        if (objDelSnsInfoRequest.uin_size() >= WEBAPI_SNSINFO_MAX_UIN_NUM_PER_REQ)
        {
            iRetVal = SendAndRecv(objDelSnsInfoRequest.uin(0), CMD_SNSINFO_OUT, 103, 1, objDelSnsInfoRequest);
            if (iRetVal != 0)
            {
                return iRetVal;
            }

            objDelSnsInfoRequest.clear_uin();
        }
    }

    if (objDelSnsInfoRequest.uin_size() > 0)
    {
        iRetVal = SendAndRecv(objDelSnsInfoRequest.uin(0), CMD_SNSINFO_OUT, 103, 1, objDelSnsInfoRequest);
        if (iRetVal != 0)
        {
            return iRetVal;
        }

        objDelSnsInfoRequest.clear_uin();
    }


    return 0;
}

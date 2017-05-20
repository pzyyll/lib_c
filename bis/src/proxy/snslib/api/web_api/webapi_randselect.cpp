#include <string.h>

//#include "server/kongfu_pet_new/kongfu_pet_protocol.h"
#include "../../../server/kongfu_pet_new/kongfu_pet_protocol.h"
#include "api/proto/rand_selector.pb.h"
#include "webapi_randselect.h"

using namespace snslib;
using namespace std;
using namespace kongfupet;

CWebApiRandSelect::CWebApiRandSelect()
{

}

CWebApiRandSelect::~CWebApiRandSelect()
{

}

int CWebApiRandSelect::GetRecommendList(unsigned int uiUin, unsigned short usLev, const std::vector<unsigned int> &vBlackList, std::vector<unsigned int> &vRecommendList)
{
    FetchRandOpponentRequest objRequest;
    FetchRandOpponentResponse objResponse;

    objRequest.set_uin(uiUin);
    objRequest.set_level(usLev);
    for (size_t i = 0; i < vBlackList.size(); ++i)
    {
    	objRequest.add_exclude(vBlackList[i]);
    }

    //发送并接收返回信息
    int iRetVal;
    iRetVal = SendAndRecv(objRequest.uin(), CMD_PET_FIGHT_MATCHER_GET_LIST, 103, 16, objRequest, objResponse);

    if (iRetVal != 0)
    {
    	return iRetVal;
    }

    for (int i = 0; i < objResponse.uins_size(); ++i)
    {
    	vRecommendList.push_back(objResponse.uins(i));
    }

    return 0;
}

int CWebApiRandSelect::UpdatePetLev(unsigned int uiUin, unsigned short usLev)
{
	UpdatePetLevel objRequest;

    objRequest.set_uin(uiUin);
    objRequest.set_level(usLev);

    //发送消息
    int iRetVal;
    iRetVal = SendAndRecv(objRequest.uin(), CMD_PET_FIGHT_MATCHER_REFRESH, 103, 16, objRequest);

    if (iRetVal != 0)
    {
        return iRetVal;
    }

    return 0;
}

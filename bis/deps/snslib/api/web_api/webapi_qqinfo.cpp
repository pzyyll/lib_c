#include <string.h>

#include "api/proto/qqinfo.pb.h"
#include "webapi_qqinfo.h"

using namespace snslib;
using namespace std;

CWebApiQQInfo::CWebApiQQInfo()
{}

CWebApiQQInfo::~CWebApiQQInfo()
{}

int CWebApiQQInfo::GetQQInfoList(unsigned int uiUin, std::vector<unsigned int> &vUinList, std::vector<QQInfo> &vQQInfoList)
{
    int iRetVal = 0;

    QQInfoRequest objRequest;
    QQInfoResponse objResponse;


    for(size_t i = 0; i < vUinList.size(); ++i)
    {
    	objRequest.add_uin(vUinList[i]);
    }

    iRetVal = SendAndRecv(uiUin, CMD_GET_QQINFO, 103, 25, objRequest, objResponse);
    if (iRetVal != 0)
    {
       return iRetVal;
    }

    for( int i = 0; i < objResponse.qqinfo_size(); ++i)
    {
    	const QQInfoResponse::QQInfo& qqInfo = objResponse.qqinfo(i);
    	QQInfo stQQInfo;
    	stQQInfo.uin = qqInfo.uin();
    	stQQInfo.VipFlag = qqInfo.vipflag();
    	stQQInfo.NickName = qqInfo.nickname();
    	stQQInfo.FaceID = qqInfo.faceid();
    	stQQInfo.FaceFlag = qqInfo.faceflag();
    	stQQInfo.AppFlag = qqInfo.appflag();
    	vQQInfoList.push_back(stQQInfo);
    }

    return 0;
}

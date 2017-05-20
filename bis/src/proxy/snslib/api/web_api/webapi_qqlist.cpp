#include <string.h>

#include "api/proto/qqlist.pb.h"
#include "webapi_qqlist.h"

using namespace snslib;
using namespace std;

CWebApiQQList::CWebApiQQList()
{}

CWebApiQQList::~CWebApiQQList()
{}

int CWebApiQQList::GetQQList(unsigned int uiUin, std::vector<unsigned int> &vQQList)
{
    int iRetVal = 0;

    QQListResponse objRequest;
    QQListResponse objResponse;

    objRequest.add_uin(uiUin);

    iRetVal = SendAndRecv(uiUin, CMD_GET_QQLIST, 103, 5, objRequest, objResponse);
    if (iRetVal != 0)
    {
       return iRetVal;
    }

    for( int i = 0; i < objResponse.uin_size(); ++i)
    {
    	vQQList.push_back(objResponse.uin(i));
    }

    return 0;
}

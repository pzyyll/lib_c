#ifndef _WEBAPI_QQLIST_H_
#define _WEBAPI_QQLIST_H_

#include <vector>
#include <string>

#include "webapi_base.h"

namespace snslib
{

class CWebApiQQList:public CWebApiBase
{
public:
	CWebApiQQList();
	~CWebApiQQList();

	int GetQQList(unsigned int uiUin, std::vector<unsigned int> &vQQList);

private:
    char m_szSendBuff[MAX_WEBAPI_BUFF_LEN];
    int m_iSendLen;
};
}
#endif //_WEBAPI_FLAG_H_

#ifndef _WEBAPI_H_
#define _WEBAPI_H_

#include "webapi_flag.h"
#include "webapi_snsinfo.h"
#include "webapi_test.h"
#include "webapi_lock.h"
#include "webapi_kongfu.h"
#include "webapi_randselect.h"
#include "webapi_qqinfo.h"
#include "webapi_qqlist.h"

namespace snslib
{
class CWebApi:public CWebApiFlag, public CWebApiSnsInfo, public CWebApiTest, public CWebApiLock, public CWebApiRandSelect, public CWebApiQQInfo, public CWebApiQQList, public CWebApiKongfu
{

public:
	CWebApi();
	~CWebApi();

public:
	int Init(const char* pszConfFile);

private:
	CWebApiConn m_objWebApiConn;
	unsigned int m_uiHostIP;
};
}
#endif //_WEBAPI_H_

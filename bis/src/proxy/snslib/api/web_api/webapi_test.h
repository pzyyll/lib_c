#ifndef _WEBAPI_TEST_H_
#define _WEBAPI_TEST_H_

#include "webapi_base.h"

namespace snslib
{

class CWebApiTest:public CWebApiBase
{
public:
    CWebApiTest();
	~CWebApiTest();

	int BenchTest(unsigned int uiUin, int iReadLen, int iWriteLen);
	int EchoTest(unsigned int uiUin, char *pszEchoText);

private:
    char m_szSendBuff[MAX_WEBAPI_BUFF_LEN];
    int m_iSendLen;
};
}
#endif //_WEBAPI_FLAG_H_

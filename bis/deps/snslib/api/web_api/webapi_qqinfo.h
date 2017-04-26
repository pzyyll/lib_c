#ifndef _WEBAPI_QQINFO_H_
#define _WEBAPI_QQINFO_H_

#include <vector>
#include <string>

#include "webapi_base.h"

typedef struct tagQQInfo
{
	int uin;
	std::string NickName;
	int VipFlag;
	int FaceFlag;
	int FaceID;
	int AppFlag;
}QQInfo;
namespace snslib
{

class CWebApiQQInfo:public CWebApiBase
{
public:
	CWebApiQQInfo();
	~CWebApiQQInfo();

	int GetQQInfoList(unsigned int uiUin, std::vector<unsigned int> &vUinList, std::vector<QQInfo> &vQQInfoList);

private:
    char m_szSendBuff[MAX_WEBAPI_BUFF_LEN];
    int m_iSendLen;
};
}
#endif //_WEBAPI_FLAG_H_

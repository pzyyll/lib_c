#ifndef _WEBAPI_SNSINFO_H_
#define _WEBAPI_SNSINFO_H_

#include <vector>
#include <map>

#include "webapi_base.h"

namespace snslib
{

typedef struct tagSnsInfo
{
    unsigned int uiUin;
    unsigned long long ullPetID;
    unsigned int uiYuanBao;
    unsigned int uiGrowth;
    unsigned int uiOnlineTime;
    bool bOnlineFlag;
}SnsInfo;

class CWebApiSnsInfo:public CWebApiBase
{
public:
    CWebApiSnsInfo();
	~CWebApiSnsInfo();

	int GetSnsInfo(std::vector<unsigned int> vuiUin, std::map<unsigned int, SnsInfo> &mSnsInfo);
    int UpdateSnsInfo(std::vector<SnsInfo> &vSnsInfo);
    int DelSnsInfo(std::vector<unsigned int> vuiUin);

private:
    char m_szSendBuff[MAX_WEBAPI_BUFF_LEN];
    int m_iSendLen;
};
}
#endif //_WEBAPI_FLAG_H_

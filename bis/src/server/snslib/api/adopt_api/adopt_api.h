#ifndef ADOPTMAPAPI_H
#define ADOPTMAPAPI_H

#include <vector>
#include "api/include/pet_protocol.h"
#include "api/include/pet_info_def.h"
#include "comm/udp_client/udp_client.h"

namespace snslib
{

typedef struct tagSectSvrConf
{
	char szIP[MAX_IP_LEN + 1];
	int iPort;
	int iUinBegin;
	int iUinEnd;

} SSectSvrConf;

const int MAX_ADOPT_PET_COUNT = 8;

typedef struct tagAdoptGetResp
{
    unsigned char byPetNum;
    unsigned long long aullPetID[MAX_ADOPT_PET_COUNT];
    char aszPetName[MAX_ADOPT_PET_COUNT][PET_NAME_LEN + 1];

} SAdoptGetResp;

class CAdoptAPI
{
public:
    CAdoptAPI();
    ~CAdoptAPI();

    int Init(const char * pszFile);

	inline const char *GetErrMsg() const
	{
		return m_szErrMsg;
	}

	int Get(unsigned int uiUin, SAdoptGetResp &stResponse);
	int GetPig(unsigned int uiUin, SAdoptGetResp &stResponse);
	int GetAll(unsigned int uiUin, SAdoptGetResp &stResponse);
	int Set(unsigned long long ullPetID, char * pszPetName);
    int Del(unsigned long long ullPetID);

private:
    SSectSvrConf * GetSvrConf(unsigned int uiUin);
    int ConnectSvr(unsigned uiUin);
    int SendAndRecv(unsigned short ushCmd, unsigned int uiUin, void *pRequest, void *pResponse);

	snslib::CUdpClient m_UdpClient;

	char m_szErrMsg[1024];
	unsigned char m_abyRequest[MAX_PET_PKG_LEN];
	unsigned int m_uiRequestLen;
	unsigned char m_abyResponse[MAX_PET_PKG_LEN];
	unsigned int m_uiResponseLen;

	std::vector<SSectSvrConf> m_SvrConf;
	int m_iTimeOut;
};

}
#endif


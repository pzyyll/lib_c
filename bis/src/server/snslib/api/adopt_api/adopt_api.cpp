#include <stdio.h>
#include <string.h>

#include "adopt_protocol.h"
#include "comm/ini_file/ini_file.h"

using namespace snslib;

typedef struct tagAdopSetReq
{
    unsigned long long ullPetID;
    char szPetName[PET_NAME_LEN + 1];
} SAdoptSetReq;

CAdoptAPI::CAdoptAPI(): m_uiRequestLen(0), m_uiResponseLen(0), m_iTimeOut(0)
{
	memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
	memset(m_abyRequest, 0, sizeof(m_abyRequest));
	memset(m_abyResponse, 0, sizeof(m_abyResponse));
}

CAdoptAPI::~CAdoptAPI()
{
	m_UdpClient.Close();
}

int CAdoptAPI::Init(const char * pszFile)
{
	if (NULL == pszFile)
	{
		strncpy(m_szErrMsg, "config file error", sizeof(m_szErrMsg) - 1);
		return -1;
	}

	CIniFile objIni(pszFile);
	if (objIni.IsValid())
	{
		int iSvrCount = 0;
		objIni.GetInt("General", "RegionCount", 0, &iSvrCount);
		objIni.GetInt("General", "TimeOut", 5000, &m_iTimeOut);
		if (iSvrCount <= 0)
		{
			strncpy(m_szErrMsg, "config file invalid", sizeof(m_szErrMsg) - 1);
			return -1;
		}

		m_SvrConf.clear();

		char szSecTitle[128] = {0};
		SSectSvrConf stSvrConf;
		for (int i = 0; i < iSvrCount; ++i)
		{
			snprintf(szSecTitle, sizeof(szSecTitle), "Region_%d", i+1);
			objIni.GetString(szSecTitle, "ServerIP", "", stSvrConf.szIP, sizeof(stSvrConf.szIP));
			objIni.GetInt(szSecTitle, "ServerPort", 0, &stSvrConf.iPort);
			objIni.GetInt(szSecTitle, "StartUIN", 0, &stSvrConf.iUinBegin);
			objIni.GetInt(szSecTitle, "EndUIN", 0, &stSvrConf.iUinEnd);

			if (stSvrConf.szIP[0] == 0 || stSvrConf.iPort == 0 ||
				stSvrConf.iUinBegin > stSvrConf.iUinEnd)
			{
				strncpy(m_szErrMsg, "config file invalid", sizeof(m_szErrMsg) - 1);
				return -1;
			}

			m_SvrConf.push_back(stSvrConf);
		}
	}
	else
	{
		strncpy(m_szErrMsg, "config file invalid", sizeof(m_szErrMsg) - 1);
		return -1;
	}

	m_UdpClient.SetTimeOut(m_iTimeOut);

	return 0;
}

SSectSvrConf * CAdoptAPI::GetSvrConf(unsigned int uiUin)
{
	std::vector<SSectSvrConf>::iterator it;
	int iUin = uiUin % 100;
	for (it = m_SvrConf.begin(); it != m_SvrConf.end(); ++it)
	{
		if (iUin >= it->iUinBegin && iUin <= it->iUinEnd)
		{
			return &(*it);
		}
	}

	return NULL;
}

int CAdoptAPI::ConnectSvr(unsigned uiUin)
{
	SSectSvrConf* pstSvrConf = GetSvrConf(uiUin);
	if (pstSvrConf == NULL)
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "get server config failed, uin=%u", uiUin);
		return -1;
	}
	if (0 != m_UdpClient.Connect(pstSvrConf->szIP, pstSvrConf->iPort))
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "connect server failed，uin=%u ip=%s,port=%d",
			uiUin, pstSvrConf->szIP, pstSvrConf->iPort);
		return -1;
	}
	return 0;
}

int CAdoptAPI::SendAndRecv(unsigned short ushCmd, unsigned int uiUin, void *pRequest, void *pResponse)
{
	// 构造协议头
	SSvrMsgHeader stHeader = {0};
    stHeader.iPkgLen = 0;
    stHeader.iPetVersion = 2;
    stHeader.iUin = uiUin;
    stHeader.iPetCmdID = ushCmd;
    stHeader.cPetMsgType = 0;


	// 请求数据打包
	int iOffset = 0;
	unsigned char *p = m_abyRequest + SVR_MSG_HEADER_LEN;
	switch (stHeader.iPetCmdID)
	{
	case ADOPT_MODIFY_PROTO_CMD_GET:
	case ADOPT_MODIFY_PROTO_CMD_GETALL:
		break;
	case ADOPT_MODIFY_PROTO_CMD_SET:
        {
            unsigned long long ullPetID = *(unsigned long long*)pRequest;
            char szPetName[PET_NAME_LEN + 1] = {0};
            strncpy(szPetName, (char *)pRequest + 8, PET_NAME_LEN);
    		iOffset = CAdoptProtocol::PackSetRequest(p, ullPetID, szPetName);
    	}
		break;
	case ADOPT_MODIFY_PROTO_CMD_DEL:
		iOffset = CAdoptProtocol::PackDelRequest(p, *(unsigned long long *)pRequest);
		break;
	default:
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "request cmd error, cmd=%04x", stHeader.iPetCmdID);
		return -1;
	}

	stHeader.iPkgLen = iOffset + SVR_MSG_HEADER_LEN;
	m_uiRequestLen = stHeader.iPkgLen;
	CAdoptProtocol::PackHeader(m_abyRequest, stHeader);

    // 发送请求
	if (0 != m_UdpClient.Send(m_abyRequest, m_uiRequestLen))
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "send failed(%s)", m_UdpClient.GetErrMsg());
		return -1;
	}

    if (stHeader.iPetCmdID == ADOPT_MODIFY_PROTO_CMD_DEL)
    {
        *(short *)pResponse = 0;
        return 0;
    }

    // 接收应答
	m_uiResponseLen = sizeof(m_abyResponse);
	if (0 != m_UdpClient.Recv(m_abyResponse, m_uiResponseLen))
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "recv failed(%s)", m_UdpClient.GetErrMsg());
		return -1;
	}

	CAdoptProtocol::UnPackHeader(m_abyResponse, stHeader);
	if (stHeader.iPetCmdID != ushCmd)
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "response cmd error, cmd=%04x", stHeader.iPetCmdID);
		return -1;
	}
    if (stHeader.iPkgLen != m_uiResponseLen)
    {
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "recv package length(%u) is not equal to the "
            "iPkgLen(%hu) field in header", m_uiResponseLen, stHeader.iPkgLen);
		return -1;
    }

	p = m_abyResponse + SVR_MSG_HEADER_LEN;
	switch (stHeader.iPetCmdID)
	{
	case ADOPT_MODIFY_PROTO_CMD_GET:
	case ADOPT_MODIFY_PROTO_CMD_GETALL:
		iOffset = CAdoptProtocol::UnPackGetResponse(p, *(SAdoptGetResp *)pResponse);
		break;
	case ADOPT_MODIFY_PROTO_CMD_SET:
	case ADOPT_MODIFY_PROTO_CMD_DEL:
		iOffset = CAdoptProtocol::UnPackSetResponse(p, *(short *)pResponse);
		break;
	default:
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "response cmd error, cmd=%u", stHeader.iPetCmdID);
		return -1;
	}

	return 0;
}


int CAdoptAPI::Get(unsigned int uiUin, SAdoptGetResp &stResponse)
{
	int iRetCode = 0;
	if ((iRetCode = ConnectSvr(uiUin)) != 0) return -1;

	memset(&stResponse, 0, sizeof(stResponse));
	iRetCode = SendAndRecv(ADOPT_MODIFY_PROTO_CMD_GET, uiUin, &uiUin, &stResponse);
	m_UdpClient.Close();

	return iRetCode;
}

int CAdoptAPI::GetPig(unsigned int uiUin, SAdoptGetResp &stResponse)
{
    int iRetCode = 0;
    if ((iRetCode = ConnectSvr(uiUin)) != 0) return -1;

    SAdoptGetResp stAllResponse;
    memset(&stAllResponse, 0, sizeof(stAllResponse));
    iRetCode = SendAndRecv(ADOPT_MODIFY_PROTO_CMD_GETALL, uiUin, &uiUin, &stAllResponse);
    m_UdpClient.Close();

    if (iRetCode == 0)
    {
        memset(&stResponse, 0, sizeof(stResponse));
        PetID stPetID;
        for (unsigned int i = 0; i < stAllResponse.byPetNum; ++i)
        {
            stPetID.ullID = stAllResponse.aullPetID[i];
            if (stPetID.ushSpec == 104 || stPetID.ushSpec == 105)
            {
                stResponse.aullPetID[stResponse.byPetNum] = stPetID.ullID;
                strncpy(stResponse.aszPetName[stResponse.byPetNum], stAllResponse.aszPetName[i], PET_NAME_LEN);
                stResponse.byPetNum++;
            }
        }
    }

    return iRetCode;
}

int CAdoptAPI::GetAll(unsigned int uiUin, SAdoptGetResp &stResponse)
{
	int iRetCode = 0;
	if ((iRetCode = ConnectSvr(uiUin)) != 0) return -1;

	memset(&stResponse, 0, sizeof(stResponse));
	iRetCode = SendAndRecv(ADOPT_MODIFY_PROTO_CMD_GETALL, uiUin, &uiUin, &stResponse);
	m_UdpClient.Close();

	return iRetCode;
}

int CAdoptAPI::Set(unsigned long long ullPetID, char * pszPetName)
{
	int iRetCode = 0;

	unsigned int uiUin;
	memcpy(&uiUin, &ullPetID, 4);

	if ((iRetCode = ConnectSvr(uiUin)) != 0) return -1;

    SAdoptSetReq stRequest;
    memset(&stRequest, 0, sizeof(stRequest));
    stRequest.ullPetID = ullPetID;
    strncpy(stRequest.szPetName, pszPetName, PET_NAME_LEN);

    short shResult = 0;

	iRetCode = SendAndRecv(ADOPT_MODIFY_PROTO_CMD_SET, uiUin, &stRequest, &shResult);

	m_UdpClient.Close();

    if (iRetCode != 0 || shResult != 0)
    {
        return -1;
    }
    return 0;
}

int CAdoptAPI::Del(unsigned long long ullPetID)
{
	int iRetCode = 0;

	unsigned int uiUin;
	memcpy(&uiUin, &ullPetID, 4);

	if ((iRetCode = ConnectSvr(uiUin)) != 0) return -1;

    short shResult = 0;

	iRetCode = SendAndRecv(ADOPT_MODIFY_PROTO_CMD_DEL, uiUin, &ullPetID, &shResult);

	m_UdpClient.Close();

    if (iRetCode != 0 || shResult != 0)
    {
        return -1;
    }
    return 0;
}


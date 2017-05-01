#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <climits>
#include <algorithm>
#include <functional>
#include <iconv.h>

#include "api/oidb_api/oidb_api.h"
#include "comm/util/pet_util.h"
#include "comm/ini_file/ini_file.h"
#include "comm/log/pet_log.h"

using namespace snslib;

#define OIDB_API_MIN_UIN 10001

COIDBProxyAPI* COIDBProxyAPI::m_pSingleton = NULL;

COIDBProxyAPI::COIDBProxyAPI() :
    m_bInit(false), m_iTimeout(0), m_uiRequestLen(0), m_uiResponseLen(0)
{
    memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
    memset(m_abyRequest, 0, sizeof(m_abyRequest));
    memset(m_abyResponse, 0, sizeof(m_abyResponse));

    m_iOIDBProxyNum = 0;
}

COIDBProxyAPI::~COIDBProxyAPI()
{
}

int COIDBProxyAPI::Init(const char *pszFile, const char *pszServiceName)
{
    if (m_bInit)
    {
        return 0;
    }

    int iRetVal = 0;
    if (pszFile == NULL)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config file is null");
        return RET_FAIL;
    }

    m_uiReqSeq=CRandomTool::Instance()->Get(0, INT_MAX);
    std::vector<STcpAddr> vstSTcpAddr;

    CIniFile objIni(pszFile);
    if (objIni.IsValid())
    {
        objIni.GetInt("OIDB_PROXY", "Timeout", 0, &m_iTimeout);
        objIni.GetInt("OIDB_PROXY", "OIDBProxyNum", 0, &m_iOIDBProxyNum);

        if ((m_iOIDBProxyNum <= 0) || (m_iOIDBProxyNum > 10))
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config item OIDB_PROXY/OIDBProxyNum [%d] is not valid", m_iOIDBProxyNum);
            return RET_FAIL;
        }

        char szSecName[1024];
        char szServerIP[20];
        int iServerPort;
        STcpAddr stCurAddr;
        for (int i = 0; i < m_iOIDBProxyNum; i++)
        {
            snprintf(szSecName, sizeof(szSecName), "OIDB_PROXY_%d", i + 1);

            objIni.GetString(szSecName, "IP", "", szServerIP, sizeof(szServerIP));
            objIni.GetInt(szSecName, "Port", 0, &iServerPort);
            if (szServerIP[0] == 0 || iServerPort == 0)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config item %s/IP[%s] /Port[%d] is not valid", szSecName, szServerIP, iServerPort);
                return RET_FAIL;
            }

            strncpy(m_astProxyConf[i].szHost, szServerIP, 20);
            m_astProxyConf[i].iPort = iServerPort;

            inet_aton(szServerIP, (struct in_addr *) &stCurAddr.uiIPAddr);
            stCurAddr.ushPort = iServerPort;

            vstSTcpAddr.push_back(stCurAddr);
        }
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config file is invalid");
        return RET_FAIL;
    }

    //设置本地IP地址
    const char *pszLocalIP;
    if ((pszLocalIP = CSysTool::GetNicAddr("eth1")) == NULL)
    {
        if ((pszLocalIP = CSysTool::GetNicAddr("eth0")) == NULL)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "get ip addr from eth1 or eth0 failed");
            return RET_FAIL;
        }
    }

    inet_aton(pszLocalIP, (struct in_addr *) &m_uiServiceIP);
    if (pszServiceName != NULL)
    {
        snprintf(m_szServiceName, sizeof(m_szServiceName), "%s", pszServiceName);
    }

    iRetVal = m_oTcpClient.Init(vstSTcpAddr, m_iTimeout);
    if (iRetVal != 0)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "init safe_tcp_client failed, ret=%d, errmsg=%s", iRetVal, m_oTcpClient.GetErrMsg());
        return RET_FAIL;
    }

    m_bInit = true;
    return RET_OK;
}

COIDBProxyAPI& COIDBProxyAPI::Instance()
{
    if (m_pSingleton == NULL)
    {
        m_pSingleton = new COIDBProxyAPI();
    }

    return *m_pSingleton;
}

int COIDBProxyAPI::SendAndRecv()
{
    int iRetVal = 0;

#if 0
    // 2013年11月28日 21:47:42
    // shimmeryang，我觉得这一块代码写的有问题，你重试两次不一定就能成功的，而且组包的地方不需要每次都做吧
    int iRetryTime = 2;
    do
    {
        m_uiRequestLen = PackTransPkg(m_abyRequest, m_stReqTransPkg);
        m_uiResponseLen = sizeof(m_abyResponse);

        //接收包头
        if ((iRetVal = m_oTcpClient.SendAndRecv(m_abyRequest, m_uiRequestLen, m_abyResponse, &m_uiResponseLen, (1 + sizeof(STransPkgHeader)))) != 0)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "SendAndRecv pkg header: %s", m_oTcpClient.GetErrMsg());
            PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "send_recv faild, ret=%d, errmsg=%s", iRetVal, m_oTcpClient.GetErrMsg());
            m_oTcpClient.CloseCurConn();
            //return RET_FAIL;
        }
        else
        {
            break;
        }
    }while(--iRetryTime);
#endif
    m_uiRequestLen = PackTransPkg(m_abyRequest, m_stReqTransPkg);
    m_uiResponseLen = sizeof(m_abyResponse);

    for (int i = 0 ; i < 2; i++)
    {
        //接收包头
        iRetVal = m_oTcpClient.SendAndRecv(m_abyRequest, m_uiRequestLen, m_abyResponse, &m_uiResponseLen, (1 + sizeof(STransPkgHeader)));
        if (iRetVal == 0)
        {
            break;
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "SendAndRecv pkg header: %s", m_oTcpClient.GetErrMsg());
            PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "send_recv faild, ret=%d, errmsg=%s", iRetVal, m_oTcpClient.GetErrMsg());
            m_oTcpClient.CloseCurConn();
            //return RET_FAIL;
        }
    }

    if (iRetVal != 0)
    {
        return RET_FAIL;
    }

    //校验包头
    char cTmpStx = m_abyResponse[0];
    STransPkgHeader *pstTransHeader = (STransPkgHeader *) (m_abyResponse + 1);

    //校验接收、应答对应的包类型（登陆态/非登陆态）是否一致
    if (cTmpStx != m_stReqTransPkg.cStx)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Response stx is not valid, req_stx=0x%x, rsp_stx=0x%x", m_stReqTransPkg.cStx, cTmpStx);
        PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "Response stx is not valid, req_stx=0x%x, rsp_stx=0x%x", m_stReqTransPkg.cStx, cTmpStx);
        m_oTcpClient.CloseCurConn();
        return RET_FAIL;
    }

    //校验UIN
    if ((*(unsigned int *) pstTransHeader->abyUin) != (*(unsigned int *) m_stReqTransPkg.stHeader.abyUin))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Response uin is not valid, req_uin=%u, rsp_uin=%u", ntohl(*(unsigned int *) m_stReqTransPkg.stHeader.abyUin), ntohl(*(unsigned int *) pstTransHeader->abyUin));
        PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "Response uin is not valid, req_uin=%u, rsp_uin=%u", ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), ntohl(*(unsigned int *)pstTransHeader->abyUin));
        m_oTcpClient.CloseCurConn();
        return RET_FAIL;
    }

    //校验abyVersion
    if (ntohs(*(unsigned short *) pstTransHeader->abyVersion) != 5)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Response header_ver is not valid, rsp_header_ver=%d", ntohs(*(unsigned short *) pstTransHeader->abyVersion));
        PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "Response header_ver is not valid, rsp_header_ver=%d", ntohs(*(unsigned short *)pstTransHeader->abyVersion));
        m_oTcpClient.CloseCurConn();
        return RET_FAIL;
    }

    //校验abyCommand
    if (ntohs(*(unsigned short *) pstTransHeader->abyCommand) != ntohs(*(unsigned short *) m_stReqTransPkg.stHeader.abyCommand))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Response cmd is not valid, req_cmd=%d, rsp_cmd=%d", ntohs(*(unsigned short *) pstTransHeader->abyCommand), ntohs(*(unsigned short *) m_stReqTransPkg.stHeader.abyCommand));
        PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "Response cmd is not valid, req_cmd=%d, rsp_cmd=%d", ntohs(*(unsigned short *)pstTransHeader->abyCommand), ntohs(*(unsigned short *)m_stReqTransPkg.stHeader.abyCommand));
        m_oTcpClient.CloseCurConn();
        return RET_FAIL;
    }

    //校验abyLength
    unsigned int ushRetPkgLen = ntohs(*(unsigned short *) pstTransHeader->abyLength);
    if (ushRetPkgLen < (sizeof(STransPkgHeader) + 2) || (ushRetPkgLen > OIDB_API_MAX_PKG_LEN))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Response pkg_len is not valid, rsp_pkg_len=%d", ushRetPkgLen);
        PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "Response pkg_len is not valid, rsp_pkg_len=%d", ushRetPkgLen);
        m_oTcpClient.CloseCurConn();
        return RET_FAIL;
    }

    //接收包体
    unsigned int uiCurRecvLen = sizeof(m_abyResponse) - sizeof(STransPkgHeader) - 1;
    if ((iRetVal = m_oTcpClient.Recv(m_abyResponse + sizeof(STransPkgHeader) + 1, &uiCurRecvLen, ushRetPkgLen - sizeof(STransPkgHeader) - 1)) != 0)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "Recv pkg body: %s", m_oTcpClient.GetErrMsg());
        PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "send_recv faild, ret=%d, errmsg=%s", iRetVal, m_oTcpClient.GetErrMsg());
        m_oTcpClient.CloseCurConn();
        return RET_FAIL;
    }
    m_uiResponseLen += uiCurRecvLen;

    int iUnPackLen = UnPackTransPkg(m_abyResponse, m_uiResponseLen, m_stRspTransPkg);
    if (iUnPackLen != (int) m_uiResponseLen)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "UnPack rsp pkg fail, unpack_len=%d, recv_len=%d", iUnPackLen, m_uiResponseLen);
        PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "UnPack rsp pkg fail, unpack_len=%d, recv_len=%d", iUnPackLen, m_uiResponseLen);
        m_oTcpClient.CloseCurConn();
        return RET_FAIL;
    }

    if (m_stRspTransPkg.cEtx != 0x3)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "UnPack rsp pkg fail, rsp_etx=0x%x", m_stRspTransPkg.cEtx);
        PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "UnPack rsp pkg fail, rsp_etx=0x%x", m_stRspTransPkg.cEtx);
        m_oTcpClient.CloseCurConn();
        return RET_FAIL;
    }

    return RET_OK;
}

/*
int COIDBProxyAPI::SendAndRecvOld()
{
    int iRetVal = 0;

    m_uiResponseLen = sizeof(m_abyResponse);
    if ((iRetVal = m_oTcpClient.SendAndRecv(m_abyRequest, m_uiRequestLen, m_abyResponse, &m_uiResponseLen)) != 0)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "SendAndRecv: %s", m_oTcpClient.GetErrMsg());
        PetLog(0, 0, PETLOG_WARN, "connection send_recv faild, ret=%d, errmsg=%s", iRetVal, m_oTcpClient.GetErrMsg());
        m_oTcpClient.CloseCurConn();
        return RET_FAIL;
    }

    if (m_uiResponseLen < OIDB_API_PROTO_HEADER_LEN)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "recv length(%u) error", m_uiResponseLen);
        PetLog(0, 0, PETLOG_WARN, "connection recv length is err, recv_len=%d", m_uiResponseLen);
        m_oTcpClient.CloseCurConn();
        return RET_FAIL;
    }

    UnPackHeader(m_abyResponse, m_stHeader);

    if (m_uiResponseLen < m_stHeader.ushLength)
    {
        int iRemainBuffLen = sizeof(m_abyResponse) - m_uiResponseLen;
        int iRemainDataLen = m_stHeader.ushLength - m_uiResponseLen;
        iRetVal = m_oTcpClient.Recv(m_abyResponse + m_uiResponseLen, (unsigned int *) &iRemainBuffLen, iRemainDataLen);
        if (iRetVal != 0)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "connection recv remain data failed, header length(%u) remain length(%u), ret=%d, errmsg=%s", m_stHeader.ushLength, iRemainDataLen, iRetVal, m_oTcpClient.GetErrMsg());
            PetLog(0, 0, PETLOG_WARN, "connection recv remain data failed, header length(%u) remain length(%u), ret=%d, errmsg=%s", m_stHeader.ushLength, iRemainDataLen, iRetVal, m_oTcpClient.GetErrMsg());
            m_oTcpClient.CloseCurConn();
            return RET_FAIL;
        }
    }
    else if (m_uiResponseLen != m_stHeader.ushLength)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "data error: header length(%u) != recv length(%u)", m_stHeader.ushLength, m_uiResponseLen);
        PetLog(0, 0, PETLOG_WARN, "connection recv length is not valid, recv_len=%d, recv_len_in_header=%d", m_uiResponseLen, m_stHeader.ushLength);
        m_oTcpClient.CloseCurConn();
        return RET_FAIL;
    }

    return RET_OK;
}
*/

int COIDBProxyAPI::PackTransPkg(char * pMem, const STransPkg &stPkg)
{
    int iOffset = 1 + sizeof(STransPkgHeader);
    if (stPkg.byExtraFlag)
    {
        iOffset += CBuffTool::WriteShort(pMem + iOffset, stPkg.stExtraInfo.ushExLen);
        iOffset += CBuffTool::WriteShort(pMem + iOffset, stPkg.stExtraInfo.shExVer);
        iOffset += CBuffTool::WriteInt(pMem + iOffset, stPkg.stExtraInfo.uiAppID);
        iOffset += CBuffTool::WriteByte(pMem + iOffset, stPkg.stExtraInfo.chKeyType);
        iOffset += CBuffTool::WriteShort(pMem + iOffset, stPkg.stExtraInfo.ushSessionKeyLen);
        iOffset += CBuffTool::WriteString(pMem + iOffset, stPkg.stExtraInfo.szBuffSessionKey, stPkg.stExtraInfo.ushSessionKeyLen);
        iOffset += CBuffTool::WriteShort(pMem + iOffset, stPkg.stExtraInfo.ushReservedLen);
        iOffset += CBuffTool::WriteString(pMem + iOffset, stPkg.stExtraInfo.szBuffReserved, stPkg.stExtraInfo.ushReservedLen);
        iOffset += CBuffTool::WriteShort(pMem + iOffset, stPkg.stExtraInfo.ushContextDataLen);
        iOffset += CBuffTool::WriteString(pMem + iOffset, stPkg.stExtraInfo.szContextData, stPkg.stExtraInfo.ushContextDataLen);
    }
    iOffset += CBuffTool::WriteString(pMem + iOffset, stPkg.abyBody, stPkg.ushBodyLen);
    iOffset += CBuffTool::WriteByte(pMem + iOffset, stPkg.cEtx);
    *(unsigned short *) stPkg.stHeader.abyLength = htons(iOffset);
    memcpy(pMem, &stPkg, 1 + sizeof(STransPkgHeader));
    return iOffset;
}

int COIDBProxyAPI::UnPackTransPkg(const char * pMem, unsigned uiLen, STransPkg& stPkg)
{
    memcpy(&stPkg, pMem, 1 + sizeof(STransPkgHeader));
    char cTmpStx = pMem[0];
    int iOffset = 1 + sizeof(STransPkgHeader);
    if (cTmpStx == 0xa)
    {
        stPkg.byExtraFlag = 1;
        iOffset += CBuffTool::ReadShort(pMem + iOffset, stPkg.stExtraInfo.ushExLen);
        iOffset += CBuffTool::ReadShort(pMem + iOffset, stPkg.stExtraInfo.shExVer);
        iOffset += CBuffTool::ReadInt(pMem + iOffset, stPkg.stExtraInfo.uiAppID);
        iOffset += CBuffTool::ReadByte(pMem + iOffset, stPkg.stExtraInfo.chKeyType);
        iOffset += CBuffTool::ReadShort(pMem + iOffset, stPkg.stExtraInfo.ushSessionKeyLen);
        if (stPkg.stExtraInfo.ushSessionKeyLen > 128)
        {
            stPkg.stExtraInfo.ushSessionKeyLen = 128;
        }
        iOffset += CBuffTool::ReadString(pMem + iOffset, stPkg.stExtraInfo.szBuffSessionKey, stPkg.stExtraInfo.ushSessionKeyLen);
        iOffset += CBuffTool::ReadShort(pMem + iOffset, stPkg.stExtraInfo.ushReservedLen);
        if (stPkg.stExtraInfo.ushReservedLen > 64)
        {
            stPkg.stExtraInfo.ushReservedLen = 64;
        }
        iOffset += CBuffTool::ReadString(pMem + iOffset, stPkg.stExtraInfo.szBuffReserved, stPkg.stExtraInfo.ushReservedLen);
        iOffset += CBuffTool::ReadShort(pMem + iOffset, stPkg.stExtraInfo.ushContextDataLen);
        if (stPkg.stExtraInfo.ushContextDataLen > 64)
        {
            stPkg.stExtraInfo.ushContextDataLen = 64;
        }
        iOffset += CBuffTool::ReadString(pMem + iOffset, stPkg.stExtraInfo.szContextData, stPkg.stExtraInfo.ushContextDataLen);
    }
    else
    {
        stPkg.byExtraFlag = 0;
    }
    stPkg.ushBodyLen = uiLen - iOffset - 1;
    iOffset += CBuffTool::ReadString(pMem + iOffset, stPkg.abyBody, stPkg.ushBodyLen);
    iOffset += CBuffTool::ReadByte(pMem + iOffset, stPkg.cEtx);

    return iOffset;
}

int COIDBProxyAPI::FillReqTransHeader(unsigned int uiUin, unsigned short ushCmdID, char cServiceType, const SessionContext *pstContext /*= NULL*/)
{
    memset(&m_stReqTransPkg, 0, sizeof(m_stReqTransPkg));

    if (ushCmdID >= 0x400)
    {
        m_stReqTransPkg.cStx = 0x0a;
        if (pstContext == NULL)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "cmd_id[0x%x] but session_context is null", ushCmdID);
            return -1;
        }
    }
    else
    {
        m_stReqTransPkg.cStx = 0x2;
    }

    m_stReqTransPkg.cEtx = 0x3;

    //TransHeader
    *(unsigned short *) m_stReqTransPkg.stHeader.abyVersion = htons(5);
    *(unsigned short *) m_stReqTransPkg.stHeader.abyCommand = htons(ushCmdID);
    *(unsigned int *) m_stReqTransPkg.stHeader.abyUin = htonl(uiUin);

    SAssistInfo *pstAssistInfo = &m_stReqTransPkg.stHeader.stAssistInfo;
    //AssistInfo里面的UserName由于是在OIDB那边进行填写的，所以将该字段暂时用于标识OIDB透明传输包的字段
    //AssistInfo里面的UserName从传输包的第15字节开始
    snprintf(pstAssistInfo->szUserName, sizeof(pstAssistInfo->szUserName), "%s", OIDB_TRANS_PKG_MAGIC);
    if ((pstContext) && (pstContext->uiConnSvrIP != 0))
    {
        *(unsigned int *) pstAssistInfo->abyServiceIP = pstContext->uiConnSvrIP;
    }
    else
    {
        *(unsigned int *) pstAssistInfo->abyServiceIP = m_uiServiceIP;
    }

    snprintf(pstAssistInfo->szServiceName, sizeof(pstAssistInfo->szServiceName), "%s", m_szServiceName);
    *(unsigned int *) pstAssistInfo->abyServiceSeq = htonl(m_uiReqSeq++);
    pstAssistInfo->cServiceType = cServiceType;
    if (pstContext)
    {
        *(unsigned int *) pstAssistInfo->abyClientIP = pstContext->uiClientIP;
    }
    *(unsigned int *) pstAssistInfo->abyClientUin = htonl(uiUin);
    //如下三个字段由OIDB_PROXY填充
    //char szUserName[11];        //调用接口的用户名
    //char szPasswd[11];          //调用接口的用户密码
    //char szDesc[30];            //备注说明

    //ExtraHeader

    if (ushCmdID >= 0x400)
    {
        m_stReqTransPkg.byExtraFlag = 1;

        TransPkgHeadExt *pstExtraInfo = &m_stReqTransPkg.stExtraInfo;
        pstExtraInfo->shExVer = 600;
        pstExtraInfo->uiAppID = pstContext->uiAppID;
        pstExtraInfo->chKeyType = pstContext->cKeyType;
        if (pstContext->cKeyType == OIDB_SESSION_SSO)
        {
            //如果是SSO类型的SESSIONKEY，需要转换成2进制的形式
            int iSSOBinKeyLen = 0;
            const char *pcSSOBinKey = CStrTool::Hex2Str(pstContext->szSessionKey, pstContext->ushSessionKeyLen, &iSSOBinKeyLen);
            if ((iSSOBinKeyLen > 128) || (iSSOBinKeyLen <= 0))
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "SSO key is not valid, sso_key_len=%d, sso_key=%s", pstContext->ushSessionKeyLen, pstContext->szSessionKey);
                return RET_FAIL;
            }
            pstExtraInfo->ushSessionKeyLen = iSSOBinKeyLen;
            memcpy(pstExtraInfo->szBuffSessionKey, pcSSOBinKey, iSSOBinKeyLen);
        }
        else
        {
            if (pstContext->ushSessionKeyLen > 128)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "session key is not valid, session_key_len=%d, session_key=%s", pstContext->ushSessionKeyLen, pstContext->szSessionKey);
                return RET_FAIL;
            }
            pstExtraInfo->ushSessionKeyLen = pstContext->ushSessionKeyLen;
            memcpy(pstExtraInfo->szBuffSessionKey, pstContext->szSessionKey, pstContext->ushSessionKeyLen);
        }
        pstExtraInfo->ushExLen = 15 + pstExtraInfo->ushSessionKeyLen;
    }
    else
    {
        m_stReqTransPkg.byExtraFlag = 0;
    }

    return 0;
}

int COIDBProxyAPI::PackHeader(char * pMem, const SOIDBProtoHeader &stHeader)
{
    int iOffset = 0;
    iOffset += CBuffTool::WriteShort(pMem + iOffset, stHeader.ushLength);
    iOffset += CBuffTool::WriteInt(pMem + iOffset, stHeader.uiUin);
    iOffset += CBuffTool::WriteShort(pMem + iOffset, stHeader.ushCommand);
    iOffset += CBuffTool::WriteInt(pMem + iOffset, stHeader.uiServiceIP, 0);
    iOffset += CBuffTool::WriteString(pMem + iOffset, (const char *) stHeader.szServiceName, sizeof(stHeader.szServiceName));
    iOffset += CBuffTool::WriteInt(pMem + iOffset, stHeader.uiClientIP, 0);
    iOffset += CBuffTool::WriteByte(pMem + iOffset, stHeader.byServiceType);
    iOffset += CBuffTool::WriteByte(pMem + iOffset, stHeader.byResult);

    if (stHeader.ushExtraFlag)
    {
        iOffset += CBuffTool::WriteInt(pMem + iOffset, stHeader.uiAppID);
        iOffset += CBuffTool::WriteByte(pMem + iOffset, stHeader.chKeyType);
        iOffset += CBuffTool::WriteShort(pMem + iOffset, stHeader.ushSessionKeyLen);
        iOffset += CBuffTool::WriteString(pMem + iOffset, stHeader.szSessionKey, stHeader.ushSessionKeyLen);
    }
    return iOffset;
}

int COIDBProxyAPI::UnPackHeader(const char * pMem, SOIDBProtoHeader &stHeader)
{
    int iOffset = 0;
    iOffset += CBuffTool::ReadShort(pMem + iOffset, stHeader.ushLength);
    iOffset += CBuffTool::ReadInt(pMem + iOffset, stHeader.uiUin);
    iOffset += CBuffTool::ReadShort(pMem + iOffset, stHeader.ushCommand);
    iOffset += CBuffTool::ReadInt(pMem + iOffset, stHeader.uiServiceIP, 0);
    iOffset += CBuffTool::ReadString(pMem + iOffset, stHeader.szServiceName, sizeof(stHeader.szServiceName));
    iOffset += CBuffTool::ReadInt(pMem + iOffset, stHeader.uiClientIP, 0);
    iOffset += CBuffTool::ReadByte(pMem + iOffset, stHeader.byServiceType);
    iOffset += CBuffTool::ReadByte(pMem + iOffset, stHeader.byResult);
    return iOffset;
}

int COIDBProxyAPI::UnPackExtraHeader(const char * pMem, SOIDBProtoHeader &stHeader)
{
    int iOffset = 0;
    iOffset += CBuffTool::ReadInt(pMem + iOffset, stHeader.uiAppID);
    iOffset += CBuffTool::ReadByte(pMem + iOffset, stHeader.chKeyType);
    iOffset += CBuffTool::ReadShort(pMem + iOffset, stHeader.ushSessionKeyLen);
    iOffset += CBuffTool::ReadString(pMem + iOffset, stHeader.szSessionKey, stHeader.ushSessionKeyLen);
    return iOffset;
}

int COIDBProxyAPI::GetFriendList(unsigned int uiUin, std::vector<unsigned int> &vecFriends)
{
    if (FillReqTransHeader(uiUin, 0x61, 0) != 0)
    {
        return RET_FAIL;
    }

    //包体为空

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        unsigned short ushFriendNum = 0;
        iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushFriendNum);
        unsigned int uiFriendUin = 0;
        for (unsigned int i = 0; i < ushFriendNum; ++i)
        {
            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiFriendUin);
            vecFriends.push_back(uiFriendUin);
        }
        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

int COIDBProxyAPI::GetFriendList(unsigned int uiUin, std::vector<unsigned int> &vecFriends, const SessionContext& stContext)
{

    // shimmeryang 支持好友上限2000
    // if (FillReqTransHeader(uiUin, 0x461, 1, &stContext) != 0)
    if (FillReqTransHeader(uiUin, 0x461, 30, &stContext) != 0)
    {
        return RET_FAIL;
    }

    //包体为空

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        unsigned short ushFriendNum = 0;
        iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushFriendNum);
        unsigned int uiFriendUin = 0;
        for (unsigned int i = 0; i < ushFriendNum; ++i)
        {
            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiFriendUin);
            vecFriends.push_back(uiFriendUin);
        }
        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

int COIDBProxyAPI::GetGroupInfo(unsigned uiUin, std::map<unsigned, GroupInfo> &mstQQFriendGroups, const SessionContext& stContext)
{
    // shimmeryang 2012-10-24 好友分组上限100
    // if (FillReqTransHeader(uiUin, 0x4b9, 1, &stContext) != 0)
    if (FillReqTransHeader(uiUin, 0x4b9, 100, &stContext) != 0)
    {
        return RET_FAIL;
    }

    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, static_cast<char> (0));
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, 0);

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    mstQQFriendGroups.clear();

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        PetLog(0, uiUin, PETLOG_TRACE, "%s|group info ok", __func__);
        // TODO: parse response
        unsigned int uiSequence = 0;
        iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiSequence);
        char chGroupNum = 0;
        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, chGroupNum);
        PetLog(0, uiUin, PETLOG_TRACE, "%s|group num %d", __func__, chGroupNum);
        char chGroupID = 0;
        char chGroupSortID = 0;
        char chGroupNameLen = 0;
        static char szGroupName[128] = { 0 };
        for (int i = 0; i < chGroupNum; ++i)
        {
            iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, chGroupID);
            iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, chGroupSortID);
            iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, chGroupNameLen);
            iOffset += CBuffTool::ReadString(m_stRspTransPkg.abyBody + iOffset, szGroupName, chGroupNameLen);
            szGroupName[static_cast<int> (chGroupNameLen)] = 0;

            PetLog(0, uiUin, PETLOG_TRACE, "%s|group id %d, sort id %d, name %s", __func__, chGroupID, chGroupSortID, szGroupName);

            GroupInfo stGroupInfo;
            stGroupInfo.uiGroupID = chGroupID;
            stGroupInfo.uiSortID = chGroupSortID;
            stGroupInfo.sGroupName = szGroupName;

            mstQQFriendGroups.insert(std::make_pair(chGroupID, stGroupInfo));
        }
        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

struct GroupListSorter
{
    bool operator()(const GroupedFriendList& first, const GroupedFriendList& second)
    {
        if (first.uiSortID <= second.uiSortID)
        {
            return true;
        }

        return false;
    }
};

#if 0
int COIDBProxyAPI::GetGroupedFriendList(unsigned int uiUin, std::vector<GroupedFriendList> &vecGroupedFriends, const SessionContext& stContext)
{
    // shimmeryang modify 20120703 在粉钻官网中，我新申请了一个拉取好友分组的信息，类型为33
    // if (FillReqTransHeader(uiUin, 0x462, 1, &stContext) != 0)
    if (FillReqTransHeader(uiUin, 0x462, 33, &stContext) != 0)
    {
        return RET_FAIL;
    }

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    vecGroupedFriends.clear();

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        unsigned short ushFriendNum = 0;
        std::map<unsigned, int> mGroupIdxByGroupID;
        iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushFriendNum);
        unsigned int uiFriendUin = 0;
        char chGroupID = 0;
        for (unsigned int i = 0; i < ushFriendNum; ++i)
        {
            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiFriendUin);
            iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, chGroupID);
            chGroupID = (chGroupID >> 2) & 0x0F;
            if (mGroupIdxByGroupID.find(chGroupID) == mGroupIdxByGroupID.end())
            {
                GroupedFriendList stGroupedFriendList;
                stGroupedFriendList.uiGroupID = chGroupID;
                stGroupedFriendList.vuiGroupMemberList.push_back(uiFriendUin);
                vecGroupedFriends.push_back(stGroupedFriendList);
                mGroupIdxByGroupID.insert(std::make_pair(chGroupID, vecGroupedFriends.size() - 1));
            }
            else
            {
                vecGroupedFriends[mGroupIdxByGroupID[chGroupID]].vuiGroupMemberList.push_back(uiFriendUin);
            }
        }
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }

    std::map<unsigned, GroupInfo> mstQQFriendGroups;
    if (GetGroupInfo(uiUin, mstQQFriendGroups, stContext) != 0)
    {
        return RET_FAIL;
    }

    for (size_t i = 0; i < vecGroupedFriends.size(); i++)
    {
        vecGroupedFriends[i].sGroupName = mstQQFriendGroups[vecGroupedFriends[i].uiGroupID].sGroupName;
        vecGroupedFriends[i].uiSortID = mstQQFriendGroups[vecGroupedFriends[i].uiGroupID].uiSortID;
    }

    std::sort(vecGroupedFriends.begin(), vecGroupedFriends.end(), GroupListSorter());

    return RET_OK;
}
#endif

// 2013年11月28日 21:55:38
// shimmeryang： 从企鹅那里拷贝一份过来使用
int COIDBProxyAPI::GetGroupedFriendList(unsigned int uiUin, std::vector<GroupedFriendList> &vecGroupedFriends, const SessionContext& stContext)
{
    // shimmeryang modify 20120703 在粉钻官网中，我新申请了一个拉取好友分组的信息，类型为33
    if (FillReqTransHeader(uiUin, 0x462, 33, &stContext) != 0)
    {
        return RET_FAIL;
    }

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    vecGroupedFriends.clear();

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        unsigned short ushFriendNum = 0;
        std::map<unsigned, int> mGroupIdxByGroupID;
        iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushFriendNum);
        unsigned int uiFriendUin = 0;
        unsigned short ushGroupID = 0;
        unsigned short ushGroupNum = 0;
        for (unsigned int i = 0; i < ushFriendNum; ++i)
        {
            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiFriendUin);
            // 接下来4字节信息表示权限设置，比如是否对我隐身，在此用不到，跳过4字节
            iOffset += 4;
            // id数量
            iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushGroupNum);

            for (unsigned int uCount = 0; uCount < ushGroupNum; ++uCount)
            {
                iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushGroupID);
                if (mGroupIdxByGroupID.find(ushGroupID) == mGroupIdxByGroupID.end())
                {
                    GroupedFriendList stGroupedFriendList;
                    stGroupedFriendList.uiGroupID = ushGroupID;
                    stGroupedFriendList.vuiGroupMemberList.push_back(uiFriendUin);
                    vecGroupedFriends.push_back(stGroupedFriendList);
                    mGroupIdxByGroupID.insert(std::make_pair(ushGroupID, vecGroupedFriends.size() - 1));
                }
                else
                {
                    vecGroupedFriends[mGroupIdxByGroupID[ushGroupID]].vuiGroupMemberList.push_back(uiFriendUin);
                }
            }
        }
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }

    std::map<unsigned, GroupInfo> mstQQFriendGroups;
    if (GetGroupInfo(uiUin, mstQQFriendGroups, stContext) != 0)
    {
        return RET_FAIL;
    }

    for (size_t i = 0; i < vecGroupedFriends.size(); i++)
    {
        if (mstQQFriendGroups[vecGroupedFriends[i].uiGroupID].sGroupName.empty())
        {
            vecGroupedFriends[i].sGroupName = "我的好友";
        }
        else
        {
            vecGroupedFriends[i].sGroupName = mstQQFriendGroups[vecGroupedFriends[i].uiGroupID].sGroupName;
        }

        vecGroupedFriends[i].uiSortID = mstQQFriendGroups[vecGroupedFriends[i].uiGroupID].uiSortID;
    }

    std::sort(vecGroupedFriends.begin(), vecGroupedFriends.end(), GroupListSorter());

    return RET_OK;
}

int COIDBProxyAPI::GetSimpleInfo(unsigned int uiUin, SQQSimpleInfo &stInfo)
{
    if (FillReqTransHeader(uiUin, 0x47, 0) != 0)
    {
        return RET_FAIL;
    }

    //包体为空

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, stInfo.uiUin);
        iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, stInfo.ushFace);
        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, stInfo.byAge);
        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, stInfo.byGender);
        unsigned char byNickLen = 0;
        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, byNickLen);
        byNickLen = byNickLen > OIDB_API_MAX_NICKNAME_LEN ? OIDB_API_MAX_NICKNAME_LEN : byNickLen;
        iOffset += CBuffTool::ReadString(m_stRspTransPkg.abyBody + iOffset, stInfo.szNick, byNickLen);
        stInfo.szNick[byNickLen] = 0;
        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

int COIDBProxyAPI::GetSimpleInfo(unsigned int uiUin, SQQSimpleInfo &stInfo, const SessionContext& stContext)
{
    if (FillReqTransHeader(uiUin, 0x480, 46, &stContext) != 0)
    {
        return RET_FAIL;
    }

    // 包体内容
    // 2013年12月02日 17:30:42
    // shimmeryang: 这些信息量不够我用啊，参照oidb的包结构填充一下里面的信息
    // 详细的文档在oidb.server.com 开发指南/使用指南/OIDB帮助文档列表  《TLV方式获取或设置用户信息Field定义.doc》
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, uiUin);
    char cDataCode = 0; // 0: utf-8 1: gbk
    m_stReqTransPkg.ushBodyLen +=
        CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, cDataCode);
    // 查询的字段
    short wFieldNum = 3;
    //性别, gender
    short ciGender = 20009;
    //FullAge
    short ciFullAge = 26005;
    //昵称, nick
    short ciNick = 20002;

    m_stReqTransPkg.ushBodyLen +=
        CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, wFieldNum);
    m_stReqTransPkg.ushBodyLen +=
        CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, ciGender);
    m_stReqTransPkg.ushBodyLen +=
        CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, ciFullAge);
    m_stReqTransPkg.ushBodyLen +=
        CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, ciNick);

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        memset(&stInfo, 0, sizeof(stInfo));

        iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, stInfo.uiUin);
        char cDataCode = 0;
        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, cDataCode);
        unsigned short ushTlvCount = 0;
        iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushTlvCount);

        if (ushTlvCount == 0 || ushTlvCount > 50)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg tlv_count[%d] is not valid", ushTlvCount);
            return RET_FAIL;
        }

        for (unsigned int i = 0; i < ushTlvCount; i++)
        {
            unsigned short ushFieldType = 0;
            unsigned short ushFieldLen = 0;
            iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushFieldType);
            iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushFieldLen);

            if (ushFieldLen == 0 || ushFieldLen > 4096)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg field_len[%d|%d] is not valid", i, ushFieldLen);
                return RET_FAIL;
            }

            printf("ushFieldLen|%d|ushFieldType|%d", ushFieldLen, ushFieldType);

            switch (ushFieldType)
            {
                case 26005:
                    {
                        if (ushFieldLen != 2)
                        {
                            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg fullage field field_len[%d|%d] is not valid", i, ushFieldLen);
                            return RET_FAIL;
                        }

                        unsigned short ushFullAge = 0;
                        iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushFullAge);
                        stInfo.byAge = ushFullAge & 0xF;
                        break;
                    }
                case 20009:
                    {
                        if (ushFieldLen != 1)
                        {
                            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg gender field field_len[%d|%d] is not valid", i, ushFieldLen);
                            return RET_FAIL;
                        }

                        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, stInfo.byGender);
                        break;
                    }
                case 20002:
                    {
                        if (ushFieldLen > 21)
                        {
                            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg nick field field_len[%d|%d] is not valid", i, ushFieldLen);
                            return RET_FAIL;
                        }

                        char szNickName[21];
                        memset(szNickName, 0, sizeof(szNickName));
                        iOffset += CBuffTool::ReadString(m_stRspTransPkg.abyBody + iOffset, szNickName, ushFieldLen);
                        snprintf(stInfo.szNick, sizeof(stInfo.szNick), "%s", szNickName);
                        break;
                    }
            }
        }

        unsigned short ushFieldLeft = 0;
        iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushFieldLeft);
        if (ushFieldLeft > 0)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg field_left[%d]", ushFieldLeft);
            return RET_FAIL;
        }

        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

int COIDBProxyAPI::GetRichFlag(unsigned int uiUin, unsigned char &byFlag, unsigned char byServiceType)
{
    if (FillReqTransHeader(uiUin, 0x66, byServiceType) != 0)
    {
        return RET_FAIL;
    }

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, byFlag);
        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

int COIDBProxyAPI::GetRichFlag(unsigned int uiUin, unsigned char &byFlag, unsigned char byServiceType, const SessionContext &stContext)
{
    std::vector<unsigned int> vecUinList;
    vecUinList.push_back(uiUin);

    std::map<unsigned int, unsigned char> mapFlags;

    int iRetVal = BatchGetRichFlag(uiUin, vecUinList, mapFlags, byServiceType, stContext);
    if (iRetVal != 0)
    {
        return iRetVal;
    }

    std::map<unsigned int, unsigned char>::iterator pOneFlag = mapFlags.find(uiUin);

    if (pOneFlag != mapFlags.end())
    {
        byFlag = pOneFlag->second;
    }
    else
    {
        byFlag = 0;
    }

    return 0;
}

int COIDBProxyAPI::SetRichFlag(unsigned int uiUin, unsigned char byFlag, unsigned char byServiceType)
{
    if (FillReqTransHeader(uiUin, 0x34, byServiceType) != 0)
    {
        return RET_FAIL;
    }

    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, byServiceType);
    char szDesc[32] = { 0 };
    char cDescLen = snprintf(szDesc, sizeof(szDesc), "%s:%s", "QQ宠物", m_szServiceName);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, cDescLen);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteString(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, szDesc, cDescLen);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, byFlag);

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }

    return RET_OK;
}

int COIDBProxyAPI::BatchGetRichFlag(unsigned int uiUin, const std::vector<unsigned int> &vecUinList, std::map<unsigned int, unsigned char> &mapFlags, unsigned char byServiceType)
{
    if (FillReqTransHeader(uiUin, 0x3d, byServiceType) != 0)
    {
        return RET_FAIL;
    }

    //包体内容
    unsigned int uiBatchCount = 500;
    unsigned int uiUinCount = vecUinList.size();
    for (unsigned int j = 0; j * uiBatchCount < uiUinCount; ++j)
    {
        unsigned int uiBegin = j * uiBatchCount;
        unsigned int uiEnd = uiBegin + uiBatchCount;

        if (uiEnd > uiUinCount)
        {
            uiEnd = uiUinCount;
        }

        m_stReqTransPkg.ushBodyLen = 0;

        for (unsigned int i = uiBegin; i < uiEnd; ++i)
        {
            m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, vecUinList[i]);
        }

        if (SendAndRecv() != 0)
        {
            return RET_FAIL;
        }

        int iOffset = 0;
        if (m_stRspTransPkg.stHeader.cResult == 0)
        {
            unsigned int uiFirst;
            unsigned char bySecond;
            for (unsigned int i = uiBegin; i < uiEnd; i++)
            {
                iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiFirst);
                iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, bySecond);
                mapFlags[uiFirst] = bySecond;
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return RET_FAIL;
        }
    }

    return RET_OK;
}

int COIDBProxyAPI::BatchGetRichFlag(unsigned int uiUin, const std::vector<unsigned int> &vecUinList, std::map<unsigned int, unsigned char> &mapFlags, unsigned char byServiceType, const SessionContext& stContext)
{
    if (FillReqTransHeader(uiUin, 0x43d, byServiceType, &stContext) != 0)
    {
        return RET_FAIL;
    }

    std::vector<unsigned int> vecUinListSorted(vecUinList);

    //包体内容
    std::sort(vecUinListSorted.begin(), vecUinListSorted.end());
    unsigned int uiBatchCount = 500;
    unsigned int uiUinCount = vecUinListSorted.size();
    for (unsigned int j = 0; j * uiBatchCount < uiUinCount; ++j)
    {
        unsigned int uiBegin = j * uiBatchCount;
        unsigned int uiEnd = uiBegin + uiBatchCount;

        if (uiEnd > uiUinCount)
        {
            uiEnd = uiUinCount;
        }

        m_stReqTransPkg.ushBodyLen = 0;
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short) (uiEnd - uiBegin));

        for (unsigned int i = uiBegin; i < uiEnd; ++i)
        {
            m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, vecUinListSorted[i]);
        }

        if (SendAndRecv() != 0)
        {
            return RET_FAIL;
        }

        int iOffset = 0;
        if (m_stRspTransPkg.stHeader.cResult == 0)
        {
            unsigned int uiNextUin = 0;
            unsigned short ushFieldCount = 0;

            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiNextUin);
            iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushFieldCount);

            if (uiNextUin != 0xffffffff)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "next uin is[%u]", uiNextUin);
                return RET_FAIL;
            }

            if (ushFieldCount == 0 || ushFieldCount > 500)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "field count[%d] is not valid", ushFieldCount);
                return RET_FAIL;
            }

            unsigned int uiFirst;
            unsigned char bySecond;
            for (unsigned int i = 0; i < ushFieldCount; i++)
            {
                iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiFirst);
                iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, bySecond);
                mapFlags[uiFirst] = bySecond;
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return RET_FAIL;
        }
    }

    return RET_OK;
}

int COIDBProxyAPI::BatchGetSimpleInfo(const std::vector<unsigned int> &vuiUin, std::map<unsigned int, SQQSimpleInfo> &mstQQSimpInfo)
{
    int iRetVal = 0;
    std::vector<SQQSimpleInfo> vstQQSimpInfo;

    iRetVal = BatchGetSimpleInfo(vuiUin, vstQQSimpInfo);
    if (iRetVal != 0)
    {
        return iRetVal;
    }

    mstQQSimpInfo.clear();
    std::vector<SQQSimpleInfo>::iterator pvstQQSimpleInfo = vstQQSimpInfo.begin();
    while (pvstQQSimpleInfo != vstQQSimpInfo.end())
    {
        mstQQSimpInfo.insert(std::pair<unsigned int, SQQSimpleInfo>(pvstQQSimpleInfo->uiUin, *pvstQQSimpleInfo));
        pvstQQSimpleInfo++;
    }

    return 0;
}

int COIDBProxyAPI::BatchGetSimpleInfo(const std::vector<unsigned int> &vuiUin, std::vector<SQQSimpleInfo> &vecInfo)
{
    if (vuiUin.empty())
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "uin vector empty");
        return RET_FAIL;
    }

    unsigned int uiUin = vuiUin[0];

    if (FillReqTransHeader(uiUin, 0x43, 0) != 0)
    {
        return RET_FAIL;
    }

    //包体内容
    unsigned int uiBatchCount = 50;
    unsigned int uiUinCount = vuiUin.size();
    for (unsigned int j = 0; j * uiBatchCount < uiUinCount; ++j)
    {
        unsigned int uiBegin = j * uiBatchCount;
        unsigned int uiEnd = uiBegin + uiBatchCount;

        if (uiEnd > uiUinCount)
        {
            uiEnd = uiUinCount;
        }

        m_stReqTransPkg.ushBodyLen = 0;

        for (unsigned int i = uiBegin; i < uiEnd; ++i)
        {
            m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, vuiUin[i]);
        }

        if (SendAndRecv() != 0)
        {
            return RET_FAIL;
        }

        int iOffset = 0;
        SQQSimpleInfo stInfo;
        if (m_stRspTransPkg.stHeader.cResult == 0)
        {
            iOffset += sizeof(short);
            for (unsigned int i = uiBegin; i < uiEnd; ++i)
            {
                iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, stInfo.uiUin);
                iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, stInfo.ushFace);
                iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, stInfo.byAge);
                iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, stInfo.byGender);
                unsigned char byNickLen = 0;
                iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, byNickLen);
                byNickLen = byNickLen > OIDB_API_MAX_NICKNAME_LEN ? OIDB_API_MAX_NICKNAME_LEN : byNickLen;
                iOffset += CBuffTool::ReadString(m_stRspTransPkg.abyBody + iOffset, stInfo.szNick, byNickLen);
                stInfo.szNick[byNickLen] = 0;
                // 跳过用户标志dwFlag字段
                iOffset += sizeof(long);

                vecInfo.push_back(stInfo);
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return RET_FAIL;
        }
    }

    return RET_OK;
}

bool cmp( unsigned int a, unsigned int b ) {
    return a > b;
}

int COIDBProxyAPI::BatchGetSimpleInfo(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, std::map<unsigned int, SQQSimpleInfo> &mstQQSimpInfo, const SessionContext &stContext)
{
    if (vuiUin.empty())
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "uin vector empty");
        return RET_FAIL;
    }

    if (FillReqTransHeader(uiUin, 0x49d, 15, &stContext) != 0)
    {
        return RET_FAIL;
    }

    std::vector<unsigned int> vuiUinSorted(vuiUin);

    // 对uin列表进行排序
    std::sort(vuiUinSorted.begin(), vuiUinSorted.end());

    //包体内容
    const unsigned int cuiBatchCount = 100;
    //昵称不截断
    const char cNickCut = 0;
    //wFieldNum
    const short wFieldNum = 3;
    //昵称
    const short wNick = 20002;
    //性别
    const short wGender = 20009;
    //年龄
    const short wAge = 20037;
    unsigned int uiUinCount = vuiUinSorted.size();
    for (unsigned int j = 0; j * cuiBatchCount < uiUinCount; ++j)
    {
        unsigned int uiBegin = j * cuiBatchCount;
        unsigned int uiEnd = uiBegin + cuiBatchCount;

        if (uiEnd > uiUinCount)
        {
            uiEnd = uiUinCount;
        }

        m_stReqTransPkg.ushBodyLen = 0;

        m_stReqTransPkg.ushBodyLen +=
            CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (char)cNickCut);
        m_stReqTransPkg.ushBodyLen +=
            CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short)wFieldNum);
        m_stReqTransPkg.ushBodyLen +=
            CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short)wNick);
        m_stReqTransPkg.ushBodyLen +=
            CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short)wGender);
        m_stReqTransPkg.ushBodyLen +=
            CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short)wAge);
        //wUinCount
        m_stReqTransPkg.ushBodyLen +=
            CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short)(uiEnd - uiBegin));
        for (unsigned int i = uiBegin; i < uiEnd; ++i)
        {
            m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, vuiUinSorted[i]);
        }

        if (SendAndRecv() != 0)
        {
            return RET_FAIL;
        }

        int iOffset = 0;
        SQQSimpleInfo stInfo;
        if (m_stRspTransPkg.stHeader.cResult == 0)
        {
            char cNickCut = 0;
            unsigned int uiNextUin = 0;
            unsigned short ushSimpleInfoNum = 0;
            iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, cNickCut);
            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiNextUin);
            iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushSimpleInfoNum);

            if (uiNextUin != 0xffffffff)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "next_uin[%u] is not valid", uiNextUin);
                return RET_FAIL;
            }

            if (ushSimpleInfoNum > 500)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "info_num[%d] is not valid", ushSimpleInfoNum);
                return RET_FAIL;
            }

            for (unsigned int k = 0; k < ushSimpleInfoNum; ++k)
            {
                memset(&stInfo, 0, sizeof(stInfo));

                unsigned int uiCurUin = 0;
                unsigned short ushCurFieldNum = 0;
                iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiCurUin);
                iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushCurFieldNum);

                if (ushCurFieldNum > 100)
                {
                    snprintf(m_szErrMsg, sizeof(m_szErrMsg), "field_num[%d|%d] is not valid", k, ushCurFieldNum);
                    return RET_FAIL;
                }

                for(unsigned int l=0; l<ushCurFieldNum; l++)
                {
                    unsigned short ushFieldType = 0;
                    unsigned short ushFieldLen = 0;
                    iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushFieldType);
                    iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, ushFieldLen);

                    if (ushFieldLen > 4096)
                    {
                        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg field_len[%d|%d] is not valid", l, ushFieldLen);
                        return RET_FAIL;
                    }

                    if (ushFieldType == 20009)
                    {
                        //性别
                        if (ushFieldLen != 1)
                        {
                            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg face agender field_len[%d|%d] is not valid", l, ushFieldLen);
                            return RET_FAIL;
                        }

                        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, stInfo.byGender);
                    }

                    if (ushFieldType == 20037)
                    {
                        //年龄
                        if (ushFieldLen != 1)
                        {
                            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg age field field_len[%d|%d] is not valid", l, ushFieldLen);
                            return RET_FAIL;
                        }

                        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, stInfo.byAge);
                    }

                    if (ushFieldType == 20002)
                    {
                        if (ushFieldLen > 64)
                        {
                            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg nick field field_len[%d|%d] is not valid", l, ushFieldLen);
                            return RET_FAIL;
                        }

                        char szNickNameUTF8[64];
                        char szNickNameGBK[64];
                        // size_t NickNameGBKLen = sizeof(szNickNameGBK);
                        memset(szNickNameUTF8, 0, sizeof(szNickNameUTF8));
                        memset(szNickNameGBK, 0, sizeof(szNickNameGBK));
                        iOffset += CBuffTool::ReadString(m_stRspTransPkg.abyBody + iOffset, szNickNameUTF8, ushFieldLen);
                        // int iRetVal = CStrTool::U2G(szNickNameUTF8, ushFieldLen, szNickNameGBK, NickNameGBKLen);
                        // if (iRetVal != 0)
                        // {
                        //     snprintf(m_szErrMsg, sizeof(m_szErrMsg), "conv utf8 to gbk failed, uin=%u", uiCurUin);
                        //     return RET_FAIL;
                        // }
                        // CStrTool::CheckValidNameGBK(szNickNameGBK, szNickNameUTF8);
                        snprintf(stInfo.szNick, sizeof(stInfo.szNick), "%s", szNickNameUTF8);
                    }
                }

                stInfo.uiUin = uiCurUin;
                mstQQSimpInfo[uiCurUin] = stInfo;
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return RET_FAIL;
        }
    }

    return RET_OK;
}

int COIDBProxyAPI::CheckFriend(unsigned int uiUin, unsigned int uiFriendUin)
{
    if (uiFriendUin <= OIDB_API_MIN_UIN)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "friend(%u) error", uiFriendUin);
        return -1;
    }

    if (FillReqTransHeader(uiUin, 0x31, 0) != 0)
    {
        return RET_FAIL;
    }

    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, uiFriendUin);

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        return RET_IS_FRIEND;
    }
    else if (m_stRspTransPkg.stHeader.cResult == 1)
    {
        return RET_NOT_FRIEND;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

int COIDBProxyAPI::CheckFriend(unsigned int uiUin, unsigned int uiFriendUin, const SessionContext &stContext)
{
    if (uiFriendUin <= OIDB_API_MIN_UIN)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "friend(%u) error", uiFriendUin);
        return -1;
    }

    if (FillReqTransHeader(uiUin, 0x431, 0, &stContext) != 0)
    {
        return RET_FAIL;
    }

    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, uiFriendUin);

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    unsigned int uiRetFriendUin = 0;
    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiRetFriendUin);
        if (uiFriendUin == uiRetFriendUin)
        {
            return RET_IS_FRIEND;
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "rsp_friend_uin[%u] is not equ req_friend_uin[%u]", uiRetFriendUin, uiFriendUin);
            return RET_FAIL;
        }
    }
    else if (m_stRspTransPkg.stHeader.cResult == 1)
    {
        return RET_NOT_FRIEND;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

int COIDBProxyAPI::GetMssFlag(unsigned int uiUin, unsigned short ushMssType, char &cMssValue)
{
    if (FillReqTransHeader(uiUin, 0x97, ushMssType) != 0)
    {
        return RET_FAIL;
    }

    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, uiUin);

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        unsigned int uiRetUin;
        char cRetVal;

        iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiRetUin);
        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, cRetVal);

        if (uiRetUin == uiUin)
        {
            cMssValue = cRetVal;
            return RET_OK;
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return uin [%d] <> request uin [%d]", uiRetUin, uiUin);
            return RET_FAIL;
        }
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

int COIDBProxyAPI::GetMssFlag(unsigned int uiUin, unsigned short ushMssType, char &cMssValue, const SessionContext& stContext)
{
    int iRetVal = 0;
    std::vector<unsigned int> vuiUin;
    std::map<unsigned int, QQMssInfo> mAllMssInfo;

    vuiUin.push_back(uiUin);

    iRetVal = BatchGetMssFlag(uiUin, vuiUin, ushMssType, mAllMssInfo, stContext);
    if (iRetVal != 0)
    {
        return iRetVal;
    }

    std::map<unsigned int, QQMssInfo>::iterator pCurMssInfo = mAllMssInfo.find(uiUin);
    if (pCurMssInfo == mAllMssInfo.end())
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "uin[%u] is not in ret map", uiUin);
        return RET_FAIL;
    }

    cMssValue = pCurMssInfo->second.cMssValue;

    return RET_OK;
}

int COIDBProxyAPI::SetMssFlag(unsigned int uiUin, unsigned short ushMssType, char cMssValue)
{

    if (FillReqTransHeader(uiUin, 0x94, ushMssType) != 0)
    {
        return RET_FAIL;
    }

    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, cMssValue);

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }

    return RET_OK;
}

int COIDBProxyAPI::BatchGetMssFlag(const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::map<unsigned int, QQMssInfo> &mstQQMssInfo)
{
    int iRetVal = 0;
    std::vector<QQMssInfo> vstQQMssinfo;

    iRetVal = BatchGetMssFlag(vuiUin, ushMssType, vstQQMssinfo);
    if (iRetVal != 0)
    {
        return iRetVal;
    }

    mstQQMssInfo.clear();
    std::vector<QQMssInfo>::iterator pvstQQMssInfo = vstQQMssinfo.begin();
    while (pvstQQMssInfo != vstQQMssinfo.end())
    {
        mstQQMssInfo.insert(std::pair<unsigned int, QQMssInfo>(pvstQQMssInfo->uiUin, *pvstQQMssInfo));
        pvstQQMssInfo++;
    }

    return 0;
}

int COIDBProxyAPI::BatchGetMssFlag(const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::vector<QQMssInfo> &vstQQMssInfo)
{
    if (vuiUin.empty())
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "uin vector empty");
        return RET_FAIL;
    }

    unsigned int uiUin = vuiUin[0];

    if (FillReqTransHeader(uiUin, 0x98, ushMssType) != 0)
    {
        return RET_FAIL;
    }

    //包体内容
    unsigned int uiBatchCount = 50;
    unsigned int uiUinCount = vuiUin.size();
    for (unsigned int j = 0; j * uiBatchCount < uiUinCount; ++j)
    {
        unsigned int uiBegin = j * uiBatchCount;
        unsigned int uiEnd = uiBegin + uiBatchCount;

        if (uiEnd > uiUinCount)
        {
            uiEnd = uiUinCount;
        }

        m_stReqTransPkg.ushBodyLen = 0;

        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (char) (uiEnd - uiBegin));

        for (unsigned int i = uiBegin; i < uiEnd; ++i)
        {
            m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, vuiUin[i]);
        }

        if (SendAndRecv() != 0)
        {
            return RET_FAIL;
        }

        int iOffset = 0;
        if (m_stRspTransPkg.stHeader.cResult == 0)
        {
            unsigned int uiNextUin = 0;
            char cDoCount = 0;

            iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, cDoCount);
            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiNextUin);

            if (uiNextUin != 0)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "next uin is[%u]", uiNextUin);
                return RET_FAIL;
            }

            if (cDoCount == 0 || cDoCount > 50)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "field count[%d] is not valid", cDoCount);
                return RET_FAIL;
            }

            unsigned int uiFirst;
            QQMssInfo stCurMssInfo;
            for (int i = 0; i < cDoCount; i++)
            {
                iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiFirst);
                iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, stCurMssInfo.cMssValue);
                stCurMssInfo.uiUin = uiFirst;
                stCurMssInfo.ushMssType = ushMssType;
                vstQQMssInfo.push_back(stCurMssInfo);
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return RET_FAIL;
        }
    }

    return RET_OK;
}

int COIDBProxyAPI::BatchGetMssFlag(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::map<unsigned int, QQMssInfo> &mstQQMssInfo, const SessionContext& stContext)
{
    if (vuiUin.empty())
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "uin vector empty");
        return RET_FAIL;
    }

    if (FillReqTransHeader(uiUin, 0x498, ushMssType, &stContext) != 0)
    {
        return RET_FAIL;
    }

    //包体内容
    unsigned int uiBatchCount = 50;
    unsigned int uiUinCount = vuiUin.size();
    for (unsigned int j = 0; j * uiBatchCount < uiUinCount; ++j)
    {
        unsigned int uiBegin = j * uiBatchCount;
        unsigned int uiEnd = uiBegin + uiBatchCount;

        if (uiEnd > uiUinCount)
        {
            uiEnd = uiUinCount;
        }

        m_stReqTransPkg.ushBodyLen = 0;

        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (char) (uiEnd - uiBegin));

        for (unsigned int i = uiBegin; i < uiEnd; ++i)
        {
            m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, vuiUin[i]);
        }

        if (SendAndRecv() != 0)
        {
            return RET_FAIL;
        }

        int iOffset = 0;
        if (m_stRspTransPkg.stHeader.cResult == 0)
        {
            unsigned int uiNextUin = 0;
            char cDoCount = 0;

            iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, cDoCount);
            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiNextUin);

            if (uiNextUin != 0)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "next uin is[%u]", uiNextUin);
                return RET_FAIL;
            }

            if (cDoCount == 0 || cDoCount > 50)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "field count[%d] is not valid", cDoCount);
                return RET_FAIL;
            }

            unsigned int uiFirst;
            QQMssInfo stCurMssInfo;
            for (int i = 0; i < cDoCount; i++)
            {
                iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiFirst);
                iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, stCurMssInfo.cMssValue);
                stCurMssInfo.uiUin = uiFirst;
                stCurMssInfo.ushMssType = ushMssType;
                mstQQMssInfo[uiFirst] = stCurMssInfo;
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return RET_FAIL;
        }
    }

    return RET_OK;
}


int COIDBProxyAPI::BatchGetAllMssFlag(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, std::map<unsigned int, QQAllMssInfo> &mstAllQQMssInfo, const SessionContext& stContext)
{
    if (vuiUin.empty())
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "uin vector empty");
        return RET_FAIL;
    }

    //2:获取多个用户
    if (FillReqTransHeader(uiUin, 0x499, 2, &stContext) != 0)
    {
        return RET_FAIL;
    }

    //包体内容
    unsigned int uiBatchCount = 50;
    unsigned int uiUinCount = vuiUin.size();
    for (unsigned int j = 0; j * uiBatchCount < uiUinCount; ++j)
    {
        unsigned int uiBegin = j * uiBatchCount;
        unsigned int uiEnd = uiBegin + uiBatchCount;

        if (uiEnd > uiUinCount)
        {
            uiEnd = uiUinCount;
        }

        m_stReqTransPkg.ushBodyLen = 0;

        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (char) (uiEnd - uiBegin));

        for (unsigned int i = uiBegin; i < uiEnd; ++i)
        {
            m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, vuiUin[i]);
        }

        if (SendAndRecv() != 0)
        {
            return RET_FAIL;
        }

        int iOffset = 0;
        if (m_stRspTransPkg.stHeader.cResult == 0)
        {
            unsigned int uiNextUin = 0;
            char cDoCount = 0;

            iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, cDoCount);
            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiNextUin);

            if (uiNextUin != 0)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "next uin is[%u]", uiNextUin);
                return RET_FAIL;
            }

            if (cDoCount == 0 || cDoCount > 50)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "field count[%d] is not valid", cDoCount);
                return RET_FAIL;
            }

            QQAllMssInfo stQQAllMssInfo;
            for (int i = 0; i < cDoCount; i++)
            {
                iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, stQQAllMssInfo.uiUin);
                iOffset += CBuffTool::ReadString(m_stRspTransPkg.abyBody + iOffset, stQQAllMssInfo.szMssValue, sizeof(stQQAllMssInfo.szMssValue));
                mstAllQQMssInfo[stQQAllMssInfo.uiUin] = stQQAllMssInfo;
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return RET_FAIL;
        }
    }


    return RET_OK;
}

/*
 int COIDBProxyAPI::GetMssFlagMulti(unsigned int uiUin, QQAllMssInfo &stQQAllMssInfo)
 {


 memset(&m_stHeader, 0, sizeof(m_stHeader));
 m_stHeader.uiUin = uiUin;
 m_stHeader.ushCommand = 0x99;
 m_stHeader.byResult = 0;
 m_stHeader.byServiceType = 1;
 m_stHeader.ushExtraFlag = 0;

 m_uiRequestLen = OIDB_API_PROTO_HEADER_LEN;

 m_uiRequestLen += CBuffTool::WriteByte(m_abyRequest + m_uiRequestLen, (char)1);
 m_uiRequestLen += CBuffTool::WriteInt(m_abyRequest + m_uiRequestLen, uiUin);

 m_stHeader.ushLength = m_uiRequestLen;

 PackHeader(m_abyRequest, m_stHeader);

 if (SendAndRecvOld() != 0)
 {
 return RET_FAIL;
 }

 int iOffset = OIDB_API_PROTO_HEADER_LEN;
 if (m_stHeader.byResult == 0)
 {
 char cDoCount = 0;
 unsigned int uiNextBeginUin = 0;

 iOffset += CBuffTool::ReadByte(m_abyResponse + iOffset, cDoCount);
 iOffset += CBuffTool::ReadInt(m_abyResponse + iOffset, uiNextBeginUin);

 if (cDoCount != 1)
 {
 snprintf(m_szErrMsg, sizeof(m_szErrMsg), "req_uin_cout [%d] is not equ rsp_uin_count [%d], next_begin_uin=%d", 1, cDoCount, uiNextBeginUin);
 return RET_FAIL;
 }

 iOffset += CBuffTool::ReadInt(m_abyResponse + iOffset, stQQAllMssInfo.uiUin);
 iOffset += CBuffTool::ReadString(m_abyResponse + iOffset, stQQAllMssInfo.szMssValue, sizeof(stQQAllMssInfo.szMssValue));

 return RET_OK;
 }
 else
 {
 snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stHeader.byResult);
 return RET_FAIL;
 }
 }
 */

int COIDBProxyAPI::GetRichFlagLevel(unsigned int uiUin, unsigned char byServiceType, unsigned char &byLevel)
{
    if (FillReqTransHeader(uiUin, 0x76, byServiceType) != 0)
    {
        return RET_FAIL;
    }

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, byLevel);
        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

int COIDBProxyAPI::GetRichFlagLevel(unsigned int uiUin, unsigned char byServiceType, unsigned char &byLevel, const SessionContext& stContext)
{
    int iRetVal = 0;
    std::vector<unsigned int> vuiUin;
    std::map<unsigned int, unsigned char> mRichFlagLevel;

    vuiUin.push_back(uiUin);

    iRetVal = BatchGetRichFlagLevel(uiUin, vuiUin, byServiceType, mRichFlagLevel, stContext);
    if (iRetVal != 0)
    {
        return iRetVal;
    }

    std::map<unsigned int, unsigned char>::iterator pCurFlag = mRichFlagLevel.find(uiUin);
    if (pCurFlag == mRichFlagLevel.end())
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "uin[%u] is not in ret map", uiUin);
        return RET_FAIL;
    }

    byLevel = pCurFlag->second;

    return RET_OK;
}

int COIDBProxyAPI::BatchGetRichFlagLevel(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, unsigned char byServiceType, std::map<unsigned int, unsigned char> &mRichFlagLevel, const SessionContext& stContext)
{
    if (FillReqTransHeader(uiUin, 0x476, byServiceType, &stContext) != 0)
    {
        return RET_FAIL;
    }

    //包体内容
    unsigned int uiBatchCount = 100;
    unsigned int uiUinCount = vuiUin.size();
    for (unsigned int j = 0; j * uiBatchCount < uiUinCount; ++j)
    {
        unsigned int uiBegin = j * uiBatchCount;
        unsigned int uiEnd = uiBegin + uiBatchCount;

        if (uiEnd > uiUinCount)
        {
            uiEnd = uiUinCount;
        }

        m_stReqTransPkg.ushBodyLen = 0;

        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short) (uiEnd - uiBegin));

        for (unsigned int i = uiBegin; i < uiEnd; ++i)
        {
            m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, vuiUin[i]);
        }

        if (SendAndRecv() != 0)
        {
            return RET_FAIL;
        }

        int iOffset = 0;
        if (m_stRspTransPkg.stHeader.cResult == 0)
        {
            unsigned int uiNextUin = 0;
            short shDoCount = 0;

            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiNextUin);
            iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, shDoCount);

            if (uiNextUin != 0xFFFFFFFF)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "next uin is[%u]", uiNextUin);
                return RET_FAIL;
            }

            if (shDoCount == 0 || shDoCount > 100)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "field count[%d] is not valid", shDoCount);
                return RET_FAIL;
            }

            unsigned int uiCurUin;
            char cCurVal;
            for (int i = 0; i < shDoCount; i++)
            {
                iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiCurUin);
                iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, cCurVal);
                mRichFlagLevel[uiCurUin] = cCurVal;
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return RET_FAIL;
        }
    }

    return RET_OK;
}


int COIDBProxyAPI::SetRichFlagLevel(unsigned int uiUin, unsigned char byServiceType, unsigned char byLevel)
{
    if (FillReqTransHeader(uiUin, 0x75, byServiceType) != 0)
    {
        return RET_FAIL;
    }

    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, byServiceType);
    char szDesc[32] = { 0 };
    char cDescLen = snprintf(szDesc, sizeof(szDesc), "%s:%s", "QQ宠物", m_szServiceName);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, cDescLen);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteString(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, szDesc, cDescLen);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, byLevel);

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }

    return RET_OK;

}

int COIDBProxyAPI::GetRichFlag2(unsigned int uiUin, unsigned char &byFlag, unsigned char byServiceType)
{

    if (FillReqTransHeader(uiUin, 0x9d, byServiceType) != 0)
    {
        return RET_FAIL;
    }

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, byFlag);
        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

int COIDBProxyAPI::SetRichFlag2(unsigned int uiUin, unsigned char byFlag, unsigned char byServiceType)
{
    if (FillReqTransHeader(uiUin, 0x9f, byServiceType) != 0)
    {
        return RET_FAIL;
    }

    m_stReqTransPkg.ushBodyLen = 0;

    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, byServiceType);
    char szDesc[32] = { 0 };
    char cDescLen = snprintf(szDesc, sizeof(szDesc), "%s:%s", "QQ宠物", m_szServiceName);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, cDescLen);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteString(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, szDesc, cDescLen);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, byFlag);

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    if (m_stRspTransPkg.stHeader.cResult == 0)
    {
        return RET_OK;
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }

    return RET_OK;
}

int COIDBProxyAPI::GetRemarkName(unsigned int uUin, std::map<unsigned int, std::string> &mapRemarkName, const SessionContext& stContext)
{
    if (FillReqTransHeader(uUin, 0x69a, 0, &stContext) != 0)
    {
        return RET_FAIL;
    }

    char cEndFlag = 0;
    unsigned int uNextUin = 0;
    unsigned int uUpdateTime = 0;

    while (true)
    {
        m_stReqTransPkg.ushBodyLen = 0;

        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, uNextUin);
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, uUpdateTime);

        if (SendAndRecv() != 0)
        {
            return RET_FAIL;
        }

        if (m_stRspTransPkg.stHeader.cResult != 0)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return RET_FAIL;
        }

        int nOffSet = 0;
        nOffSet += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + nOffSet, cEndFlag);
        nOffSet += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + nOffSet, uNextUin);
        nOffSet += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + nOffSet, uUpdateTime);

        while (nOffSet < m_stRspTransPkg.ushBodyLen)
        {
            unsigned int uFriendUin = 0;
            unsigned char uchRemarkLen = 0;
            char szRemark[256] = {0};
            nOffSet += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + nOffSet, uFriendUin);
            nOffSet += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + nOffSet, uchRemarkLen);
            nOffSet += CBuffTool::ReadString(m_stRspTransPkg.abyBody + nOffSet, szRemark, uchRemarkLen);
            szRemark[sizeof(szRemark) - 1] = '\0';

            mapRemarkName[uFriendUin] = szRemark;
        }

        if (cEndFlag == 1)
        {
            break;
        }
    }

    return RET_OK;
}



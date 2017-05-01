#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <iconv.h>

#include "api/sns_oidb_api/sns_oidb_api.h"
#include "comm/util/pet_util.h"
#include "comm/ini_file/ini_file.h"
#include "comm/log/pet_log.h"

using namespace snslib;
using namespace snslib::oidb2;

#define OIDB_API_MIN_UIN 10001

CSnsOIDBProxyAPI::CSnsOIDBProxyAPI() :
    m_bInit(false), m_iTimeout(0), m_uiRequestLen(0), m_uiResponseLen(0)
{
    memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
    memset(m_abyRequest, 0, sizeof(m_abyRequest));
    memset(m_abyResponse, 0, sizeof(m_abyResponse));

    m_iOIDBProxyNum = 0;
}

CSnsOIDBProxyAPI::~CSnsOIDBProxyAPI()
{
}

int CSnsOIDBProxyAPI::Init(const char *pszFile, const char *pszServiceName)
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
        char szServerIP[OIDB_API_MAX_IP_LEN + 1];
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

            strncpy(m_astProxyConf[i].szHost, szServerIP, OIDB_API_MAX_IP_LEN);
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

int CSnsOIDBProxyAPI::SendAndRecv()
{
  int ret = SendAndRecvOnce();
  if (ret)
  {
    ret = SendAndRecvOnce();
  }

  return ret;
}

int CSnsOIDBProxyAPI::SendAndRecvOnce()
{
    int iRetVal = 0;

    m_uiRequestLen = PackTransPkg(m_abyRequest, m_stReqTransPkg);
    m_uiResponseLen = sizeof(m_abyResponse);

    //接收包头
    if ((iRetVal = m_oTcpClient.SendAndRecv(m_abyRequest, m_uiRequestLen, m_abyResponse, &m_uiResponseLen, (1 + sizeof(STransPkgHeader)))) != 0)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "SendAndRecv pkg header: %s", m_oTcpClient.GetErrMsg());
        PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "send in send_recv faild, ret=%d, errmsg=%s", iRetVal, m_oTcpClient.GetErrMsg());
        m_oTcpClient.CloseCurConn();

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
    unsigned short ushRetPkgLen = ntohs(*(unsigned short *) pstTransHeader->abyLength);
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
        PetLog(0, ntohl(*(unsigned int *)m_stReqTransPkg.stHeader.abyUin), PETLOG_WARN, "recv in send_recv faild, ret=%d, errmsg=%s", iRetVal, m_oTcpClient.GetErrMsg());
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
int CSnsOIDBProxyAPI::SendAndRecvOld()
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

int CSnsOIDBProxyAPI::PackTransPkg(char * pMem, const STransPkg &stPkg)
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

int CSnsOIDBProxyAPI::UnPackTransPkg(const char * pMem, unsigned uiLen, STransPkg& stPkg)
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

int CSnsOIDBProxyAPI::FillReqTransHeader(unsigned int uiUin, unsigned short ushCmdID, char cServiceType, const SessionContext *pstContext /*= NULL*/)
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

int CSnsOIDBProxyAPI::PackHeader(char * pMem, const SOIDBProtoHeader &stHeader)
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

int CSnsOIDBProxyAPI::UnPackHeader(const char * pMem, SOIDBProtoHeader &stHeader)
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

int CSnsOIDBProxyAPI::UnPackExtraHeader(const char * pMem, SOIDBProtoHeader &stHeader)
{
    int iOffset = 0;
    iOffset += CBuffTool::ReadInt(pMem + iOffset, stHeader.uiAppID);
    iOffset += CBuffTool::ReadByte(pMem + iOffset, stHeader.chKeyType);
    iOffset += CBuffTool::ReadShort(pMem + iOffset, stHeader.ushSessionKeyLen);
    iOffset += CBuffTool::ReadString(pMem + iOffset, stHeader.szSessionKey, stHeader.ushSessionKeyLen);
    return iOffset;
}

int CSnsOIDBProxyAPI::GetFriendList(unsigned int uiUin, std::vector<unsigned int> &vecFriends)
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

int CSnsOIDBProxyAPI::GetFriendList(unsigned int uiUin, std::vector<unsigned int> &vecFriends, const SessionContext& stContext)
{

    if (FillReqTransHeader(uiUin, 0x461, 1, &stContext) != 0)
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
        return m_stRspTransPkg.stHeader.cResult;
    }
}

int CSnsOIDBProxyAPI::GetGroupInfo(unsigned uiUin, std::map<unsigned, GroupInfo> &mstQQFriendGroups, const SessionContext& stContext)
{
    if (FillReqTransHeader(uiUin, 0x4b9, 1, &stContext) != 0)
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
        return m_stRspTransPkg.stHeader.cResult;
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

int CSnsOIDBProxyAPI::GetGroupedFriendList(unsigned int uiUin, std::vector<GroupedFriendList> &vecGroupedFriends, const SessionContext& stContext)
{
    if (FillReqTransHeader(uiUin, 0x462, 1, &stContext) != 0)
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
                stGroupedFriendList.vGroupMemberList.push_back(uiFriendUin);
                vecGroupedFriends.push_back(stGroupedFriendList);
                mGroupIdxByGroupID.insert(std::make_pair(chGroupID, vecGroupedFriends.size() - 1));
            }
            else
            {
                vecGroupedFriends[mGroupIdxByGroupID[chGroupID]].vGroupMemberList.push_back(uiFriendUin);
            }
        }
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return m_stRspTransPkg.stHeader.cResult;
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

int CSnsOIDBProxyAPI::GetSimpleInfo(unsigned int uiUin, SQQSimpleInfo &stInfo)
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

int CSnsOIDBProxyAPI::GetSimpleInfo(unsigned int uiUin, SQQSimpleInfo &stInfo, const SessionContext& stContext)
{
    if (FillReqTransHeader(uiUin, 0x480, 46, &stContext) != 0)
    {
        return RET_FAIL;
    }

    //包体内容
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, uiUin);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (char) 1);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short) 2); //需要获取两个字段
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short) 20015); //个性头像
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short) 20002); //昵称

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

            if (ushFieldType == 20015)
            {
                //个性头像
                if (ushFieldLen != 2)
                {
                    snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg face field field_len[%d|%d] is not valid", i, ushFieldLen);
                    return RET_FAIL;
                }

                iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, stInfo.ushFace);
            }

            if (ushFieldType == 20002)
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

int CSnsOIDBProxyAPI::BatchGetSimpleInfo(const std::vector<unsigned int> &vuiUin, std::map<unsigned int, SQQSimpleInfo> &mstQQSimpInfo)
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

int CSnsOIDBProxyAPI::BatchGetSimpleInfo(const std::vector<unsigned int> &vuiUin, std::vector<SQQSimpleInfo> &vecInfo)
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

int CSnsOIDBProxyAPI::BatchGetSimpleInfo(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, std::vector<SQQSimpleInfo> &vstQQSimpleInfo, const SessionContext &stContext)
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

        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (char)0);  //cNickCut 昵称絥ot囟希畛?6字节
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short)2);  //wFieldNum
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short) 20015); //个性头像
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short) 20002); //昵称
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short)(uiEnd - uiBegin));  //wUinCount
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

                    if (ushFieldType == 20015)
                    {
                        //个性头像
                        if (ushFieldLen != 2)
                        {
                            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg face field field_len[%d|%d] is not valid", l, ushFieldLen);
                            return RET_FAIL;
                        }

                        iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, stInfo.ushFace);
                    }

                    if (ushFieldType == 20002)
                    {
                        if (ushFieldLen > 64)
                        {
                            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg nick field field_len[%d|%d] is not valid", l, ushFieldLen);
                            return RET_FAIL;
                        }

                        iOffset += CBuffTool::ReadString(m_stRspTransPkg.abyBody + iOffset, stInfo.szNick, ushFieldLen);
                    }
                }

                stInfo.uiUin = uiCurUin;
                vstQQSimpleInfo.push_back(stInfo);
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return m_stRspTransPkg.stHeader.cResult;
        }
    }

    return RET_OK;
}

int CSnsOIDBProxyAPI::BatchGetSimpleInfo(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, std::map<unsigned int, SQQSimpleInfo> &mstQQSimpInfo, const SessionContext &stContext)
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

        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (char)0);  //cNickCut 昵称截断，最长18字节
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short)2);  //wFieldNum
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short) 20015); //个性头像
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short) 20002); //昵称
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteShort(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (short)(uiEnd - uiBegin));  //wUinCount
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

                    if (ushFieldType == 20015)
                    {
                        //个性头像
                        if (ushFieldLen != 2)
                        {
                            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg face field field_len[%d|%d] is not valid", l, ushFieldLen);
                            return RET_FAIL;
                        }

                        iOffset += CBuffTool::ReadShort(m_stRspTransPkg.abyBody + iOffset, stInfo.ushFace);
                    }

                    if (ushFieldType == 20002)
                    {
                        if (ushFieldLen > 64)
                        {
                            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "return pkg nick field field_len[%d|%d] is not valid", l, ushFieldLen);
                            return RET_FAIL;
                        }

                        iOffset += CBuffTool::ReadString(m_stRspTransPkg.abyBody + iOffset, stInfo.szNick, ushFieldLen);
                    }
                }

                stInfo.uiUin = uiCurUin;
                mstQQSimpInfo[uiCurUin] = stInfo;
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return m_stRspTransPkg.stHeader.cResult;
        }
    }

    return RET_OK;
}

int CSnsOIDBProxyAPI::CheckFriend(unsigned int uiUin, unsigned int uiFriendUin)
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

int CSnsOIDBProxyAPI::CheckFriend(unsigned int uiUin, unsigned int uiFriendUin, const SessionContext &stContext)
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
        return m_stRspTransPkg.stHeader.cResult;
    }
}

int CSnsOIDBProxyAPI::GetMssFlag(unsigned int uiUin, unsigned short ushMssType, char &cMssValue)
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

int CSnsOIDBProxyAPI::GetMssFlag(unsigned int uiUin, unsigned short ushMssType, char &cMssValue, const SessionContext& stContext)
{
    if (uiUin < OIDB_API_MIN_UIN)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "uin(%u) error", uiUin);
        return RET_FAIL;
    }

    std::vector<unsigned> vuiUin;
    vuiUin.push_back(uiUin);
    std::vector<QQMssInfo> vstQQMssInfo;
                                    
    int iRet = BatchGetMssFlag(uiUin, vuiUin, ushMssType, vstQQMssInfo, stContext);
    if (iRet) return iRet;
                                          
    cMssValue = vstQQMssInfo[0].cMssValue;
    return 0;
}

int CSnsOIDBProxyAPI::BatchGetMssFlag(const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::map<unsigned int, QQMssInfo> &mstQQMssInfo)
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

int CSnsOIDBProxyAPI::BatchGetMssFlag(const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::vector<QQMssInfo> &vstQQMssInfo)
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

int CSnsOIDBProxyAPI::BatchGetMssFlag(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::vector<QQMssInfo> &vstQQMssInfo, const SessionContext& stContext)
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
                vstQQMssInfo.push_back(stCurMssInfo);
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return m_stRspTransPkg.stHeader.cResult;
        }
    }

    return RET_OK;
}

int CSnsOIDBProxyAPI::BatchGetMssFlag(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::map<unsigned int, QQMssInfo> &mstQQMssInfo, const SessionContext& stContext)
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
            return m_stRspTransPkg.stHeader.cResult;
        }
    }

    return RET_OK;
}

int CSnsOIDBProxyAPI::GetMssFlagMulti(unsigned int uiUin, QQAllMssInfo& stQQAllMssInfo)
{
    if (FillReqTransHeader(uiUin, 0x99, 0) != 0)
    {
        return RET_FAIL;
    }

    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, (char)1);
    m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, uiUin);

    if (SendAndRecv() != 0)
    {
        return RET_FAIL;
    }

    int iOffset = 0;
    if (m_stRspTransPkg.stHeader.cResult == 0)
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
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
        return RET_FAIL;
    }
}

int CSnsOIDBProxyAPI::BatchGetMssFlagMulti(const std::vector<unsigned int> &vuiUin, std::map<unsigned int, QQAllMssInfo>& mstQQAllMssInfo)
{
    int iRet = 0;
    for (size_t i = 0; i < vuiUin.size(); i++)
    {
        QQAllMssInfo stQQAllMssInfo;
        iRet = GetMssFlagMulti(vuiUin[i], stQQAllMssInfo);
        if (iRet) return iRet;
        mstQQAllMssInfo.insert(std::make_pair(vuiUin[i], stQQAllMssInfo));
    }
    return 0;
}

int CSnsOIDBProxyAPI::GetMssFlagMulti(unsigned int uiUin, QQAllMssInfo &stQQAllMssInfo, const SessionContext& stContext)
{
    std::vector<unsigned> vuiUin;
    vuiUin.push_back(uiUin);
    std::vector<QQAllMssInfo> vstQQAllMssInfo;
                        
    int iRet = BatchGetMssFlagMulti(uiUin, vuiUin, vstQQAllMssInfo, stContext);
    if (iRet) return iRet;
                              
    stQQAllMssInfo = vstQQAllMssInfo[0];
    return 0;
}

int CSnsOIDBProxyAPI::BatchGetMssFlagMulti(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, std::vector<QQAllMssInfo> &vstQQAllMssInfo, const SessionContext& stContext)
{
    vstQQAllMssInfo.clear();

    if (vuiUin.empty())
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "uin vector empty");
        return RET_FAIL;
    }

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
   
        char cUinCount = vuiUin.size();
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, cUinCount);
        for (int i = 0; i < cUinCount; i++)
        {
            m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, vuiUin[i]);
        }

        if (SendAndRecv() != 0)
        {
            return RET_FAIL;
        }

        int iOffset = 0;
        QQAllMssInfo stQQAllMssInfo;
        if (m_stRspTransPkg.stHeader.cResult == 0)
        {
            char cDoCount = 0;
            unsigned int uiNextBeginUin = 0;
            iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, cDoCount);
            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiNextBeginUin);

            for (int i = 0; i < cDoCount; i++)
            {
                iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, stQQAllMssInfo.uiUin);
                iOffset += CBuffTool::ReadString(m_stRspTransPkg.abyBody + iOffset, stQQAllMssInfo.szMssValue, sizeof(stQQAllMssInfo.szMssValue));
                vstQQAllMssInfo.push_back(stQQAllMssInfo);
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return m_stRspTransPkg.stHeader.cResult;
        }
    }

    return RET_OK;
}

int CSnsOIDBProxyAPI::BatchGetMssFlagMulti(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, std::map<unsigned int, QQAllMssInfo> &mstQQAllMssInfo, const SessionContext& stContext)
{
    mstQQAllMssInfo.clear();

    if (vuiUin.empty())
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "uin vector empty");
        return RET_FAIL;
    }

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
   
        char cUinCount = vuiUin.size();
        m_stReqTransPkg.ushBodyLen += CBuffTool::WriteByte(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, cUinCount);
        for (int i = 0; i < cUinCount; i++)
        {
            m_stReqTransPkg.ushBodyLen += CBuffTool::WriteInt(m_stReqTransPkg.abyBody + m_stReqTransPkg.ushBodyLen, vuiUin[i]);
        }

        if (SendAndRecv() != 0)
        {
            return RET_FAIL;
        }

        int iOffset = 0;
        QQAllMssInfo stQQAllMssInfo;
        if (m_stRspTransPkg.stHeader.cResult == 0)
        {
            char cDoCount = 0;
            unsigned int uiNextBeginUin = 0;
            iOffset += CBuffTool::ReadByte(m_stRspTransPkg.abyBody + iOffset, cDoCount);
            iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, uiNextBeginUin);

            for (int i = 0; i < cDoCount; i++)
            {
                iOffset += CBuffTool::ReadInt(m_stRspTransPkg.abyBody + iOffset, stQQAllMssInfo.uiUin);
                iOffset += CBuffTool::ReadString(m_stRspTransPkg.abyBody + iOffset, stQQAllMssInfo.szMssValue, sizeof(stQQAllMssInfo.szMssValue));
                mstQQAllMssInfo.insert(std::make_pair(stQQAllMssInfo.uiUin, stQQAllMssInfo));
            }
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "result = %d", m_stRspTransPkg.stHeader.cResult);
            return m_stRspTransPkg.stHeader.cResult;
        }
    }

    return RET_OK;
}

#include <string.h>
#include <stdio.h>

#include "comm/util/pet_util.h"
#include "webapi_base.h"
#include "comm/ini_file/ini_file.h"
#include "comm/log/pet_log.h"

using namespace snslib;

const int APP_HEADER_BUFF_LEN = 38;

//静态成员变量的定义部分
char CWebApiBase::m_szErrMsg[256] = {0};

char CWebApiBase::m_szSendBuff[MAX_WEBAPI_BUFF_LEN];
char CWebApiBase::m_szRecvBuff[MAX_WEBAPI_BUFF_LEN];
int CWebApiBase::m_iSendLen = 0;
int CWebApiBase::m_iRecvLen = 0;

CWebApiBase::CWebApiBase()
{
    m_pobjWebApiConn = NULL;
    m_uiHostIP = 0;
}
CWebApiBase::~CWebApiBase()
{
}

int CWebApiBase::BaseInit(CWebApiConn *pobjWebApiConn, unsigned int uiHostIP)
{
    m_pobjWebApiConn = pobjWebApiConn;
    m_uiHostIP = uiHostIP;

    return 0;
}

/**
 * @brief 发送数据
 * @param stAppHeader 需要发送数据的AppHeader部分
 * @param pszPkgBody 需要发送数据包体部分头指针
 * @param iPkgBodyLen 需要发送数据的包体长度
 *
 * @return 成功返回0，其他表示失败
 */
int CWebApiBase::Send(const AppHeader &stAppHeader, const char *pszPkgBody, int iPkgBodyLen)
{
    int iRetVal = 0;

    if (m_pobjWebApiConn == NULL)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "webapi_conn is not valid");
        return -1;
    }

    if ((iPkgBodyLen < 0)||((iPkgBodyLen > 0)&&(pszPkgBody == NULL))||(iPkgBodyLen > (int)(MAX_WEBAPI_BUFF_LEN - sizeof(ClientHeader) - sizeof(AppHeader))))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "pkg_body or pkg_body_len is not valid, len=%d", iPkgBodyLen);
        return -1;
    }

    m_iSendLen = 0;
    m_iSendLen += CBuffTool::WriteInt(m_szSendBuff+m_iSendLen, 0); //skip uiLength
    m_iSendLen += CBuffTool::WriteInt(m_szSendBuff+m_iSendLen, stAppHeader.uiUin);
    m_iSendLen += CBuffTool::WriteInt(m_szSendBuff+m_iSendLen, m_uiHostIP);
    m_iSendLen += CBuffTool::WriteShort(m_szSendBuff+m_iSendLen, stAppHeader.ushVersion);
    m_iSendLen += CBuffTool::WriteShort(m_szSendBuff+m_iSendLen, stAppHeader.ushZoneID);
    m_iSendLen += CBuffTool::WriteShort(m_szSendBuff+m_iSendLen, stAppHeader.ushCmdID);
    m_iSendLen += CBuffTool::WriteShort(m_szSendBuff+m_iSendLen, stAppHeader.ushSrcSvrID);
    m_iSendLen += CBuffTool::WriteShort(m_szSendBuff+m_iSendLen, stAppHeader.ushDestSvrID);
    m_iSendLen += CBuffTool::WriteShort(m_szSendBuff+m_iSendLen, stAppHeader.ushSrcMID);
    m_iSendLen += CBuffTool::WriteShort(m_szSendBuff+m_iSendLen, stAppHeader.ushDestMID);
    char *pCheckSumPos = m_szSendBuff+m_iSendLen;
    m_iSendLen += CBuffTool::WriteShort(m_szSendBuff+m_iSendLen, (unsigned short)0);
    m_iSendLen += CBuffTool::WriteInt(m_szSendBuff+m_iSendLen, stAppHeader.uiTransactionID);
    m_iSendLen += CBuffTool::WriteShort(m_szSendBuff+m_iSendLen, stAppHeader.ushStepID);
    m_iSendLen += CBuffTool::WriteInt(m_szSendBuff+m_iSendLen, stAppHeader.iRet);

    memcpy(m_szSendBuff+m_iSendLen, pszPkgBody, iPkgBodyLen);
    m_iSendLen += iPkgBodyLen;

    unsigned short ushCheckSum = CStrTool::CheckSum(m_szSendBuff, m_iSendLen);

    CBuffTool::WriteShort(pCheckSumPos, ushCheckSum);
    CBuffTool::WriteInt(m_szSendBuff, m_iSendLen-APP_HEADER_BUFF_LEN);

    PetLog(0, stAppHeader.uiUin, PETLOG_TRACE, "%s|%d|%s", __func__, m_iSendLen, CStrTool::Str2Hex(m_szSendBuff, m_iSendLen));
    iRetVal = m_pobjWebApiConn->Send(m_szSendBuff, m_iSendLen);
    if (iRetVal != 0)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", m_pobjWebApiConn->GetErrMsg());
        return -3;
    }

    return 0;
}

/**
 * @brief 接收数据
 * @param stAppHeader 接收到数据的AppHeader部分
 * @param ppszPkgBody 接收到数据包体部分头指针，该指针指向的空间不需要外部释放
 * @param piPkgBodyLen 接收到数据的包体长度
 *
 * @return 成功返回0，其他表示失败
 */
int CWebApiBase::Recv(AppHeader &stAppHeader, char **ppszPkgBody, int *piPkgBodyLen)
{
    int iRetVal = 0;

    if (m_pobjWebApiConn == NULL)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "webapi_conn is not valid");
        return -1;
    }

    m_iRecvLen = sizeof(m_szRecvBuff);
    iRetVal = m_pobjWebApiConn->Recv(m_szRecvBuff, &m_iRecvLen);
    if(iRetVal != 0)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", m_pobjWebApiConn->GetErrMsg());
        return -1;
    }

    if (m_iRecvLen < APP_HEADER_BUFF_LEN)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "recv pkg_len is not valid, recv_len=%d", m_iRecvLen);
        return -2;
    }

    int iPkgBodyLen = 0;
    CBuffTool::ReadInt(m_szRecvBuff, iPkgBodyLen);

    if ((iPkgBodyLen + APP_HEADER_BUFF_LEN) > MAX_WEBAPI_BUFF_LEN)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "pkg_len in header is not valid, len=%d", iPkgBodyLen);
        return -3;
    }

    PetLog(0, stAppHeader.uiUin, PETLOG_TRACE, "%s|%d|%d|%s", __func__, iPkgBodyLen, m_iRecvLen, CStrTool::Str2Hex(m_szRecvBuff, m_iRecvLen));

    //BUGFIX 如果第一次接收的数据包长度没有达到一个包头长度，会返回失败，但是这种概率非常小
    if (m_iRecvLen < (iPkgBodyLen + APP_HEADER_BUFF_LEN))
    {
        int iRemainLen = iPkgBodyLen + APP_HEADER_BUFF_LEN - m_iRecvLen;
        iRetVal = m_pobjWebApiConn->Recv(m_szRecvBuff + m_iRecvLen, &iRemainLen, iRemainLen);
        if(iRetVal != 0)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "recv remian pkg failed, remain_len=%d, ret=%d, errmsg=%s", iRemainLen, iRetVal, m_pobjWebApiConn->GetErrMsg());
            return -4;
        }

        m_iRecvLen += iRemainLen;
    }

    PetLog(0, stAppHeader.uiUin, PETLOG_TRACE, "%s|%d|%s", __func__, m_iRecvLen, CStrTool::Str2Hex(m_szRecvBuff, m_iRecvLen));

    int iOffSet = 0;
    iOffSet += CBuffTool::ReadInt(m_szRecvBuff+iOffSet, stAppHeader.uiLength); //skip uiLength
    iOffSet += CBuffTool::ReadInt(m_szRecvBuff+iOffSet, stAppHeader.uiUin);
    iOffSet += CBuffTool::ReadInt(m_szRecvBuff+iOffSet, stAppHeader.uiIP);
    iOffSet += CBuffTool::ReadShort(m_szRecvBuff+iOffSet, stAppHeader.ushVersion);
    iOffSet += CBuffTool::ReadShort(m_szRecvBuff+iOffSet, stAppHeader.ushZoneID);
    iOffSet += CBuffTool::ReadShort(m_szRecvBuff+iOffSet, stAppHeader.ushCmdID);
    iOffSet += CBuffTool::ReadShort(m_szRecvBuff+iOffSet, stAppHeader.ushSrcSvrID);
    iOffSet += CBuffTool::ReadShort(m_szRecvBuff+iOffSet, stAppHeader.ushDestSvrID);
    iOffSet += CBuffTool::ReadShort(m_szRecvBuff+iOffSet, stAppHeader.ushSrcMID);
    iOffSet += CBuffTool::ReadShort(m_szRecvBuff+iOffSet, stAppHeader.ushDestMID);
    CBuffTool::ReadShort(m_szRecvBuff+iOffSet, stAppHeader.ushCheckSum);
    iOffSet += CBuffTool::WriteShort(m_szRecvBuff+iOffSet, (unsigned short)0);
    iOffSet += CBuffTool::ReadInt(m_szRecvBuff+iOffSet, stAppHeader.uiTransactionID);
    iOffSet += CBuffTool::ReadShort(m_szRecvBuff+iOffSet, stAppHeader.ushStepID);
    iOffSet += CBuffTool::ReadInt(m_szRecvBuff+iOffSet, stAppHeader.iRet);

    if ((int)(stAppHeader.uiLength + APP_HEADER_BUFF_LEN) != m_iRecvLen)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "verify pkg failed, recv_len=%d, pkg_header_len=%d", m_iRecvLen, stAppHeader.uiLength);
        return -5;
    }

    /*
    unsigned short ushRealCheckSum = CStrTool::CheckSum(m_szRecvBuff, m_iRecvLen);
    if (ushRealCheckSum != stAppHeader.ushCheckSum)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "verify pkg failed, recv_cksum=%d, pkg_header_cksum=%d", ushRealCheckSum, stAppHeader.ushCheckSum);
        return -6;
    }
    */
    *ppszPkgBody = m_szRecvBuff + APP_HEADER_BUFF_LEN;
    *piPkgBodyLen = m_iRecvLen - APP_HEADER_BUFF_LEN;

    return 0;
}

/**
 * @brief 发送并且接收数据
 * @param stSendAppHeader 需要发送数据的AppHeader
 * @param pszSendPkgBody 需要发送数据的包体
 * @param iSendPkgBodyLen 需要发送数据的包体长度
 * @param stRecvAppHeader 接收到的数据对应的AppHeader
 * @param ppszRecvPkgBody 接收到的数据包体头指针，该指针指向的空间不需要外部释放
 * @param piRecvPkgBodyLen 收到数据的长度
 *
 * @return 成功返回0，其他表示失败
 * @note ppszRecvPkgBody指向的数据，只是该类成员变量中的一个BUFF，不需要函数外部释放
 */
int CWebApiBase::SendAndRecv(const AppHeader &stSendAppHeader, const char *pszSendPkgBody, int iSendPkgBodyLen, AppHeader &stRecvAppHeader, char **ppszRecvPkgBody, int *piRecvPkgBodyLen)
{
    int iRetVal = 0;

    iRetVal = Send(stSendAppHeader, pszSendPkgBody, iSendPkgBodyLen);
    if (iRetVal != 0)
    {
        return iRetVal;
    }

    iRetVal = Recv(stRecvAppHeader, ppszRecvPkgBody, piRecvPkgBodyLen);
    if (iRetVal != 0)
    {
        return iRetVal;
    }

    return 0;
}

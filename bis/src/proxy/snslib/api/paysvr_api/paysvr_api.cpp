#include <stdio.h>
#include <string.h>
#include "api/paysvr_api/paysvr_api.h"
#include "comm/ini_file/ini_file.h"
#include "comm/log/pet_log.h"
#include "comm/util/pet_util.h"

using namespace snslib;

CPaySvrAPI::CPaySvrAPI(): m_Timeout(DEFAULT_TIMEOUT), m_iPaySvrNum(0),
    m_iCurrPayID(0), m_SendLen(0), m_RecvLen(0)
{
    memset(m_ErrMsg, 0, sizeof(m_ErrMsg));
    //memset(m_ServerIP, 0, sizeof(m_ServerIP));
    memset(m_astPayConf, 0, sizeof(m_astPayConf));
    memset(m_SendBuf, 0, sizeof(m_SendBuf));
    memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
}

CPaySvrAPI::~CPaySvrAPI()
{
}

int CPaySvrAPI::Init(const char * file)
{
	if (NULL == file)
	{
		snprintf(m_ErrMsg, sizeof(m_ErrMsg), "配置文件错误");
		return RET_FAIL;
	}

	CIniFile objIni(file);
	if (objIni.IsValid())
	{
        objIni.GetInt("PAYSVR", "Timeout", DEFAULT_TIMEOUT, &m_Timeout);
        objIni.GetInt("PAYSVR", "PaySvrNum", 1, &m_iPaySvrNum);

        if ((m_iPaySvrNum <= 0)||(m_iPaySvrNum > 10))
        {
            snprintf(m_ErrMsg, sizeof(m_ErrMsg), "config item PAYSVR/PaySvrNum [%d] is not valid", m_iPaySvrNum);
            return RET_FAIL;
        }

        char szServerIP[16];
        int iServerPort;
        if (m_iPaySvrNum == 1)
        {
            objIni.GetString("PAYSVR", "IP", "", szServerIP, sizeof(szServerIP));
            objIni.GetInt("PAYSVR", "Port", 0, &iServerPort);

            if (szServerIP[0] != 0 && iServerPort != 0)
            {
                // Valid
                strncpy(m_astPayConf[0].szHost, szServerIP, 15);
                m_astPayConf[0].iPort = iServerPort;
            }
        }

        char szSecName[1024];
        for (int i=0; i<m_iPaySvrNum; i++)
        {
            snprintf(szSecName, sizeof(szSecName), "PAYSVR_%d", i+1);

            objIni.GetString(szSecName, "IP", "", szServerIP, sizeof(szServerIP));
            objIni.GetInt(szSecName, "Port", 0, &iServerPort);
            if (m_iPaySvrNum != 1 && (szServerIP[0] == 0 || iServerPort == 0))
            {
                snprintf(m_ErrMsg, sizeof(m_ErrMsg), "config item %s/IP[%s] /Port[%d] is not valid", szSecName, szServerIP, iServerPort);
                return RET_FAIL;
            }

            if (szServerIP[0] != 0 && iServerPort != 0)
            {
                strncpy(m_astPayConf[i].szHost, szServerIP, 15);
                m_astPayConf[i].iPort = iServerPort;
            }
        }

        if (m_astPayConf[0].szHost[0] == 0 || m_astPayConf[0].iPort == 0)
        {
            snprintf(m_ErrMsg, sizeof(m_ErrMsg), "config item %s/IP[%s] /Port[%d] is not valid", "PAYSVR", szServerIP, iServerPort);
            return RET_FAIL;
        }
	}
	else
	{
		snprintf(m_ErrMsg, sizeof(m_ErrMsg), "配置文件错误");
		return RET_FAIL;
	}

	m_iCurrPayID = CRandomTool::Instance()->Get(0, m_iPaySvrNum);

	/*
    if (0 != m_TcpClient.Init(m_ServerIP, m_ServerPort))
    {
        snprintf(m_ErrMsg, sizeof(m_ErrMsg), "连接PayServer出错 err=%s", m_TcpClient.GetErrMsg());
        return RET_FAIL;
    }

    m_TcpClient.SetTimeOut(m_Timeout);
    */

    return RET_OK;
}

/*
int CPaySvrAPI::Init(const char * ip, unsigned short port, int timeout)
{
	if (NULL == ip)
	{
		snprintf(m_ErrMsg, sizeof(m_ErrMsg), "PayServer IP 错误");
		return RET_FAIL;
	}
    strncpy(m_ServerIP, ip, MAX_IP_LEN);

	m_ServerPort = port;
    m_Timeout = timeout;

	if (0 != m_TcpClient.Init(m_ServerIP, m_ServerPort))
	{
		snprintf(m_ErrMsg, sizeof(m_ErrMsg), "连接PayServer出错 err=%s", m_TcpClient.GetErrMsg());
		return RET_FAIL;
	}
	m_TcpClient.SetTimeOut(m_Timeout);

	return RET_OK;
}
*/

int CPaySvrAPI::CheckPayRequest(const PayRequest & req)
{
    if (req.channel != PAY_CHANNEL_QQACCT && req.channel != PAY_CHANNEL_QQPOINT &&
        req.channel != PAY_CHANNEL_QB_QPOINT && req.channel != PAY_CHANNEL_QPOINT_QB &&
        req.channel != PAY_CHANNEL_TENPAY)
    {
		snprintf(m_ErrMsg, sizeof(m_ErrMsg), "参数错误 channel=%d", req.channel);
		return RET_ERR_INVALID_PARAM;
    }

    if (req.payUin <= 10000 || req.provideUin <= 10000 || req.provideUin != uint32_t(req.providePetID & 0xFFFFFFFF))
    {
        snprintf(m_ErrMsg, sizeof(m_ErrMsg), "参数错误 payUin=%u,provideUin=%u,"
            "providePetID=%llu(%u)", req.payUin, req.provideUin, req.providePetID,
            uint32_t(req.providePetID & 0xFFFFFFFF));
        return RET_ERR_INVALID_PARAM;
    }

    if (req.itemCount > PAY_MAX_ITEM_COUNT)
    {
        snprintf(m_ErrMsg, sizeof(m_ErrMsg), "参数错误 itemCount=%u", req.itemCount);
        return RET_ERR_INVALID_PARAM;
    }

    return RET_OK;
}

int CPaySvrAPI::SendAndRecv()
{
    int iRetVal = 0;
    int i=0;
    for(i=0; i<m_iPaySvrNum; i++)
    {
        if (!m_astPayConf[m_iCurrPayID].bInit)
        {
            iRetVal = m_aobjTcpClient[m_iCurrPayID].Init(m_astPayConf[m_iCurrPayID].szHost, m_astPayConf[m_iCurrPayID].iPort, m_Timeout);
            if(iRetVal != 0)
            {
                PetLog(0, 0, PETLOG_WARN, "connection_%d init failed, ret=%d, errmsg=%s", m_iCurrPayID, iRetVal, m_aobjTcpClient[m_iCurrPayID].GetErrMsg());
                m_iCurrPayID = (m_iCurrPayID+1)%m_iPaySvrNum;
                continue;
            }

            m_aobjTcpClient[m_iCurrPayID].SetTimeOut(m_Timeout);
            m_astPayConf[m_iCurrPayID].bInit = true;
        }

        if (m_aobjTcpClient[m_iCurrPayID].CheckConnect() != 0)
        {
            //当前连接是有问题的
            PetLog(0, 0, PETLOG_WARN, "connection_%d is disconnected, reconnect...", m_iCurrPayID);
            iRetVal = m_aobjTcpClient[m_iCurrPayID].ReconnectServer();
            if (iRetVal != 0)
            {
                PetLog(0, 0, PETLOG_WARN, "connection_%d reconnect failed, ret=%d, errmsg=%s", m_iCurrPayID, iRetVal, m_aobjTcpClient[m_iCurrPayID].GetErrMsg());
                m_iCurrPayID = (m_iCurrPayID+1)%m_iPaySvrNum;
                continue;
            }

            //重连成功
            PetLog(0, 0, PETLOG_WARN, "connection_%d reconnect succ", m_iCurrPayID);
            iRetVal = m_aobjTcpClient[m_iCurrPayID].SendAndRecv(m_SendBuf, m_SendLen, m_RecvBuf, m_RecvLen);
            if (iRetVal != 0)
            {
                //数据发送失败
                PetLog(0, 0, PETLOG_WARN, "connection_%d reconnect succ, but send_recv failed", m_iCurrPayID);
                m_iCurrPayID = (m_iCurrPayID+1)%m_iPaySvrNum;
                continue;
            }
            //数据收发成功
            break;
        }
        else
        {
            //当前连接是OK的
            iRetVal = m_aobjTcpClient[m_iCurrPayID].SendAndRecv(m_SendBuf, m_SendLen, m_RecvBuf, m_RecvLen);
            if (iRetVal != 0)
            {
                //失败重连一次
                PetLog(0, 0, PETLOG_WARN, "connection_%d send_recv failed", m_iCurrPayID);
                iRetVal = m_aobjTcpClient[m_iCurrPayID].ReconnectServer();
                if (iRetVal != 0)
                {
                    //重连失败
                    PetLog(0, 0, PETLOG_WARN, "connection_%d reconnect failed, ret=%d, errmsg=%s", m_iCurrPayID, iRetVal, m_aobjTcpClient[m_iCurrPayID].GetErrMsg());
                    m_iCurrPayID = (m_iCurrPayID+1)%m_iPaySvrNum;
                    continue;
                }

                //重连成功
                iRetVal = m_aobjTcpClient[m_iCurrPayID].SendAndRecv(m_SendBuf, m_SendLen, m_RecvBuf, m_RecvLen);
                if (iRetVal != 0)
                {
                    //数据发送失败
                    PetLog(0, 0, PETLOG_WARN, "connection_%d reconnect succ, but send_recv failed", m_iCurrPayID);
                    m_iCurrPayID = (m_iCurrPayID+1)%m_iPaySvrNum;
                    continue;
                }
            }

            //数据发送成功
            break;
        }
    }

    if (i == m_iPaySvrNum)
    {
        return -1;
    }

    return 0;
}

int CPaySvrAPI::Pay(int payID, const PayRequest & req, PayAns & ans)
{
    int ret = CheckPayRequest(req);
    if (ret != RET_OK) return ret;

    m_Header.version = 1;
    m_Header.cmd = CMD_PAY_PAYITEM;
    m_Header.payID = payID;
    m_Header.length = 0;

    m_SendLen = PAY_PROTO_HEADER_SIZE;
    m_SendLen += CPaySvrProto::Write(m_SendBuf + m_SendLen, req);

    short len = strlen(req.szPPKInfo);
    if (len > 0)
    {
        m_SendLen += CBuffTool::WriteShort(m_SendBuf + m_SendLen, len);
        m_SendLen += CBuffTool::WriteString(m_SendBuf + m_SendLen, req.szPPKInfo, len);
    }

    m_Header.length = m_SendLen - PAY_PROTO_HEADER_SIZE;
    CPaySvrProto::Write(m_SendBuf, m_Header);

    m_RecvLen = 8192;
    //ret = m_TcpClient.SendAndRecv(m_SendBuf, m_SendLen, m_RecvBuf, m_RecvLen);
    ret = SendAndRecv();
    if (ret != 0)
    {
        //snprintf(m_ErrMsg, sizeof(m_ErrMsg), "发送接收数据超时 ret=%d,err=%s", ret,
        //    m_TcpClient.GetErrMsg());
        return RET_ERR_SEND_RECV;
    }

    int offset = 0;
    offset += CPaySvrProto::Read(m_RecvBuf + offset, m_Header);
    offset += CPaySvrProto::Read(m_RecvBuf + offset, ans);

    if ((unsigned int)offset != m_RecvLen || m_Header.length + PAY_PROTO_HEADER_SIZE != m_RecvLen)
    {
        snprintf(m_ErrMsg, sizeof(m_ErrMsg), "解析应答包错误 m_Header.length=%u,offset=%d,"
            "m_RecvLen=%u", m_Header.length, offset, m_RecvLen);
        return RET_ERR_PROTOCOL;
    }

    return RET_OK;
}

int CPaySvrAPI::Pay(int payID, const PayAttr & attr, const PayRequest & req, PayAns & ans)
{
    int ret = CheckPayRequest(req);
    if (ret != RET_OK) return ret;

    m_Header.version = 1;
    m_Header.cmd = CMD_PAY_PAYITEM_ATTR;
    m_Header.payID = payID;
    m_Header.length = 0;

    m_SendLen = PAY_PROTO_HEADER_SIZE;
    m_SendLen += CPaySvrProto::Write(m_SendBuf + m_SendLen, req);
    m_SendLen += CPaySvrProto::Write(m_SendBuf + m_SendLen, attr);

    short len = strlen(req.szPPKInfo);
    if (len > 0)
    {
        m_SendLen += CBuffTool::WriteShort(m_SendBuf + m_SendLen, len);
        m_SendLen += CBuffTool::WriteString(m_SendBuf + m_SendLen, req.szPPKInfo, len);
    }

    m_Header.length = m_SendLen - PAY_PROTO_HEADER_SIZE;
    CPaySvrProto::Write(m_SendBuf, m_Header);

    m_RecvLen = 8192;
    //ret = m_TcpClient.SendAndRecv(m_SendBuf, m_SendLen, m_RecvBuf, m_RecvLen);
    ret = SendAndRecv();
    if (ret != 0)
    {
        //snprintf(m_ErrMsg, sizeof(m_ErrMsg), "发送接收数据超时 ret=%d,err=%s", ret,
        //    m_TcpClient.GetErrMsg());
        return RET_ERR_SEND_RECV;
    }

    int offset = 0;
    offset += CPaySvrProto::Read(m_RecvBuf + offset, m_Header);
    offset += CPaySvrProto::Read(m_RecvBuf + offset, ans);

    if ((unsigned int)offset != m_RecvLen || m_Header.length + PAY_PROTO_HEADER_SIZE != m_RecvLen)
    {
        snprintf(m_ErrMsg, sizeof(m_ErrMsg), "解析应答包错误 m_Header.length=%u,offset=%d,"
            "m_RecvLen=%u", m_Header.length, offset, m_RecvLen);
        return RET_ERR_PROTOCOL;
    }

    return RET_OK;
}


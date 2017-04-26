#ifndef _WEBAPI_BASE_H_
#define _WEBAPI_BASE_H_

#include <string.h>
#include <stdio.h>

#include "webapi_conn.h"
#include "api/include/sns_protocol.h"
#include "comm/log/pet_log.h"
#include "comm/util/pet_util.h"

namespace snslib
{

class CWebApiBase
{
public:
	const static int MAX_WEBAPI_HOST_NUM = 100;
	const static int MAX_WEBAPI_BUFF_LEN = 102400;

public:
	CWebApiBase();
	virtual ~CWebApiBase();

public:
	int BaseInit(CWebApiConn *pobjWebApiConn, unsigned int uiHostIP);

	static const char *GetErrMsg()
	{
	    return m_szErrMsg;
	}

protected:
	/**
	 * @brief 发送数据
	 * @param stAppHeader 需要发送数据的AppHeader部分
	 * @param pszPkgBody 需要发送数据包体部分头指针
	 * @param iPkgBodyLen 需要发送数据的包体长度
	 *
	 * @return 成功返回0，其他表示失败
	 */
	int Send(const AppHeader &stAppHeader, const char *pszPkgBody, int iPkgBodyLen);

    /**
     * @brief 接收数据
     * @param stAppHeader 接收到数据的AppHeader部分
     * @param ppszPkgBody 接收到数据包体部分头指针，该指针指向的空间不需要外部释放
     * @param piPkgBodyLen 接收到数据的包体长度
     *
     * @return 成功返回0，其他表示失败
     */
	int Recv(AppHeader &stAppHeader, char **ppszPkgBody, int *piPkgBodyLen);

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
	int SendAndRecv(const AppHeader &stSendAppHeader, const char *pszSendPkgBody, int iSendPkgBodyLen, AppHeader &stRecvAppHeader, char **ppszRecvPkgBody, int *piRecvPkgBodyLen);

	/**
	 * @brief 发送并接收数据
	 */
	template<class REQUEST_TYPE, class RESPONSE_TYPE>
	int SendAndRecv(unsigned int uiUin, unsigned short ushCmdID, unsigned short ushDstAppID, unsigned short ushDstSvrID, REQUEST_TYPE &objRequest, RESPONSE_TYPE &objResponse)
	{
	    int iRetVal = 0;

        char szSendBuff[MAX_WEBAPI_BUFF_LEN];
        int iSendLen;

	    AppHeader stSendAppHeader, stRecvAppHeader;
        memset(&stSendAppHeader, 0x0, sizeof(stSendAppHeader));
        stSendAppHeader.uiUin = uiUin;
        stSendAppHeader.ushCmdID = ushCmdID;
        stSendAppHeader.ushDestSvrID = ushDstSvrID;

        if(!objRequest.SerializeToArray(szSendBuff, sizeof(szSendBuff)))
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "serialize req to buff failed");
            return -1;
        }

        iSendLen = objRequest.ByteSize();

        char *pszRecvBuff = NULL;
        int iRecvLen = 0;
        iRetVal = SendAndRecv(stSendAppHeader, szSendBuff, iSendLen, stRecvAppHeader, &pszRecvBuff, &iRecvLen);
        if (iRetVal != 0)
        {
            return -2;
        }

        if (iRecvLen <= 0)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "recv len is not valid, recv_len=%d", iRecvLen);
            return -3;
        }

        if (stRecvAppHeader.iRet != 0)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "ret val in header is %d", stRecvAppHeader.iRet);
            return stRecvAppHeader.iRet;
        }

        if(!objResponse.ParseFromArray(pszRecvBuff, iRecvLen))
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "parse rsp from buff failed");
            return -4;
        }

        return 0;
	}

    /**
     * @brief 发送并接收数据，接收的数据没有包体部分
     */
    template<class REQUEST_TYPE>
    int SendAndRecv(unsigned int uiUin, unsigned short ushCmdID, unsigned short ushDstAppID, unsigned short ushDstSvrID, REQUEST_TYPE &objRequest)
    {
        int iRetVal = 0;

        char szSendBuff[MAX_WEBAPI_BUFF_LEN];
        int iSendLen;

        AppHeader stSendAppHeader, stRecvAppHeader;
        memset(&stSendAppHeader, 0x0, sizeof(stSendAppHeader));
        stSendAppHeader.uiUin = uiUin;
        stSendAppHeader.ushCmdID = ushCmdID;
        stSendAppHeader.ushDestSvrID = ushDstSvrID;

        if(!objRequest.SerializeToArray(szSendBuff, sizeof(szSendBuff)))
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "serialize req to buff failed");
            return -1;
        }

        iSendLen = objRequest.ByteSize();

        PetLog(0, uiUin, PETLOG_TRACE, "PROTO_SERIAL|%d|%s", iSendLen, CStrTool::Str2Hex(szSendBuff, iSendLen));

        char *pszRecvBuff = NULL;
        int iRecvLen = 0;
        iRetVal = SendAndRecv(stSendAppHeader, szSendBuff, iSendLen, stRecvAppHeader, &pszRecvBuff, &iRecvLen);
        if (iRetVal != 0)
        {
            return -2;
        }

        if (stRecvAppHeader.iRet != 0)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "ret val in header is %d", stRecvAppHeader.iRet);
            return stRecvAppHeader.iRet;
        }

        return 0;
    }

protected:
	static char m_szErrMsg[256];
    unsigned int m_uiHostIP;

private:

	CWebApiConn *m_pobjWebApiConn;

    //下面定义成全局变量，会导致线程不安全，但是不过不用多线程就无所谓
	static char m_szSendBuff[MAX_WEBAPI_BUFF_LEN];
	static char m_szRecvBuff[MAX_WEBAPI_BUFF_LEN];
	static int m_iSendLen;
	static int m_iRecvLen;
};
}
#endif //_WEBAPI_BASE_H_

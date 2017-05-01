#ifndef _WEBAPI_CONN_H_
#define _WEBAPI_CONN_H_

#include <string.h>
#include <stdio.h>

#include "comm/tcp_client/tcp_client.h"
#include "comm/log/pet_log.h"
#include "comm/util/pet_util.h"

namespace snslib
{

class CWebApiConn
{
public:
	const static int MAX_WEBAPI_HOST_NUM = 100;
	const static int MAX_WEBAPI_BUFF_LEN = 102400;

public:
	CWebApiConn();
	virtual ~CWebApiConn();

public:
    /**
     * @brief 初始化
     * @param pszConfFile 配置文件
     *
     * @return 成功返回0，其他表示失败
     */
	int Init(const char* pszConfFile);

	/**
	 * @brief 发送数据
	 * @param pszSendBuff 需要发送的数据包
	 * @param iSendLen 需要发送数据包的长度
	 *
	 * @return 成功返回0，其他表示失败
	 */
	int Send(char *pszSendBuff, int iSendLen);

    /**
     * @brief 接收数据
     * @param ppszRecvBuff 接收到的数据包，该指针指向的空间不需要外部释放
     * @param piPkgBodyLen 接收到数据包的长度
     * @param iExpireLen 期待接收数据的长度，0-表示不限制接收长度
     *
     * @return 成功返回0，其他表示失败
     */
	int Recv(char *pszRecvBuff, int *piRecvLen, int iExpireLen = 0);

	/**
	 * @brief 随机重连一台WebPCL
	 */
	int Reconnect();

    /**
     * @brief 获取错误信息
     */
    const char *GetErrMsg()
    {
        return m_szErrMsg;
    }

private:
	int m_iHostNum;
	char m_aszIpList[MAX_WEBAPI_HOST_NUM][16];
	int m_aiPortList[MAX_WEBAPI_HOST_NUM];
	CTcpClient m_objTcpClient;

	char m_szRecvBuff[MAX_WEBAPI_BUFF_LEN];
	int m_iRecvLen;

	char m_szErrMsg[256];
};
}
#endif //_WEBAPI_BASE_H_

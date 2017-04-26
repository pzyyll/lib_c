#ifndef PAYSVR_API_H
#define PAYSVR_API_H

#include "api/paysvr_api/paysvr_def.h"
#include "comm/tcp_client/tcp_client.h"
#include "api/paysvr_api/paysvr_proto.h"

namespace snslib
{

class CPaySvrAPI
{
public:
    // 默认端口和超时时间
    const static int DEFAULT_TIMEOUT            = 8000;
    const static unsigned short DEFAULT_PORT    = 4321;

    // 最大Paysvr数目
    const static int PAYSVR_NUM_MAX = 10;

    // 接口错误码定义
    const static int RET_OK                 = 0;
    const static int RET_FAIL               = -1;
    const static int RET_ERR_INVALID_PARAM  = -101;
    const static int RET_ERR_SEND_RECV      = -102;
    const static int RET_ERR_PROTOCOL       = -103;
    const static int RET_ERR_MAX_PRICE      = -104;

    typedef struct tagPaySvrConf
    {
        char szHost[16];
        int iPort;
        bool bInit;

        tagPaySvrConf()
        {
            memset(this, 0, sizeof(*this));
        }
    } PaySvrConf;

	CPaySvrAPI();
	~CPaySvrAPI();

	/**
	 * 初始化paysvr接口
	 * 参数: file: 接口配置文件
	 * 返回值: 接口错误码定义
	 */
	int Init(const char * file);

	/**
	 * 初始化paysvr接口
	 * 参数: ip: paysvr ip, port: paysvr端口, timeout: 支付接口调用超时时间
	 * 返回值: 接口错误码定义
	 */
	//int Init(const char * ip, unsigned short port, int timeout = DEFAULT_TIMEOUT);

	/**
	 * 商品支付
	 * 参数: payID: 支付ID,主要用来区分各种支付活动, req: 支付请求, ans: 支付应答
	 * 返回值: 接口错误码定义
	 */
	int Pay(int payID, const PayRequest & req, PayAns & ans);

	/**
	 * 带更新属性的商品支付
	 * 参数: payID: 支付ID,主要用来区分各种支付活动, attr: 更新属性, req: 支付请求, ans: 支付应答
	 * 返回值: 接口错误码定义
	 */
	int Pay(int payID, const PayAttr & attr, const PayRequest & req, PayAns & ans);

    inline const char * GetErrMsg() const
    {
        return m_ErrMsg;
    }
private:
    int CheckPayRequest(const PayRequest & req);
    int SendAndRecv();
private:
    PayHeader m_Header;
	char m_ErrMsg[1024];
	//char m_ServerIP[petlib::MAX_IP_LEN + 1];
	//unsigned short m_ServerPort;
	int m_Timeout;

    PaySvrConf m_astPayConf[PAYSVR_NUM_MAX];
    CTcpClient m_aobjTcpClient[PAYSVR_NUM_MAX];
    int m_iPaySvrNum;
    int m_iCurrPayID;

    unsigned int m_SendLen;
	char m_SendBuf[8192];

    unsigned int m_RecvLen;
	char m_RecvBuf[8192];

	//CTcpClient m_TcpClient;
};

}
#endif



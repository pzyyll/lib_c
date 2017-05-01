#ifndef _SNS_OIDB_API_H_
#define _SNS_OIDB_API_H_

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <limits.h>
#include "comm/safe_tcp_client/safe_tcp_client.h"
#include "api/sns_oidb_api/sns_oidb_def.h"

namespace snslib
{
namespace oidb2
{
const unsigned char OIDB_API_MAX_IP_LEN  = 15;

// MSSTYPE
// 以下这几个都是QQ会员标记位
const unsigned short OIDB_MSSTYPE_CLUB = 64;		//会员身份
const unsigned short OIDB_MSSTYPE_CLUB_ANNUAL = 65;	//年费会员
const unsigned short OIDB_MSSTYPE_163S = 65;		//年费会员，按OIDB标准命名
const unsigned short OIDB_MSSTYPE_163C = 66;		//163上网卡用户
const unsigned short OIDB_MSSTYPE_263U = 67;		//263拨号用户
const unsigned short OIDB_MSSTYPE_MCLB = 68;		//移动会员
const unsigned short OIDB_MSSTYPE_TCNT = 69;		//15元手机包月用户
const unsigned short OIDB_MSSTYPE_VNETCLB = 81;		//Vnet会员用户

const unsigned short OIDB_MSSTYPE_PETVIP = 92;      //粉钻增值位
const unsigned short OIDB_MSSTYPE_QZONEVIP = 78;    //黄钻增值位

const unsigned short OIDB_MSSTYPE_PETVIP_YEAR = 103;//粉钻年费增值位

const unsigned short OIDB_PROXY_NUM_MAX = 10;   //最多能够并行的OIDB_PROXY服务器数量

const unsigned char OIDB_API_MAX_NICKNAME_LEN    = 64;
const unsigned short OIDB_API_MAX_PKG_LEN       = 8192;

const int OIDB_MAX_SESSIONKEY_LEN       = 256;      // session key最大长度
const int OIDB_MAX_IP_LEN = 16;	// ip 长度
const char OIDB_SESSION_PTLOGIN = 1;
const char OIDB_SESSION_GAMESIG = 2;
const char OIDB_SESSION_SSO     = 3;
const char OIDB_SESSION_CLIENT  = 4;

typedef struct tagQQSimpleInfo
{
    unsigned int uiUin;
    unsigned short ushFace;
    unsigned char byAge;
    unsigned char byGender;
    char szNick[OIDB_API_MAX_NICKNAME_LEN + 1];

} SQQSimpleInfo;

typedef struct tagQQMssInfo
{
    unsigned int uiUin;
    unsigned short ushMssType;
    char cMssValue;
}QQMssInfo;

typedef struct tagQQAllMssInfo
{
    unsigned int uiUin;
    char szMssValue[20];    //现在oidb提供20*8个增值位，按照位的偏移量来取，各增值位的偏移量为OIDB_MSSTYPE_...，详细的增值位信息请参考oidb网站上的数据
}QQAllMssInfo;

typedef struct tagSessionContext
{
    unsigned int uiAppID;
    char cKeyType;
    char szSessionKey[OIDB_MAX_SESSIONKEY_LEN];
    unsigned short ushSessionKeyLen;
    unsigned int uiClientIP;
    unsigned int uiConnSvrIP;

    tagSessionContext() { memset(this, 0x0, sizeof(*this)); }
} SessionContext;

typedef struct tagGroupInfo
{
	unsigned int uiGroupID;
	unsigned int uiSortID;
	std::string sGroupName; // group name is utf8 encoded
	tagGroupInfo() : uiGroupID(0), uiSortID(0)
	{
	}
} GroupInfo;

typedef struct tagGroupedFriendList
{
	unsigned int uiGroupID;
	unsigned int uiSortID;
	std::string sGroupName;
	std::vector<unsigned int> vGroupMemberList; // qqnick is gbk encoded
	tagGroupedFriendList() : uiGroupID(0), uiSortID(0)
	{
	}
} GroupedFriendList;

typedef struct tagProtoHeader
{
    unsigned short ushLength;
    unsigned int uiUin;
    unsigned short ushCommand;
    unsigned int uiServiceIP;
    char szServiceName[16];
    unsigned int uiClientIP;
    unsigned char byServiceType;
    unsigned char byResult;

    unsigned short ushExtraFlag; // 字段不被序列化/反序列化，只是作为标识
    unsigned int uiAppID;
    char chKeyType; // 类型见OIDB_SESSION_xxx
    unsigned short ushSessionKeyLen;
    char szSessionKey[OIDB_MAX_SESSIONKEY_LEN];
} SOIDBProtoHeader;

const unsigned short OIDB_API_PROTO_HEADER_LEN  = 34;
const unsigned short OIDB_API_PROTO_EXTRA_HEADER_MIN_LEN = 9;

typedef struct tagOIDBProxyConf
{
    char szHost[OIDB_API_MAX_IP_LEN + 1];
    int iPort;
    bool bInit;

    tagOIDBProxyConf()
    {
        memset(this, 0, sizeof(*this));
    }
} OIDBProxyConf;

class CSnsOIDBProxyAPI
{
public:
    CSnsOIDBProxyAPI();
    ~CSnsOIDBProxyAPI();

    static const int RET_OK = 0;
    static const int RET_FAIL = -1;
    static const int RET_IS_FRIEND = 1;
    static const int RET_NOT_FRIEND = 0;

    /**
     * 初始化oidb_api接口
     * 参数: pszFile 接口配置文件, pszServiceName 调用接口的程序业务名称
     * 返回值: RET_OK, RET_FAIL
     */
    int Init(const char *pszFile, const char *pszServiceName);

    /**
     * 获取COIDBProxyAPI的单体实例。
     *
     * @return COIDBProxyAPI的单体实例
     */
    static CSnsOIDBProxyAPI& Instance();

    /**
     * 获取好友列表
     * 参数: uiUin QQ号码, vecFriends 返回的好友列表
     * 返回值: RET_OK, RET_FAIL
     */
    int GetFriendList(unsigned int uiUin, std::vector<unsigned int> &vecFriends);
    int GetFriendList(unsigned int uiUin, std::vector<unsigned int> &vecFriends, const SessionContext& stContext);

    int GetGroupedFriendList(unsigned int uiUin, std::vector<GroupedFriendList> &vecGroupedFriends, const SessionContext& stContext);
    int GetGroupInfo(unsigned uiUin, std::map<unsigned, GroupInfo> &mstQQFriendGroups, const SessionContext& stContext);

    /**
     * 获取简单资料，包括昵称、性别等
     * 参数: uiUin QQ号码, stInfo 返回简单资料
     * 返回值: RET_OK, RET_FAIL
     */
    int GetSimpleInfo(unsigned int uiUin, SQQSimpleInfo &stInfo);
    int GetSimpleInfo(unsigned int uiUin, SQQSimpleInfo &stInfo, const SessionContext& stContext);

    /**
     * 批量获取简单资料
     * 参数: uiUin QQ号码, vecUinList 号码列表, vecInfo 返回的资料列表
     * 返回值: RET_OK, RET_FAIL
     */
    int BatchGetSimpleInfo(const std::vector<unsigned int> &vecUinList, std::vector<SQQSimpleInfo> &vecInfo);
    int BatchGetSimpleInfo(const std::vector<unsigned int> &vuiUin, std::map<unsigned int, SQQSimpleInfo> &mstQQSimpInfo);
    int BatchGetSimpleInfo(unsigned int uiUin, const std::vector<unsigned int> &vecUinList, std::vector<SQQSimpleInfo> &vecInfo, const SessionContext& stContext);
    int BatchGetSimpleInfo(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, std::map<unsigned int, SQQSimpleInfo> &mstQQSimpInfo, const SessionContext& stContext);

    /**
     * 验证uiFriendUin是不是uiUin的好友
     * 参数: uiUin QQ号码, uiFriendUin 需要验证的好友号码
     * 返回值: RET_OK, RET_FAIL, RET_IS_FRIEND, RET_NOT_FRIEND
     */
    int CheckFriend(unsigned int uiUin, unsigned int uiFriendUin);
    int CheckFriend(unsigned int uiUin, unsigned int uiFriendUin, const SessionContext& stContext);

    /**
     * 获取单个用户单个增值位的值
     * param uiUin: 需要获取用户的UIN
     * param ushMssType: 需要获取用户的增值位 OIDB_MSSTYPE_...
     * param cMssValue: 增值位的值 0-没有该增值位 1-有该增值位
     *
     * return: RET_OK, RET_FAIL
     */
    int GetMssFlag(unsigned int uiUin, unsigned short ushMssType, char &cMssValue);
    int GetMssFlag(unsigned int uiUin, unsigned short ushMssType, char &cMssValue, const SessionContext& stContext);

    /**
     * 批量获取多个用户的单个增值位
     * param vuiUin: 需要获取用户的UIN
     * param ushMssType: 需要获取用户的增值位 OIDB_MSSTYPE_...
     * param vstQQMssInfo: 增值位信息 QQMssInfo.cMssValue  0-没有该增值位 1-有该增值位
     *
     * return: RET_OK, RET_FAIL
     */
    int BatchGetMssFlag(const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::vector<QQMssInfo> &vstQQMssInfo);
    int BatchGetMssFlag(const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::map<unsigned int, QQMssInfo> &mstQQMssInfo);
    int BatchGetMssFlag(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::vector<QQMssInfo> &vstQQMssInfo, const SessionContext& stContext);
    int BatchGetMssFlag(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::map<unsigned int, QQMssInfo> &mstQQMssInfo, const SessionContext& stContext);

    /**
     * 获取单个用户的所有增值位
     * param uiUin: 需要获取用户的UIN
     * param stQQAllMssInfo: 增值位信息 QQAllMssInfo.szMssValue 按位取，偏移量是增值位的编号
     *
     * return: RET_OK, RET_FAIL
     */
    //delete by jamieli，后面不再使用该接口
    int GetMssFlagMulti(unsigned int uiUin, QQAllMssInfo &stQQAllMssInfo);

    int BatchGetMssFlagMulti(const std::vector<unsigned int> &vuiUin, std::map<unsigned int, QQAllMssInfo>& mstQQAllMssInfo);

    int GetMssFlagMulti(unsigned int uiUin, QQAllMssInfo &stQQAllMssInfo, const SessionContext& stContext);

    int BatchGetMssFlagMulti(unsigned uiUin, const std::vector<unsigned int> &vuiUin, std::map<unsigned int, QQAllMssInfo> &mstQQAllMssInfo, const SessionContext& stContext);
    int BatchGetMssFlagMulti(unsigned uiUin, const std::vector<unsigned int> &vuiUin, std::vector<QQAllMssInfo> &vstQQAllMssInfo, const SessionContext& stContext);

    inline const char *GetErrMsg()
    {
        return m_szErrMsg;
    }


    static int PackTransPkg(char * pMem, const STransPkg &stPkg);
    static int UnPackTransPkg(const char * pMem, unsigned uiLen, STransPkg& stPkg);

    static int PackHeader(char * pMem, const SOIDBProtoHeader &stHeader);
    static int UnPackHeader(const char * pMem, SOIDBProtoHeader &stHeader);
    static int UnPackExtraHeader(const char * pMem, SOIDBProtoHeader &stHeader);

private:
    int FillReqTransHeader(unsigned int uiUin, unsigned short ushCmdID, char cServiceType, const SessionContext *pstContext = NULL);
    int SendAndRecv();
    int SendAndRecvOnce();

private:
    bool m_bInit;
    char m_szErrMsg[256];
    int m_iTimeout;

    unsigned int m_uiServiceIP;
    char m_szServiceName[16];

    char m_abyRequest[OIDB_API_MAX_PKG_LEN];
    unsigned int m_uiRequestLen;
    char m_abyResponse[OIDB_API_MAX_PKG_LEN];
    unsigned int m_uiResponseLen;
    STransPkg m_stReqTransPkg;
    STransPkg m_stRspTransPkg;

    CSafeTcpClient m_oTcpClient;
    OIDBProxyConf m_astProxyConf[OIDB_PROXY_NUM_MAX];
    int m_iOIDBProxyNum;

    unsigned int m_uiReqSeq;

    //SOIDBProtoHeader m_stHeader;

};
}
}
#endif


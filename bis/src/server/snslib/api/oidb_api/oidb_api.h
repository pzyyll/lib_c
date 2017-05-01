#ifndef _OIDB_API_H_
#define _OIDB_API_H_

#include <vector>
#include <map>
#include <string>
#include <string.h>
#include "comm/safe_tcp_client/safe_tcp_client.h"
#include "api/oidb_api/oidb_def.h"

namespace snslib
{

// RichFlag
const unsigned char OIDB_API_SERVICE_TYPE_PET   = 4;    //RICHFLAG狗头标志位
const unsigned char OIDB_API_SERVICE_TYPE_CFACE = 12;   //RICHFLAG自定义头像标志位

// RichFlag2
const unsigned char OIDB_API_SERVICE_TYPE_BEAR = 53;	//熊熊图标

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
const unsigned short OIDB_MSSTYPE_QQGAMEVIP = 77;   //蓝钻增值位
const unsigned short OIDB_MSSTYPE_QZONEVIP = 78;    //黄钻增值位
const unsigned short OIDB_MSSTYPE_LVZUAN_1 = 82;    //绿钻增值位1
const unsigned short OIDB_MSSTYPE_LVZUAN_2 = 91;    //绿钻增值位2

const unsigned short OIDB_MSSTYPE_PETVIP_YEAR = 103;//粉钻年费增值位
const unsigned short OIDB_MSSTYPE_QQGAMEVIP_YEAR = 102; //蓝钻年费增值位
const unsigned short OIDB_MSSTYPE_QZONEVIP_YEAR = 104;  //黄钻年费增值位
const unsigned short OIDB_MSSTYPE_LVZUAN_YEAR = 136;  //绿钻年费增值位

// RichFlagLevel
const unsigned char OIDB_RICHFLAGLEVEL_FZ = 49;		//粉钻等级
const unsigned char OIDB_RICHFLAGLEVEL_PET = 59;	//宠物分级
const unsigned char OIDB_RICHFLAGLEVEL_HZ = 2;      //黄钻等级
const unsigned char OIDB_RICHFLAGLEVEL_LZ = 16;     //蓝钻等级
const unsigned char OIDB_RICHFLAGLEVEL_LVZ = 14;     //绿钻等级

const unsigned short OIDB_PROXY_NUM_MAX = 10;   //最多能够并行的OIDB_PROXY服务器数量

const unsigned char OIDB_API_MAX_NICKNAME_LEN    = 63;
// shimmeryang： 修改为81920
const unsigned int OIDB_API_MAX_PKG_LEN       = 81920;

const int OIDB_MAX_SESSIONKEY_LEN       = 256;      // session key最大长度
const int OIDB_MAX_IP_LEN = 16;	// ip 长度

const char OIDB_SESSION_PTLOGIN = 1;
const char OIDB_SESSION_GAMESIG = 2;
const char OIDB_SESSION_SSO     = 3;
const char OIDB_SESSION_CLIENT  = 4;

const int OIDB_PET_SSO_ST_APPID = 0x200E01;
const int OIDB_PET_CLIENT_ST_APPID = 0x1000201;

typedef struct tagQQSimpleInfo
{
    unsigned int uiUin;
    // 2013年12月02日 18:20:09 头像已经不能在oidb拉取了，找edzhong，
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

#define IsBitSet(pBuf,bit) (((unsigned char*)(pBuf))[(bit)/8]&(0x80>>((bit)%8)))

typedef struct tagQQAllMssInfo
{
    unsigned int uiUin;
    char szMssValue[20]; //20个字节标志位, 详细的增值位说明请参考“OIDB帮助文档列表”中的《增值中心bitmap定义》

    bool IsHZ()
    {
        return IsBitSet(szMssValue, OIDB_MSSTYPE_QZONEVIP);
    }
    bool IsHZYear()
    {
        return IsBitSet(szMssValue, OIDB_MSSTYPE_QZONEVIP_YEAR);
    }

    bool IsLZ()
    {
        return IsBitSet(szMssValue, OIDB_MSSTYPE_QQGAMEVIP);
    }
    bool IsLZYear()
    {
        return IsBitSet(szMssValue, OIDB_MSSTYPE_QQGAMEVIP_YEAR);
    }

    bool IsFZ()
    {
        return IsBitSet(szMssValue, OIDB_MSSTYPE_PETVIP);
    }
    bool IsFZYear()
    {
        return IsBitSet(szMssValue, OIDB_MSSTYPE_PETVIP_YEAR);
    }

    bool IsLvZ()
    {
        return (IsBitSet(szMssValue, OIDB_MSSTYPE_LVZUAN_1) || IsBitSet(szMssValue, OIDB_MSSTYPE_LVZUAN_2));
    }
    bool IsLvZYear()
    {
        return IsBitSet(szMssValue, OIDB_MSSTYPE_LVZUAN_YEAR);
    }
}QQAllMssInfo;

typedef struct tagSessionContext
{
    //如果是企鹅的SSO_ST，则写：0x200E01；如果是的session_key，则写业务在ptlogin对应的appid
    unsigned int uiAppID;
    //PET_SSO_ST为3，ptlogin为1，client_key为4
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
	std::string sGroupName;
	tagGroupInfo() : uiGroupID(0)
	{
	}
} GroupInfo;

typedef struct tagGroupedFriendList
{
	unsigned int uiGroupID;
	unsigned int uiSortID;
	std::string sGroupName;
	std::vector<unsigned int> vuiGroupMemberList;
	tagGroupedFriendList() : uiGroupID(0)
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
    char szHost[20];
    int iPort;
    bool bInit;

    tagOIDBProxyConf()
    {
        memset(this, 0, sizeof(*this));
    }
} OIDBProxyConf;

class COIDBProxyAPI
{
public:
    COIDBProxyAPI();
    ~COIDBProxyAPI();

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
    static COIDBProxyAPI& Instance();

    /**
     * 获取好友列表
     * 参数: uiUin QQ号码, vecFriends 返回的好友列表
     * 返回值: RET_OK, RET_FAIL
     */
    // shimmeryang: 下面两个协议停止使用
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
     * 获取RichFlag
     * 参数: uiUin QQ号码, byFlag 返回的RichFlag, byServiceType 业务类型信息
     * 返回值: RET_OK, RET_FAIL
     */
    int GetRichFlag(unsigned int uiUin, unsigned char &byFlag, unsigned char byServiceType);
    int GetRichFlag(unsigned int uiUin, unsigned char &byFlag, unsigned char byServiceType, const SessionContext& stContext);

    /**
     * 设置RichFlag
     * 参数: uiUin QQ号码, byFlag 设置的RichFlag, byServiceType 业务类型信息，默认为宠物业务
     * 返回值: RET_OK, RET_FAIL
     */
    int SetRichFlag(unsigned int uiUin, unsigned char byFlag, unsigned char byServiceType);
    int SetRichFlag(unsigned int uiUin, unsigned char byFlag, unsigned char byServiceType, const SessionContext& stContext);

    /**
     * 批量获取RichFlag
     * 参数: uiUin QQ号码, vecUinList 号码列表, mapFlags 返回的RichFlag, byServiceType 业务类型信息，默认为宠物业务
     * 返回值: RET_OK, RET_FAIL
     */
    int GetFriendsRichFlag(unsigned int uiUin, const std::vector<unsigned int> &vecUinList, std::map<unsigned int, unsigned char> &mapFlags, unsigned char byServiceType)
    {
        return BatchGetRichFlag(uiUin, vecUinList, mapFlags, byServiceType);
    }
    int GetFriendsRichFlag(unsigned int uiUin, const std::vector<unsigned int> &vecUinList, std::map<unsigned int, unsigned char> &mapFlags, unsigned char byServiceType, const SessionContext& stContext)
    {
        return BatchGetRichFlag(uiUin, vecUinList, mapFlags, byServiceType, stContext);
    }
    int BatchGetRichFlag(unsigned int uiUin, const std::vector<unsigned int> &vecUinList, std::map<unsigned int, unsigned char> &mapFlags, unsigned char byServiceType);
    int BatchGetRichFlag(unsigned int uiUin, const std::vector<unsigned int> &vecUinList, std::map<unsigned int, unsigned char> &mapFlags, unsigned char byServiceType, const SessionContext& stContext);

    /**
     * 批量获取简单资料
     * 参数: uiUin QQ号码, vecUinList 号码列表, vecInfo 返回的资料列表
     * 返回值: RET_OK, RET_FAIL
     */
    // 上面两个BatchGetSimpleInfo都没有带session，已经不能使用了
    int BatchGetSimpleInfo(const std::vector<unsigned int> &vecUinList, std::vector<SQQSimpleInfo> &vecInfo);
    int BatchGetSimpleInfo(const std::vector<unsigned int> &vuiUin, std::map<unsigned int, SQQSimpleInfo> &mstQQSimpInfo);
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
     * 设置单个用户单个增值位的值
     * param uiUin: 需要获取用户的UIN
     * param ushMssType: 需要获取用户的增值位 OIDB_MSSTYPE_...
     * param cMssValue: 增值位的值 0-没有该增值位 1-有该增值位
     *
     * return: RET_OK, RET_FAIL
     */
    int SetMssFlag(unsigned int uiUin, unsigned short ushMssType, char cMssValue);
    int SetMssFlag(unsigned int uiUin, unsigned short ushMssType, char cMssValue, const SessionContext& stContext);

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
    int BatchGetMssFlag(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, unsigned short ushMssType, std::map<unsigned int, QQMssInfo> &mstQQMssInfo, const SessionContext& stContext);

    /**
     * 批量获取多个用户的多个增值位
     * param vuiUin: 需要获取用户的UIN
     *
     * return: RET_OK, RET_FAIL
     */
    // add by tofuli 2013.01.28
    int BatchGetAllMssFlag(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, std::map<unsigned int, QQAllMssInfo> &mstAllQQMssInfo, const SessionContext& stContext);

    /**
     * 获取单个用户的所有增值位
     * param uiUin: 需要获取用户的UIN
     * param stQQAllMssInfo: 增值位信息 QQAllMssInfo.szMssValue 按位取，偏移量是增值位的编号
     *
     * return: RET_OK, RET_FAIL
     */
    //delete by jamieli，后面不再使用该接口
    //int GetMssFlagMulti(unsigned int uiUin, QQAllMssInfo &stQQAllMssInfo);

	/**
	 * 拉取/设置业务等级
	 * param uiUin: 需要操作的用户UIN
	 * param ushType: 业务类型
	 * param cValue: 业务值
	 *
	 * return: RET_OK, RET_FAIL
	 */
	int GetRichFlagLevel(unsigned int uiUin, unsigned char byServiceType, unsigned char &byLevel);
	int GetRichFlagLevel(unsigned int uiUin, unsigned char byServiceType, unsigned char &byLevel, const SessionContext& stContext);
	int SetRichFlagLevel(unsigned int uiUin, unsigned char byServiceType, unsigned char byLevel);
    int BatchGetRichFlagLevel(unsigned int uiUin, const std::vector<unsigned int> &vuiUin, unsigned char byServiceType, std::map<unsigned int, unsigned char> &mRichFlagLevel, const SessionContext& stContext);

    /**
     * 获取RichFlag
     * 参数: uiUin QQ号码, byFlag 返回的RichFlag, byServiceType 业务类型信息
     * 返回值: RET_OK, RET_FAIL
     */
    int GetRichFlag2(unsigned int uiUin, unsigned char &byFlag, unsigned char byServiceType);

    /**
     * 设置RichFlag2
     * 参数: uiUin QQ号码, byFlag 设置的RichFlag, byServiceType 业务类型信息，默认为宠物业务
     * 返回值: RET_OK, RET_FAIL
     */
    int SetRichFlag2(unsigned int uiUin, unsigned char byFlag, unsigned char byServiceType);

    /**
     * 获取QQ备注名称
     * 参数: uiUin QQ号码，mapRemarkName 返回的备注名称，stContext 登陆态信息
     * 返回值: RET_OK, RET_FAIL
     */
    // 2013年11月28日 21:43:22 shimmeryang: 从pet那边移植过来
    int GetRemarkName(unsigned int uUin, std::map<unsigned int, std::string> &mapRemarkName, const SessionContext& stContext);

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
    //int SendAndRecvOld();

private:
    static COIDBProxyAPI* m_pSingleton;

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
#endif


#ifndef _OIDB_PROXY_DEF_H_
#define _OIDB_PROXY_DEF_H_

#define OIDB_TRANS_PKG_MAGIC "MGCOIDBTRS"
// 2013年11月28日 21:38:07 
// shimmeryang: 现在的好友分组信息已经超过8192个字节了
const int MAX_TRANSPORT_PKG_LEN         = 81920;
const int MAX_TRANSPORT_PKG_BODY_LEN    = MAX_TRANSPORT_PKG_LEN - 5;
const int SSO_PET_ST_APPID = 0x200e01;

typedef struct tagAssistInfo
{
    char abyVersion[2];         //辅助信息结构版本号 		unsigned short
    char szUserName[11];        //调用接口的用户名
    char szPasswd[11];          //调用接口的用户密码
    char abyServiceIP[4];       //前端应用主机IP			unsigned long
    char szServiceName[16];     //前端应用服务名称
    char abyServiceTime[4];     //前端访问接口时间			time_t
    char abyServiceSeq[4];      //前端访问接口流水号		unsigned long
    char cServiceType;          //调用接口的服务类型
    char abyClientIP[4];        //触发前端服务的用户IP		unsigned long
    char szClientName[21];      //触发前端服务的用户名
    char abyClientUin[4];       //触发前端服务的用户UIN		unsigned long
    char abyFlag[4];            //传递标志					long
    char szDesc[30];            //备注说明

} SAssistInfo;

//带登陆态扩展包头，是变长包头(详见下面说明)
typedef struct
{
    unsigned short   ushExLen;                //扩展包头长度(包括shExtLen本身)，必填
    short   shExVer;                //扩展包版本号，初始版本为600，必填
    unsigned int   uiAppID;                //由签名派发方制定，如ptlogin、db等，必填
    char   chKeyType;               //key类型，登陆key类型，见下面解释 必填
    unsigned short   ushSessionKeyLen;         //sessionkey的长度，登陆key 必填
    char   szBuffSessionKey[128];       //sessionkey(最长128字节)，登陆key 必填
    unsigned short   ushReservedLen;                           //保留长度，目前为0
    char   szBuffReserved[64];        //保留字节((最长64字节)
    unsigned short   ushContextDataLen;                        //上下文长度，不需要则填0
    char   szContextData[64];      //上下文数据((最长64字节)，oidb原样返回

} TransPkgHeadExt; //oidb登陆态扩展包头

typedef struct tagTransPkgHeader
{
    char    abyLength[2];
    char    abyVersion[2];
    char    abyCommand[2];
    char    abyUin[4];
    char    cResult;    //11
    SAssistInfo stAssistInfo;
} STransPkgHeader;
//127

typedef struct tagTransPkg
{
    char    cStx;
    STransPkgHeader stHeader;
    unsigned char byExtraFlag; // not serialized
    TransPkgHeadExt stExtraInfo;
    unsigned short ushBodyLen; // not serialized
    char    abyBody[MAX_TRANSPORT_PKG_LEN - 5];
    char    cEtx;
} STransPkg;

#endif


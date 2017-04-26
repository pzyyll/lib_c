/**
 * @file    sns_protocol.h
 * @brief   定义sns project中的各种协议
 * @author  winuxli
 * @date    2010-04-14
 */

#ifndef SNS_PROTOCOL_H
#define SNS_PROTOCOL_H

namespace snslib
{

const int MAX_KEY_LENGTH = 16;

// 全局业务，依次分配
const unsigned short APPID_PCL                = 1;
const unsigned short APPID_WEBPCL             = 2;
const unsigned short APPID_GROUTE             = 3;
const unsigned short APPID_APPROUTE           = 4;
const unsigned short APPID_QQLIST             = 5;
const unsigned short APPID_QQINFO             = 6;
const unsigned short APPID_FLAG	              = 7;

// 业务ID
const unsigned short APPID_APP_TEST           = 100;
const unsigned short APPID_APP_TEST_YY        = 101;
const unsigned short APPID_APP_TNT            = 102;
//const unsigned short APPID_APP_PET_FIGHT      = 103;


typedef struct tagBusHeader
{
    unsigned int uiSrcID;
    unsigned int uiDestID;
    unsigned int uiRouterID;
    unsigned int uiTTL;
    unsigned int uiClientPos;                                  // PCL确定玩家对应socket的索引, 用来在网络不同节点间标示一个完整的连接
}BusHeader;

typedef struct tagAppHeader
{
    unsigned int uiLength;						// 数据长度，不包括appheader
    unsigned int uiUin;							// 玩家uin
    unsigned int uiIP;							// 用户链接IP
    unsigned short ushVersion;					//协议版本
    unsigned short ushZoneID;					//大区ID
    unsigned short ushCmdID;					// 命令字
    unsigned short ushSrcSvrID;                 			// 源SVRID
    unsigned short ushDestSvrID;              			// 目的SVRID
    unsigned short ushSrcMID;					// 源模块ID
    unsigned short ushDestMID;					// 目的模块ID,  值为TYPE_MODULE_TRANSACTION，数据应该以事务方式处理
    unsigned short ushCheckSum;				 // 数据包头+包体的校验位
//    unsigned int uiTransactionID;					// 事务id
    //只有一个TransactionID不能满足两个zonesvr之间的事务处理，修改为src/dest两个. stanleyluo
    unsigned int uiSrcTranID;                   // 源事务ID
    unsigned int uiDestTranID;                  // 目的事务ID
    unsigned short ushStepID;					// 步骤id
    int 			iRet;						// 处理返回值
//  该字段只在PCL和Client交互时使用，不在SNS内部传递
//    char szKey[MAX_KEY_LENGTH];
}AppHeader;

// 事务处理结构定义
typedef struct tagTransactionHeader
{
	unsigned int		uiUin;
	unsigned short 		ushType;				// 修改事务，添加事务
	unsigned short 		ushStepNum;				// 步骤个数
	unsigned int 		uiTransactionID;		// 事务ID，事务处理线程自动分配
	unsigned long long  ullTimerID;				// 定时器ID
	int					iRet;
}TransactionHeader;

typedef struct tagStepHeader
{
	unsigned int 		uiUin;
	unsigned short 		ushID;					// 步骤ID，自动生成，从1开始
	unsigned short 		ushType;				// 请求类型
	unsigned short		ushLevel;				// 请求级别，从1开始，不能间断，同一级别同时发送
	unsigned short 		ushAppID;				// 应用 ID
	unsigned short 		ushSvrID;				// 服务器 ID
	unsigned short 		ushCmd;					// 命令字
	unsigned int		uiParmLen;				// 数据长度，不包括header长度
	unsigned short 		ushProcessFlag;			// 处理标志位
	int					iRet;
}StepHeader;

typedef struct tagSNSQQInfo
{
    unsigned int uiUin;
    char szNickName[16+1];      //QQ昵称
    char cVipFlag;              //粉钻标志位
    char cCFaceFlag;            //自定义头像标志位 0-系统头像 1-自定义头像
    unsigned short ushFaceID;   //系统头像的ID
    unsigned long ulFlag;     //开通关系
    unsigned long ulReserve[3];
}SNSQQInfo;

// 模块类型定义
const unsigned short TYPE_MODULE_APP			=1;			// app应用
const unsigned short TYPE_MODULE_TRANSACTION	=2;			// 事务应用
const unsigned short TYPE_MODULE_LOADER         =3;         //loader
const unsigned short TYPE_MODULE_WRITER         =4;         //writer

// 事务类型定义
const unsigned short TRANSACTION_TYPE_ADD		= 1;		// 添加事务
const unsigned short TRANSACTION_TYPE_MODIFY	= 2;		// 修改事务（Snsapp V2 已经废弃了）
const unsigned short TRANSACTION_TYPE_MIDDLE	= 10;		// 中间处理结果
const unsigned short TRANSACTION_TYPE_END		= 20;		// 处理结束

// 步骤类型定义
const unsigned short STEP_TYPE_APP				=1;			// 应用
const unsigned short STEP_TYPE_LOGIC			=2;			// 逻辑处理

// 步骤级别控制
const unsigned short STEP_LEVEL_AUTO			=1;			// 自动管理级别，默认
const unsigned short STEP_LEVEL_HOLD			=2;			// 和上个级别相同

// 处理标志位定义
const unsigned short STEP_NOT_PROCESS 			= 0;	// 未处理
const unsigned short STEP_SENDED				= 1;	// 消息已经发送
const unsigned short STEP_RECVED				= 2;	// 消息已经接收

// 事务处理结果定义
const int TRANSACTION_RETURN_OK					= 0;
const int TRANSACTION_RETURN_TIMEOUT			= 1;		// 处理超时
const int TRANSACTION_RETURN_SYS_ERROR			= 2;		// 系统错误

//add by jamieli 临时测试使用，具体的CmdID由winuxli分配
const int CMD_ADD_TIMER_REQ = 0x0101;
const int CMD_ADD_TIMER_RSP = 0x0102;
const int CMD_DEL_TIMER_REQ = 0x0103;
const int CMD_TIMER_INFO = 0x0104;  //定时器触发消息
const int CMD_TEST_ECHO = 0x0105;   //回射命令字
const int CMD_TEST_ADDTIMER = 0x0106;   //增加定时器
const int CMD_TEST_ADD_TRANSACTION = 0x0107;   //添加事务
const int CMD_TEST_PUTHDB_DATA = 0x0108;    //向HDB中添加数据，同时测试回写
const int CMD_TEST_GETHDB_DATA = 0x0109;    //从HDB中读取数据
const int CMD_TEST_OUTHDB_DATA = 0x010a;    //从HDB中删除数据

const int CMD_TEST_DELTIMER = 0x010b;       //删除定时器
const int CMD_BENCH_TEST = 0x010c;          //用于压力测试的命令字
const int CMD_TEST_FRIENDLIST = 0x0110;		// 获取好友列表

// LOADER WRITER
const unsigned short CMD_LOAD_DATA = 0x0201;    //加载数据
const unsigned short CMD_WRITE_DATA = 0x0202;   //回写数据
const unsigned short CMD_LAST_WRITE_DATA = 0x0210;	//回写数据

// QQ List
const int CMD_GET_QQLIST = 0x0301;
const int CMD_QQLIST_OIDB = 0x0302;
const int CMD_QQLIST_JITONG = 0x0303;
const int CMD_GET_GROUPED_QQLIST = 0x0304;
const int CMD_GROUPED_QQLIST_OIDB = 0x0305;
const int CMD_CHECK_FRIEND = 0x0306;
const int CMD_CHECK_QQLIST_OIDB = 0x0307;
const int CMD_QQLIST_SYNC = 0x0308;




// QQ INFO
const int CMD_GET_QQINFO= 0x0401;
const int CMD_QQINFO_OIDB = 0x0402;
const int CMD_UPDATE_QQINFO = 0x0403;
const int CMD_QQINFO_SYNC = 0x0404;

// QQ FLAG
const unsigned short CMD_GET_FLAG				= 0x0001;			// 获取标志位
const unsigned short CMD_SET_FLAG				= 0x0002;			// 设置标志位
const unsigned short CMD_DEL_FLAG				= 0x0003;			// 删除标志位

// APP Framework 数据加载返回
const unsigned short CMD_DATA_LOAD_INFO = 0x0501;   //加载数据结果，反馈给APP
const unsigned short CMD_FRAMEWORK_SPEED_MONITOR = 0xFFFF; 	// 监控速度用

// REPLICATION
const unsigned short CMD_REPLICATION_UPDATE = 0x0601;   //复制更新
const unsigned short CMD_REPLICATION_DELETE = 0x0602;   //复制删除

// SNSINFO
const unsigned short CMD_SNSINFO_GET = 0x0701;  //获取SNSINFO中的信息
const unsigned short CMD_SNSINFO_PUT = 0x0702;  //增加SNSINFO中的信息
const unsigned short CMD_SNSINFO_OUT = 0x0703;  //删除SNSINFO中的信息

// LOGCENTRE
const unsigned short CMD_LOGCENTRE_PUT = 0x0801;    //向日志中心发送日志


// PAYSVR
const unsigned short CMD_PAYSVR_REQ = 0x0901;   //PAYSVR购买请求
const unsigned short CMD_PAYSVR_RSP = 0x0902;   //PAYSVR购买应答

// LOCK SVR
const unsigned short CMD_LOCK_GET				= 0x0A01;			// 获取LOCK
const unsigned short CMD_LOCK_SET				= 0x0A02;			// 设置LOCK
const unsigned short CMD_LOCK_DEL				= 0x0A03;			// 删除LOCK

//ROUTER
const unsigned short CMD_SNSROUTER_SET_REQ	= 0x1001;	//router中设置某uin->svr
const unsigned short CMD_SNSROUTER_SET_RSP	= 0x1002;	//router中设置某uin->svr
const unsigned short CMD_SNSROUTER_DEL_REQ    = 0x1003;		//router中删除某uin->svr
const unsigned short CMD_SNSROUTER_DEL_RSP     = 0x1004;		//router中删除某uin->svr
const unsigned short CMD_SNSAPP_HEARTBEAT    = 0x1005;        // snsapp->router心跳

//  PCL的包头
#pragma pack(1)

// PCL二进制包头
typedef struct tagClientHeader{
	unsigned int	uiLength;			// 包长度，包括数据字段
	unsigned int	uiUin;				// 用户UIN
	unsigned short	ushVersion;			// 协议版本号
	unsigned int	uiAppID;			// 应用ID
	unsigned short	ushZoneID;			// 大区ID
	unsigned short	ushCmdID;			// 命令字
	unsigned short	ushCheckSum;		// 数据校验
}ClientHeader;

// WEBPCL传过来的网络序AppHeader
typedef struct tagAppHeader_NetByteOrder
{
    unsigned int   uiLength;					// 数据长度，不包括appheader
    unsigned int   uiUin;						// UIN
	unsigned int   uiIP;						// IP
    unsigned short ushVersion;
    unsigned short ushZoneID;
    unsigned short ushCmdID;					// 命令字
    unsigned short ushSrcSvrID;                 // 源SVRID
    unsigned short ushDestSvrID;                // 目的SVRID
    unsigned short ushSrcMID;					// 源模块ID
    unsigned short ushDestMID;					// 目的模块ID
    unsigned short ushCheckSum;
    unsigned int   uiTransactionID;				// 事务id
    unsigned short ushStepID;					// 步骤id
    int 		   iRet;						// 处理返回值
}AppHeader_NetByteOrder;

#pragma pack()

// PCL执行指令
const unsigned short CMD_PCL_AUTH = 0x0001;
const unsigned short CMD_PCL_TRANSMIT_APP_HEADER = 0x0101;

}

enum
{
    // HTTPPost:
    // HTTPHeader + ProtobufBody
    // TCP:
    // ClientHeader + ProtobufBody
    PCL_BODY_BINARY = 0,
    // HTTPPost:
    // HttpHeader + JsonBody
    // TCP:
    // JsonBody
    PCL_BODY_JSON   = 1,

    // httpget:
    // get /<uri> http/1.0
    PCL_BODY_HTTPGET = 10,
    // httpget:
    // get /crossdomain.xml http/1.0
    // tcp:
    // <policy-file-request/>
    PCL_BODY_POLICY = 11,


    // HTTPGET or HTTPPOST
    PCL_PROTO_HTTP  = 1 << 8,
    // TCP
    PCL_PROTO_TCP   = 3 << 8,
};

enum PlatformType
{
    QQ_PLATFORM = 0,  // QQ/QZ friend list and nickname
    XY_PLATFORM = 1,  // pengyou friend list and pengyou nick
    QG_PLATFORM = 2,  // qqgame friend list and qqgame nick
};


//TT feeds
const int CMD_TTFEEDS = 0x1100; //获取TTFeeds



#endif

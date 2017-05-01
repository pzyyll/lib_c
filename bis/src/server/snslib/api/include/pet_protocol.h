/*
 * FileName:    pet_protocol.h
 * Author:      borisliu
 * Date:        2009-03-09
 * Description: 宠物协议定义文件
 * History:
 *  <author>        <time>          <desc>
 *  borisliu        2009-03-09      创建
 */

#ifndef PET_PROTOCOL_H
#define PET_PROTOCOL_H

#include <time.h>

namespace snslib
{

const int MAX_PET_PKG_LEN = 8192;
const unsigned short MAX_SESSION_LEN		 = 16;
const unsigned short GAME_HEAD_LENGTH       = 26;

/*
 * 内部通信的Bus包头定义

typedef struct tagBusHeader
{
    unsigned int uiSrcID;           // 源BusID
    unsigned int uiDestID;          // 目的BusID
    unsigned int uiRouterID;        // 路由BusID，注意，如果该值为0xFFFFFFFF，表示该包是HeartBeat数据包
    unsigned int uiTTL;             // TTL
    unsigned int uiClientPos;       // ClientPos，WebPCL用来定位链接
} BusHeader;
*/
/*
 * 宠物包头定义
 */
typedef struct tagPetHeader
{
    unsigned short ushLength;       // 宠物包头+包体的长度
    unsigned long long ullPetID;    // 宠物ID
    unsigned short ushVersion;      // 协议版本号
    unsigned char byLangVer;        // 语言版本号
    unsigned short ushCmdID;        // 协议命令字
    unsigned short ushCheckSum;     // 宠物包头+包体的校验和
} PetHeader;

const int PET_HEADER_LEN = 17;      // 宠物包头长度 17字节

/*
 * 内部使用Queue通信的Queue包头定义
 */
typedef struct tagQueueHeader
{
    unsigned short ushCmdID;        // 命令字
    time_t tInTime;                 // 入Queue时间
    unsigned short ushLength;       // 包体长度
    char achReserved[8];            // 保留字段
} QueueHeader;

/*
 * 新加的小游戏的包头
*/
typedef struct tagGameHeader
{
    unsigned short ushCmd;
    unsigned short ushVersion;
    unsigned short ushGameID;
    unsigned int uiSvrID;
    char szSession[MAX_SESSION_LEN + 1];
} GameHeader;
/*
 * 客户端或web与server通信命令字范围规定：
 *
 * game_svr     0x0300-0x03FF
 * feed_svr     0x0500-0x05FF
 * trip_svr     0x0600-0x06FF
 * friend_svr   0x0700-0x07FF
 * friend_svr2  0x0900-0x09FF
 * home_svr     0x0A00-0x0AFF
 * goods_svr    0x1000-0x10FF
 * zone_svr     0x1100-0x11FF
 *
 * 判断一个命令字属于哪个服务，可以用 ushCmdID & CMD_TYPE_MASK，然后与各CMD_TYPE进行对比
 */

// 命令字类型掩码
const unsigned short CMD_TYPE_MASK          = 0xFF00;

const unsigned short CMD_TYPE_GAME_SVR      = 0x0332;
const unsigned short CMD_TYPE_FEED_SVR      = 0x0500;
const unsigned short CMD_TYPE_TRIP_SVR      = 0x0600;
const unsigned short CMD_TYPE_FRIEND_SVR    = 0x0700;
const unsigned short CMD_TYPE_FRIEND_SVR2   = 0x0900;
const unsigned short CMD_TYPE_HOME_SVR      = 0x0A00;
const unsigned short CMD_TYPE_GOODS_SVR     = 0x1000;
const unsigned short CMD_TYPE_ZONE_SVR      = 0x1100;
const unsigned short CMD_TYPE_MSG_PROXY_SVR	= 0x1200;
const unsigned short CMD_TYPE_MSG_EVENT     = 0x1300;
const unsigned short CMD_TYPE_MSG_CENTRE    = 0x7700;

const unsigned short CMD_TYPE_REX_GAME_SVR  = 0xFF00;

// msg_proxy命令字定义
const unsigned short CMD_MSG_PROXY_SEND_MSG		= 0x1201;	// 发消息

// feed_svr 命令字定义
const unsigned short CMD_TYPE_FEEDSVR                   = 0x0500;
const unsigned short CMD_FEED_SSO_LOGIN                 = 0x052F; //SSO登陆
const unsigned short CMD_FEED_LOGIN                     = 0x0530; //登陆
const unsigned short CMD_FEED_LOGOUT                    = 0x0531; //退出
const unsigned short CMD_FEED_GET_PETINFO               = 0x0532; //获取所有属性（客户端使用）
const unsigned short CMD_FEED_GET_ZONE_SHOWINFO         = 0x0533; //获取宠物Zone显示信息，支持批量查询
const unsigned short CMD_FEED_DISCARD                   = 0x0534; //抛弃宠物
const unsigned short CMD_FEED_ENTER_NORMALSTATUS        = 0x0535; //终止打工/学习
const unsigned short CMD_FEED_HELLO                     = 0x0536; //Hello包
const unsigned short CMD_FEED_SET_AUTOFEEDFLAG          = 0x0537; //设置自动喂养标志位
const unsigned short CMD_FEED_FEED                      = 0x0538; //Feed（喂养接口）
const unsigned short CMD_FEED_GET_AREA_INFO             = 0x0539; //获取大区、小区信息
const unsigned short CMD_FEED_GET_ZONE_SHOWINFO2        = 0x053A; //获取宠物Zone显示信息版本2，增加了称号系统
const unsigned short CMD_FEED_SPECIAL_INTERACT          = 0x053B; //特殊的交互信息（用于单人桌面交互）
const unsigned short CMD_FEED_COMM_INTERACT             = 0x053C; //通用交互信息（用于心情动画圈和以后需要实时增加属性的地方）
const unsigned short CMD_FEED_GET_OTHER_PETINFO			= 0x053D; //获取其他企鹅属性

const unsigned short CMD_FEED_ATTR_CHG_NOTIFY           = 0x0550; //属性值变化通知信息
const unsigned short CMD_FEED_STATUS_CHG_NOTIFY         = 0x0551; //状态值变化通知信息
const unsigned short CMD_FEED_NICKNAME_CHG_NOTIFY       = 0x0552; //宠物主人昵称/宠物昵称改变
const unsigned short CMD_FEED_COMBINEPIC_CHG_NOTIFY     = 0x0553; //宠物背景合成图片变化通知
const unsigned short CMD_FEED_URLMSG_NOTIFY             = 0x0554; //带URL广告消息
const unsigned short CMD_FEED_LOGOUT_NOTIFY             = 0x0555; //通知退出
const unsigned short CMD_FEED_UNUSED001                 = 0x0556; //无效命令字
const unsigned short CMD_FEED_CLIENTCONFXML_CHG_NOTIFY  = 0x0557; //客户端配置XML文件版本更新
const unsigned short CMD_FEED_POPPIC_CHG_NOTIFY         = 0x0558; //宠物泡泡装扮ID变化
const unsigned short CMD_FEED_AVATAR_CHG_NOTIFY         = 0x0559; //宠物Avatar的装配版本变化
const unsigned short CMD_FEED_VEHICLE_CHG_NOTIFY        = 0x055A; //交通工具变更通知
const unsigned short CMD_FEED_VIPFLAG_CHG_NOTIFY        = 0x055B; //粉钻标志位变化通知
const unsigned short CMD_FEED_PET_TIPS_NOTIFY           = 0x055C; //下发给宠物的TIPS消息
const unsigned short CMD_FEED_AUTOFEEDFLAG_CHG_NOTIFY   = 0x055D; //自动喂养状态变更通知
const unsigned short CMD_FEED_BUTTON_CHG_NOTIFY         = 0x055E; //图标显示变更通知
const unsigned short CMD_FEED_MISC_INFO_NOTIFY          = 0x055F; //登陆时一些杂项信息、标志位告知
const unsigned short CMD_FEED_NEW_FEEDS_NOTIFY          = 0x0560; //登陆时下发桌面SNS的待浏览Feeds条数
const unsigned short CMD_FEED_CHECK_HELLO_NOTIFY        = 0x0561; //主动要求客户端上发Hello包
const unsigned short CMD_FEED_CLIENT_LOG                = 0x0562; //发送客户端日志

const unsigned short CMD_FEED_WORK                      = 0x0570; //开始打工
const unsigned short CMD_FEED_STUDY                     = 0x0571; //开始学习
const unsigned short CMD_FEED_ENTER_STATUS              = 0x0572; //开始学习
const unsigned short CMD_FEED_WEBGET_PETINFO            = 0x0573; //获取所有属性（WEB使用）
const unsigned short CMD_FEED_UPDATEPET_ATTR            = 0x0574; //修改宠物属性
const unsigned short CMD_FEED_WEBFEED                   = 0x0575; //通过Web喂养
const unsigned short CMD_FEED_UPDATE_SCHOLARSHIP        = 0x0576; //更新奖学金领取标志位
const unsigned short CMD_FEED_UPDATE_COMMUNITYINFO      = 0x0577; //更新社区存储字段
const unsigned short CMD_FEED_GET_AVATARINFO            = 0x0578; //获取Avatar装配方案
const unsigned short CMD_FEED_UPDATE_AVATARINFO         = 0x0579; //保存Avatar装配方案
const unsigned short CMD_FEED_GET_STUDYINFO             = 0x057A; //获取已学课程信息
const unsigned short CMD_FEED_UPDATEPET_SPEC_ATTR       = 0x057B; //修改宠物的一些特殊属性，用于测试不能用于任何业务模块
const unsigned short CMD_FEED_ENTER_TRAVEL              = 0x057C; //进入旅游状态
const unsigned short CMD_FEED_INNER_GET_PETINFO_REQ     = 0x057D; //获取所有属性-请求（不同大区之间查询）
const unsigned short CMD_FEED_INNER_GET_PETINFO_RSP     = 0x057E; //获取所有属性-应答（不同大区之间查询）
const unsigned short CMD_FEED_FROZEN_ACCOUNT            = 0x057F; //冻结、解冻帐户
const unsigned short CMD_FEED_UPDATE_STUDYINFO          = 0x0580; //更新学习的课程信息
const unsigned short CMD_FEED_WEBHELLO                  = 0x0581; //Web侧的Hello包
const unsigned short CMD_FEED_WEBLOGIN                  = 0x0582; //Web侧的Login包
const unsigned short CMD_FEED_UPDATE_PERMISSION         = 0x0583; //更新宠物权限
const unsigned short CMD_FEED_LOADING_REPORT            = 0x0584; //客户端加载方式上报


// trip_svr命令字定义
const unsigned short CMD_TRIP_END                       = 0x0602; //结束旅游


// goods_svr命令字定义


// zone_svr 命令字定义
const unsigned short CMD_ZONE_TIME_TICK         = 0x1101;   // 对时
const unsigned short CMD_ZONE_LOGIN             = 0x1102;   // 登录小区
const unsigned short CMD_ZONE_CHANGE_SCENE      = 0x1103;   // 切换场景
const unsigned short CMD_ZONE_MOVE              = 0x1104;   // 宠物移动
const unsigned short CMD_ZONE_CHAT              = 0x1105;   // 聊天
const unsigned short CMD_ZONE_EMOTION           = 0x1106;   // 表情
const unsigned short CMD_ZONE_SCENE_PET_LIST    = 0x1107;   // 获取场景所有宠物
const unsigned short CMD_ZONE_LOGOUT            = 0x1108;   // 退出小区
const unsigned short CMD_ZONE_GET_POSITION      = 0x1109;   // 获取小区宠物当前位置
const unsigned short CMD_ZONE_JUMP_SCENE        = 0x110A;   // 直接跳到一级场景
const unsigned short CMD_ZONE_SEND_SYS_MSG      = 0x110B;   // 发送系统消息
const unsigned short CMD_ZONE_XOGAME_CHANGESCENE_MSG = 0x110C;  // XO Game场景切换消息
const unsigned short CMD_ZONE_XOGAME_PLAYER_CONTINUE_MSG      = 0x110D;   // 玩家续关消息
const unsigned short CMD_ZONE_XOGAME_QUESTION_MSG      = 0x110E;   // XO Game问题消息
const unsigned short CMD_ZONE_XOGAME_RESULT_MSG      = 0x110F;   // XO Game 答题结果消息
const unsigned short CMD_ZONE_XOGAME_FAILEDLIST_MSG      = 0x1110;   // XO Game 答题结果消息
const unsigned short CMD_ZONE_XOGAME_GAME_READY_MSG = 0x1111; //XO Game准备开始消息
const unsigned short CMD_ZONE_XOGAME_RIGHTLIST_MSG      = 0x1112;   // XO Game 答题结果消息
const unsigned short CMD_ZONE_SKILL_CAST_CONFIG_MSG = 0x1113;    //配置投掷技能
const unsigned short CMD_ZONE_SKILL_CAST_MSG = 0x1114;       //释放投掷技能
const unsigned short CMD_ZONE_SKILL_CAST_RESPONSE_MSG = 0x1115; //释放投掷技能应答
const unsigned short CMD_ZONE_SKILL_CAST_HITLIST_MSG = 0x1116;  //投掷技能命中列表
const unsigned short CMD_ZONE_SKILL_CHANGE_CONFIG_MSG = 0x1117;
const unsigned short CMD_ZONE_SKILL_CHANGE_MSG = 0x1118;
const unsigned short CMD_ZONE_SKILL_CHANGE_STATUS_BC = 0x1119;

const unsigned short CMD_ZONE_BC_PATH           = 0x1120;   // 路径广播
const unsigned short CMD_ZONE_BC_VIEW           = 0x1121;   // 视图变化广播
const unsigned short CMD_ZONE_BC_CHAT           = 0x1122;   // 聊天信息广播
const unsigned short CMD_ZONE_BC_EMOTION        = 0x1123;   // 表情广播
const unsigned short CMD_ZONE_BC_AREA_PETS      = 0x1124;   // 小区宠物信息广播
const unsigned short CMD_ZONE_BC_INFO_VER       = 0x1125;   // 宠物信息版本变化广播
const unsigned short CMD_ZONE_BC_ACTION_PLAY    = 0x1126;   // 广播播放动画消息

const unsigned short CMD_ZONE_SET_CHAT_DENY_TIME    = 0x1130;   // 设置禁言时间

const unsigned short CMD_ZONE_XOGAME2_CHANGESCENE_MSG = CMD_ZONE_XOGAME_CHANGESCENE_MSG;
const unsigned short CMD_ZONE_XOGAME2_PAYOPENGAME = 0x1141;
const unsigned short CMD_ZONE_XOGAME2_CONTINUE_MSG = CMD_ZONE_XOGAME_PLAYER_CONTINUE_MSG;
const unsigned short CMD_ZONE_XOGAME2_BOXLIST_MSG = 0x1143;
const unsigned short CMD_ZONE_XOGAME2_OPENBOX_MSG = 0x1144;
const unsigned short CMD_ZONE_XOGAME2_BOXOPENED_MSG = 0x1145;
const unsigned short CMD_ZONE_XOGAME2_FINISHED_MSG = 0x1146;
const unsigned short CMD_ZONE_XOGAME2_GAME_READY_MSG = 0x1147;


// homesvr 命令字定义
const unsigned short CMD_HOME_GET_HOME_CONFIG           = 0x0A01;
const unsigned short CMD_HOME_POST_POSITION             = 0x0A03;
const unsigned short CMD_HOME_POST_MESSAGE              = 0x0A04;
const unsigned short CMD_HOME_VISIT_HOME                = 0x0A05;
const unsigned short CMD_HOME_INVITE_REQ                = 0x0A06;
const unsigned short CMD_HOME_INVITE_RESP               = 0x0A07;
const unsigned short CMD_HOME_LEAVE                     = 0x0A08;
const unsigned short CMD_HOME_EVICT                     = 0x0A0A;
const unsigned short CMD_HOME_POST_EMOTION              = 0x0A0C;

const unsigned short CMD_HOME_WEB_GET_HOME_INFO         = 0x0A11;
const unsigned short CMD_HOME_WEB_HOLD_PARTY            = 0x0A12;
const unsigned short CMD_HOME_WEB_CHANGE_HOME           = 0x0A13;

const unsigned short CMD_HOME_NOTIFY_HOME_INFO          = 0x0A21;
const unsigned short CMD_HOME_NOTIFY_POSITION_INFO      = 0x0A22;
const unsigned short CMD_HOME_NOTIFY_TALK_MSG           = 0x0A23;
const unsigned short CMD_HOME_NOTIFY_PARTY              = 0x0A24;
const unsigned short CMD_HOME_NOTIFY_GUEST_IN           = 0x0A25;
const unsigned short CMD_HOME_NOTIFY_EMOTION            = 0x0A26;

const unsigned short CMD_HOME_GET_HOME_INFO_2           = 0x0A31;
const unsigned short CMD_HOME_GET_HOME_INFO_3           = 0x0A32;
const unsigned short CMD_HOME_VISIT_HOME_2              = 0x0A33;
const unsigned short CMD_HOME_VISIT_HOME_3              = 0x0A34;
const unsigned short CMD_HOME_VISIT_HOME_4              = 0x0A35;
const unsigned short CMD_HOME_INVITE_RESP_2             = 0x0A36;
const unsigned short CMD_HOME_LEAVE_2                   = 0x0A37;
const unsigned short CMD_HOME_NOTIFY_LEAVE              = 0x0A38;
const unsigned short CMD_HOME_EVICT_2                   = 0x0A39;

const unsigned short CMD_HOME_NOTIFY_LOGIN              = 0x0A51;
const unsigned short CMD_HOME_NOTIFY_LOGOUT             = 0x0A52;


/*
 * 内部Queue通信命令字范围规定：
 *
 * feed_svr         0x0100-0x01FF
 * goods_svr        0x0200-0x02FF
 * pet_loader       0x0300-0x03FF
 * pet_lazywriter   0x0400-0x04FF
 * state_svr        0x0500-0x05FF
 * zone_svr         0x0600-0x06FF
 * msg_proxy        0x0700-0x07FF
 */

const unsigned short QUEUE_CMD_TYPE_FEED_SVR        = 0x0100;
const unsigned short QUEUE_CMD_TYPE_GOODS_SVR       = 0x0200;
const unsigned short QUEUE_CMD_TYPE_PET_LOADER      = 0x0300;
const unsigned short QUEUE_CMD_TYPE_PET_LAZYWRITER  = 0x0400;
const unsigned short QUEUE_CMD_TYPE_STATE_SVR       = 0x0500;
const unsigned short QUEUE_CMD_TYPE_ZONE_SVR        = 0x0600;
const unsigned short QUEUE_CMD_TYPE_MSG_PROXY		= 0x0700;
const unsigned short QUEUE_CMD_TYPE_STAT_MAP_ROUTER = 0x0800;
const unsigned short QUEUE_CMD_TYPE_STAT_MAP_INFO   = 0x0900;
const unsigned short QUEUE_CMD_TYPE_DIR             = 0x0A00;

// feed_svr cmd define
const unsigned short QUEUE_CMD_FEED_MSG	               = 0x0101;    //普通的客户端数据包
const unsigned short QUEUE_CMD_FEED_LOAD_INFO          = 0x0102;    //登陆时，数据LOAD完毕返回的数据包
const unsigned short QUEUE_CMD_FEED_LOAD_RETURN_INFO   = 0x0103;    //查询离线信息时，数据LOAD完毕返回的数据包
const unsigned short QUEUE_CMD_FEED_LOGOUT_IMMEDIATE   = 0x0104;    //及时退出请求包
const unsigned short QUEUE_CMD_FEED_AUTOFEED_CHECK     = 0x0105;    //检查自动喂养是否可以开通的应答

// zone_svr cmd define
const unsigned short QUEUE_CMD_ZONE_MSG	                = 0x0601;
const unsigned short QUEUE_CMD_ZONE_LOGOUT_NOTIFY       = 0x0602;
const unsigned short QUEUE_CMD_ZONE_ATTR_CHG_NOTIFY     = 0x0603;
const unsigned short QUEUE_CMD_ZONE_SYSTEM_MSG          = 0x0604;
const unsigned short QUEUE_CMD_ZONE_USEGOODS_NOTIFY     = 0x0605;

// goodsvr cmd define
const unsigned short QUEUE_CMD_GOODS_MSG	    = 0x0201;

// loader cmd define
const unsigned short QUEUE_CMD_LOADER_LOAD_INFO		    = 0x0301; // load到session里面去，并且在对应的queue中返回是否成功
const unsigned short QUEUE_CMD_LOADER_LOAD_RETURN_INFO   = 0x0302; // 不load到session里面去，直接将得到的宠物信息(不包括物品信息)返回到对应的queue

//lazy writer cmd define
const unsigned short QUEUE_CMD_WRITER_WRITE_IMMEDIATELY = 0x0401; // 以最快的速度将请求的数据回写到数据库里面去，通过这个命令字发送的用户都是要立即踢下线的

// statesvr cmd define
const unsigned short QUEUE_CMD_STATESVR_TIMER_ADDNODE           = 0x0501;
const unsigned short QUEUE_CMD_STATESVR_TIMER_USER_LOGOUT		= 0x0502;
const unsigned short QUEUE_CMD_STATESVR_TIMER_USER_LOGIN        = 0x0503;
const unsigned short QUEUE_CMD_STATESVR_TIMER_DELNODE           = 0x0504;
const unsigned short QUEUE_CMD_STATESVR_TIMER_STATUS_BEGIN		= 0x0510;
const unsigned short QUEUE_CMD_STATESVR_TIMER_STATUS_STOP		= 0x0511;
const unsigned short QUEUE_CMD_STATESVR_TIMER_MSG				= 0x0520;
const unsigned short QUEUE_CMD_STATESVR_TIMER_DELETE_REQ		= 0x0550;

// msgproxy cmd define
const unsigned short QUEUE_CMD_MSG_PROXY_MSG    = 0x0701;

// stat_map_clt_cmd define
const unsigned short QUEUE_CMD_STAT_MAP_ROUTER  = 0x0801;
const unsigned short QUEUE_CMD_STAT_MAP_INFO    = 0x0802;

// Send Svr queue 协议
const unsigned short QUEUE_CMD_SEND_EVENT       = 0x0901;       // 发送事件
const unsigned short QUEUE_CMD_SEND_WL_NOTIFY   = 0x0902;       // 发送给无线宠物网关，让宠物上线或下线
const unsigned short QUEUE_CMD_SEND_SET_TITLE   = 0x0903;       // 设置称号
const unsigned short QUEUE_CMD_SEND_NOTIFY_PETLOGIN = 0x0904;   // 发送宠物登陆成功的通知
const unsigned short QUEUE_CMD_SEND_XOGAME_RANK = 0x0905; //发送XOGame Rank
const unsigned short QUEUE_CMD_SEND_STOP_TRAVEL  = 0x0906; //发送结束旅游状态通知

// dir上报
const unsigned short QUEUE_CMD_DIR_ZONE_INFO_SUBMIT = 0x0A01; // 大区信息上报
const unsigned short QUEUE_CMD_DIR_AREA_INFO_SUBMIT = 0x0A02; // 小区信息上报
const unsigned short QUEUE_CMD_DIR_SUBMIT_TIMER = 0x0A03; // 通知dir定时向统计平台上报大区小区信息

// 贴心宝贝 queue cmd 定义
const unsigned short QUEUE_CMD_AUTOFEED_FEED    = 0x0B01;       // 属性通知
const unsigned short QUEUE_CMD_AUTOFEED_STUDY   = 0x0B02;       // 学习结束通知
const unsigned short QUEUE_CMD_AUTOFEED_CHECK   = 0x0B03;       // 检查是否可以开启贴心宝贝
const unsigned short QUEUE_CMD_AUTOFEED_STATUS  = 0x0B04;		// 状态结束检查
const unsigned short QUEUE_CMD_AUTOFEED_WORK	= 0x0B05;		// 打工结束通知
const unsigned short QUEUE_CMD_AUTOFEED_TRAVEL	= 0x0B06;		// 旅游结束通知
const unsigned short QUEUE_CMD_AUTOFEED_LOGIN	= 0x0B07;		// 宠物登录通知
const unsigned short QUEUE_CMD_AUTOFEED_FLAG	= 0x0B08;		// 自动喂养标记变更

// 家园系统 queue cmd 定义
const unsigned short QUEUE_CMD_HOME_LOAD_HOME_INFO  = 0x0C01;       // 加载家园信息
const unsigned short QUEUE_CMD_HOME_GET_RANDOM_PET  = 0x0C02;       // 获取随机宠物
const unsigned short QUEUE_CMD_HOME_HOLD_PARTY      = 0x0C03;       // 召开派对

// 天气预报查询从客户端传过来的命令字，以及返回给客户端的命令字
const unsigned short CMD_WEATHER_QUERY = 0x1501; // 查询天气情况

// 事件系统协议
const unsigned short CMD_EVENT_PASSPORT			=0x1301;   		// 事件认证通知
const unsigned short CMD_EVENT_CHECK_PASSPORT	=0x1302;		// check认证信息
const unsigned short CMD_EVENT_SET_OPTION		=0x1303;		// 事件操作项

//结婚系统svr
const unsigned short CMD_MARRY_SVR              =0x1400;
const unsigned short CMD_MARRY_PROPOSE          =0x1401; // 结婚函数
const unsigned short CMD_MARRY_QUERY_DIVORCE    =0x1402; // 查询离婚婚记录请求命令
const unsigned short CMD_MARRY_PROCESS_PROPOSE  =0x1403; // 处理求婚被求婚记录请求命令
const unsigned short CMD_MARRY_DIVORCE          =0x1404; // 离婚请求命令
const unsigned short CMD_MARRY_GET_MANIFESTO    =0x1405; // 获取求婚宣言请求命令
const unsigned short CMD_MARRY_MARRYINFO        =0x1406; // 获取宠物数据库信息请求命令
const unsigned short CMD_MARRY_LAYEGG           =0x1407; // 生蛋的请求命令
const unsigned short CMD_MARRY_QUERY_EGG        =0x1408; // 查询宠物蛋的请求命令
const unsigned short CMD_MARRY_PROCESS_EGG      =0x1409; // 处理宠物蛋的请求命令
const unsigned short CMD_MARRY_QUERY_MAIL       =0x1410; // 查询社区邮件的请求命令
const unsigned short CMD_MARRY_PROCESS_MAIL     =0x1411; // 处理社区邮件的请求命令
const unsigned short CMD_MARRY_ADD_LOVE         =0x1412; // 加爱情值的请求命令
const unsigned short CMD_MARRY_GET_GIFT         =0x1413; // 取结婚纪念日礼物

//魔法碰碰球,采用0x0300协议
const unsigned short CMD_BALL_PROTO             =0x0332;

//消息中心专用命令字
const unsigned short CMD_MSG_CENTRE             =0x7700;
const unsigned short CMD_MSG_CENTRE_INFO        =0x7701;    //转发到消息中心的消息
const unsigned short CMD_MSG_CENTRE_ROUTE       =0x7702;    //转发到Router的消息

}

#endif


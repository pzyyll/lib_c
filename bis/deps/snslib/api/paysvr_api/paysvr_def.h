#ifndef PAY_DEFINE_H
#define PAY_DEFINE_H

namespace snslib
{

// 支付渠道定义
const unsigned char PAY_CHANNEL_QQACCT          = 1;    // QB支付
const unsigned char PAY_CHANNEL_QQPOINT         = 2;    // Q点支付
const unsigned char PAY_CHANNEL_QB_QPOINT       = 3;    // 混合支付，先QB再Q点
const unsigned char PAY_CHANNEL_QPOINT_QB       = 4;    // 混合支付, 先Q点再QB
const unsigned char PAY_CHANNEL_TENPAY          = 5;    // 游戏帐户支付

// Item类型定义
const unsigned char PAY_ITEM_TYPE_COMMODITY     = 0;    // 支付商品
const unsigned char PAY_ITEM_TYPE_GOODS         = 1;    // 支付物品
const unsigned char PAY_ITEM_TYPE_HAPPYFIGHT    = 2;    // 支付大乐斗物品
const unsigned char PAY_ITEM_TYPE_KONGFU        = 3;    // 支付功夫物品

// session验证类型定义
const unsigned char PAY_SESSION_QQ_CLIENT       = 1;    // 客户端登录key
const unsigned char PAY_SESSION_COMMUNITY       = 2;    // 社区登录key
const unsigned char PAY_SESSION_QQ_3G           = 3;    // 3G key
const unsigned char PAY_SESSION_NO_SESSION      = 4;    // 无session模式，需要特殊申请

// 属性更新，均为增量
typedef struct tagPayAttr
{
    int     yb;             // 元宝
    int     growth;         // 成长值
    int     starvation;     // 饥饿值
    int     cleanness;      // 清洁值
    short   feeling;        // 心情值
    int     strong;         // 武力值
    int     iq;             // 智力值
    int     charm;          // 魅力值
    int     love;           // 爱情值
} PayAttr;

// 支付商品
typedef struct tagPayItem
{
    unsigned long long itemID;
    unsigned short count;
} PayItem;

const unsigned int PAY_MAX_PAY_MONEY    = 30000;    // 一次支付最多可以支付的金额
const unsigned int PAY_MAX_ITEM_COUNT   = 32;       // 一次支付最多可以支付的物品种类数
const int PAY_MAX_PAY_INFO_LEN          = 256;      // 支付信息最大长度
const int PAY_MAX_PPKINFO_LEN           = 256;      // 扩展信息（暂时只有密保签名用到）
const int PAY_MAX_SESSION_KEY_LEN       = 256;      // session key最大长度

// 支付请求信息
typedef struct tagPayRequest
{
    unsigned char channel;                      // 支付渠道
    unsigned char itemType;                     // Item类型，参考上面的Item类型定义
    unsigned int itemCount;                     // Item种类数
    PayItem items[PAY_MAX_ITEM_COUNT];          // Items
    unsigned int price;                         // 价格,所有支付都由前段批价,必须填写
    char payInfo[PAY_MAX_PAY_INFO_LEN + 1];     // 支付信息,供用户查询,必须规范填写
    unsigned int payUin;                        // 支付UIN
    unsigned int provideUin;                    // 发货UIN
    unsigned long long  providePetID;           // 发货PetID
    unsigned char sessionType;                  // Session类型
    char sessionKey[PAY_MAX_SESSION_KEY_LEN + 1];
    char clientIP[16];      // 客户IP地址
    unsigned char vip;                          // 支付方的粉钻位 0: 非粉钻 1: 粉钻
    char szPPKInfo[PAY_MAX_PPKINFO_LEN + 1];
} PayRequest;

const int PAY_MAX_ANS_DISPLAY_INFO_LEN      = 512;
const int PAY_MAX_ANS_SERIALNO_LEN          = 128;

const int PAY_REQUEST_MIN_LEN = 32;
const int PAY_REQUEST_MAX_LEN = 1200;

// 支付应答信息
typedef struct tagPayAns
{
    int retCode;
    char displayInfo[PAY_MAX_ANS_DISPLAY_INFO_LEN + 1];
    char serialNo[PAY_MAX_ANS_SERIALNO_LEN + 1];
    int money;
    int qb;
    int qpoint;

} PayAns;

// Portal返回码定义
const int PAY_PORTAL_OK                 = 0;          //成功
const int PAY_PORTAL_ERR_PRICE          = -201;       //批价错误
const int PAY_PORTAL_ERR_PROVIDE        = -202;       //发货错误
const int PAY_PORTAL_ERR_NOT_EXIST      = -203;       //帐户不存在
const int PAY_PORTAL_ERR_FREEZE         = -204;       //帐户冻结
const int PAY_PORTAL_ERR_LOSS           = -205;       //帐户挂失
const int PAY_PORTAL_ERR_BALANCE        = -206;       //余额不足
const int PAY_PORTAL_ERR_PAYLIMIT       = -207;       //单笔限制
const int PAY_PORTAL_ERR_DAYLIMIT       = -208;       //单日限制
const int PAY_PORTAL_ERR_TOTAL_ENOUGH   = -209;       //QB或QD余额不足，但可以混合支付
const int PAY_PORTAL_ERR_FRIENDS_7DAY   = -210;       //好友7天限制
const int PAY_PORTAL_ERR_OTHER          = -240;       //其他错误

// 协议头定义
typedef struct tagPayHeader
{
    unsigned short version;
    unsigned short cmd;
    unsigned short payID;
    unsigned short length;
} PayHeader;

const unsigned int PAY_PROTO_HEADER_SIZE         = 8;

// 协议命令字定义
const unsigned short CMD_PAY_PAYITEM        = 0x0001;
const unsigned short CMD_PAY_PAYITEM_ATTR   = 0x0002;

typedef struct tagPayInnerRequest
{
    unsigned long long providePetID;
    unsigned int price;
    unsigned char itemType;
    unsigned int itemCount;
    PayItem items[PAY_MAX_ITEM_COUNT];
} PayInnerRequest;

}

#endif


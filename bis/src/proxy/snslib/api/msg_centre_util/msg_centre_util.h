#ifndef _MSG_CENTRE_UTIL_H_
#define _MSG_CENTRE_UTIL_H_
#include <stdio.h>
#define _MSG_CENTRE_UTIL_H_
#include <stdio.h>
#include <stdarg.h>

#include <string.h>
#include <map>
#include <vector>

#include "comm/util/pet_util.h"
#include "api/include/msg_centre_protocol.h"

namespace snslib
{

//字段的类型
enum
{
    MC_TYPE_NOR = 1,    //NOR类型，表示后面的VAL没有内容
    MC_TYPE_BYTE,       //BYTE类型，表示后面的VAL是一个BYTE
    MC_TYPE_SHORT,      //SHORT类型，表示后面的VAL是一个SHORT
    MC_TYPE_INT,        //INT类型，表示后面的VAL是一个INT
    MC_TYPE_LL,         //LL类型，表示后面的VAL是一个LONGLONG
    MC_TYPE_STR,        //STR类型，表示后面的VAL是一个固定长度的BUFF，存储了可以打印的字符串，长度由FieldMaxLen表示
    MC_TYPE_BIN,        //STR类型，表示后面的VAL是一个固定长度的BUFF，存储了不可以打印的内容，长度由FieldMaxLen表示
};

typedef struct tagMsgCentreMetaData
{
    char szFieldName[64];
    unsigned short ushFieldID;      //TLV中的T定义
    unsigned short ushFieldType;    //字段的类型，支持NOR BYTE SHORT INT LL STR
    unsigned short ushFieldMaxLen;  //TLV中L的最大长度（对于固定长度的类型，表示长度）
}MsgCentreMetaData;

class CMCUtil
{
private:
    static int m_iInitFlag;
    static std::map<unsigned short, MsgCentreMetaData> m_mMsgCentreMetaData;

    static void Init();

public:

    //内部数据结构格式：
    // 总体格式：[20Byte:BusHeader][20Byte:PetHeader][2Byte:MsgCentrePkgNum][MsgCentrePkg_1][MsgCentrePkg_2]……[MsgCentrePkg_n]
    // 单个消息：[4Byte:MagicNum][2Byte:PkgType][2Byte:PkgLen][PkgLen Byte:PkgVal]
    //以下两个函数完成了PetHeader后面所有内容的打包和解包工作

    /**
     * @brief PackMCPkg 将传入的多个MsgCentreMsg打包到指定的Buff中
     * @param pvBuffer 外部分配的存储空间指针
     * @param piPackLen[IN/OUT] 外部分配的存储空间长度，打包时，如果超出该长度，将会失败退出；传出打包到Buff中的长度
     * @param vstMsgCentreMsg 需要打包的消息内容
     * @return 0-成功 其他-失败
     */
    static int PackMCPkg(void* pvBuffer, int *piPackLen, std::vector<MsgCentreMsg> vstMsgCentreMsg);

    /**
     * @brief UnpackMCPkg 将Buff中的数据解包，存放入vstMsgCentreMsg中
     * @param pvBuffer 外部分配的存储空间指针
     * @param piUnpackLen[IN/OUT] 需要解包的长度；传出实际解包的长度
     * @param vstMsgCentreMsg 解包以后的信息
     * @return 0-成功 其他-失败
     */
    static int UnpackMCPkg(void* pvBuffer, int *piUnpackLen, std::vector<MsgCentreMsg> &vstMsgCentreMsg);

    /**
     * @brief 将数据格式化输出到字符串中
     */
    static const char *FormatMCPkg(std::vector<MsgCentreMsg> &vstMsgCentreMsg);

    CMCUtil()
    {
    }

    ~CMCUtil()
    {
    }

};
}
#endif

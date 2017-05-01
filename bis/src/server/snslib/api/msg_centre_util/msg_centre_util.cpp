#include "api/msg_centre_util/msg_centre_util.h"
using namespace snslib;

//静态成员初始化
int CMCUtil::m_iInitFlag = 0;
std::map<unsigned short, MsgCentreMetaData> CMCUtil::m_mMsgCentreMetaData;

MsgCentreMetaData g_stMsgCentreMetaData[]=
{
    {"M_PetName",     MCID_MODIFY_PETNAME,       MC_TYPE_STR,        16},
    {"M_QQName",      MCID_MODIFY_QQNAME,        MC_TYPE_STR,        16},
    {"M_VipFlag",     MCID_MODIFY_VIPFLAG,       MC_TYPE_BYTE,       1},
    {"M_PetLevel",    MCID_MODIFY_PETLEVEL,      MC_TYPE_SHORT,      2},
    {"M_Permission",  MCID_MODIFY_PERMISSION,    MC_TYPE_INT,        4},
    {"M_TitleID",     MCID_MODIFY_TITLEID,       MC_TYPE_SHORT,      2},
    {"M_PetStatus",   MCID_MODIFY_PETSTATUS,     MC_TYPE_SHORT,      2},
    {"M_AvatarVer",   MCID_MODIFY_AVATARVER,     MC_TYPE_SHORT,      2},
    {"M_PetHealth",   MCID_MODIFY_PETHEALTH,     MC_TYPE_SHORT,      2},
    {"M_AvatarInfo",  MCID_MODIFY_AVATARINFO,    MC_TYPE_BIN,        40},
    {"M_Lovingness",  MCID_MODIFY_LOVINGNESS,    MC_TYPE_INT,        4},
    {"M_ZoneID",      MCID_MODIFY_ZONEID,        MC_TYPE_SHORT,      2},
    {"M_AreaID",      MCID_MODIFY_AREAID,        MC_TYPE_SHORT,      2},
    {"M_YuanBao",     MCID_MODIFY_YUANBAO,       MC_TYPE_INT,        4},
    {"M_Growth",      MCID_MODIFY_GROWTH,        MC_TYPE_INT,        4},
    {"M_OnlineTime",  MCID_MODIFY_ONLINETIME,    MC_TYPE_INT,        4},
	{"M_VipLevel",    MCID_MODIFY_VIPLEVEL,      MC_TYPE_SHORT,      2},
	{"M_VipYearFlag", MCID_MODIFY_VIPYEARFLAG,   MC_TYPE_BYTE,       1},

    {"S_PetName",     MCID_SYNC_PETNAME,       MC_TYPE_STR,        16},
    {"S_QQName",      MCID_SYNC_QQNAME,        MC_TYPE_STR,        16},
    {"S_VipFlag",     MCID_SYNC_VIPFLAG,       MC_TYPE_BYTE,       1},
    {"S_PetLevel",    MCID_SYNC_PETLEVEL,      MC_TYPE_SHORT,      2},
    {"S_Permission",  MCID_SYNC_PERMISSION,    MC_TYPE_INT,        4},
    {"S_TitleID",     MCID_SYNC_TITLEID,       MC_TYPE_SHORT,      2},
    {"S_PetStatus",   MCID_SYNC_PETSTATUS,     MC_TYPE_SHORT,      2},
    {"S_AvatarVer",   MCID_SYNC_AVATARVER,     MC_TYPE_SHORT,      2},
    {"S_VipLevel",    MCID_SYNC_VIPLEVEL,      MC_TYPE_SHORT,      2},

    {"PetLoadNode", MCID_PETLOADNODE,   MC_TYPE_INT,        4},
    {"PetDelNode",  MCID_PETDELNODE,    MC_TYPE_INT,        4},
    {"PetRegist",   MCID_PETREGIST,     MC_TYPE_NOR,        0},
    {"PetDiscard",  MCID_PETDISCARD,    MC_TYPE_NOR,        0},
    {"PetLogin",    MCID_PETLOGIN,      MC_TYPE_NOR,        0},
    {"PetLogout",   MCID_PETLOGOUT,     MC_TYPE_NOR,        0},

    //任务系统添加的消息
    {"FishGet",   MCID_FISH_GET,     MC_TYPE_BIN,         8},
    {"FarmGet",   MCID_FARM_GET,     MC_TYPE_BIN,         8},
	{"NpcTalk",   MCID_NPC_TALK,     MC_TYPE_BIN,		  8},
    {"GameSucc",  MCID_NPC_GAMESUCC, MC_TYPE_BIN,         8},

};

void CMCUtil::Init()
{
    if (m_iInitFlag == 0)
    {
        m_mMsgCentreMetaData.clear();
        int iMCMetaDataNum = sizeof(g_stMsgCentreMetaData)/sizeof(MsgCentreMetaData);
        for(int i=0; i<iMCMetaDataNum; i++)
        {
            m_mMsgCentreMetaData.insert(std::pair<unsigned short, MsgCentreMetaData>(g_stMsgCentreMetaData[i].ushFieldID, g_stMsgCentreMetaData[i]));
        }

        m_iInitFlag = 1;
    }
}
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
int CMCUtil::PackMCPkg(void* pvBuffer, int *piPackLen, std::vector<MsgCentreMsg> vstMsgCentreMsg)
{
    Init();
    int iOffSet = 0;
    int iMsgCentreMsgNum = 0;

    if ((pvBuffer == NULL)||(*piPackLen < 2)||vstMsgCentreMsg.size() == 0)
    {
        //传入参数错误
        return -1;
    }

    iMsgCentreMsgNum = vstMsgCentreMsg.size();

    iOffSet += CBuffTool::WriteShort((char *)pvBuffer + iOffSet, (unsigned short)iMsgCentreMsgNum);

    for(int i=0; i<iMsgCentreMsgNum; i++)
    {
        int iMsgReqLen = 8;     //消息填充到Buff里面需要的长度
        int iBuffRemainLen = *piPackLen - iOffSet;  //Buff剩余的空间

        std::map<unsigned short, MsgCentreMetaData>::iterator pstMsgCentreMetaData = m_mMsgCentreMetaData.find(vstMsgCentreMsg[i].ushMsgType);
        if (pstMsgCentreMetaData == m_mMsgCentreMetaData.end())
        {
            //没有找到对应的消息类型
            return -2;
        }

        iMsgReqLen += pstMsgCentreMetaData->second.ushFieldMaxLen;

        if (iBuffRemainLen < iMsgReqLen)
        {
            //BUFF长度不够
            return -3;
        }

        iOffSet += CBuffTool::WriteInt((char *)pvBuffer + iOffSet, MC_MSG_MAGIC_NUM);
        iOffSet += CBuffTool::WriteShort((char *)pvBuffer + iOffSet, vstMsgCentreMsg[i].ushMsgType);

        if ((((pstMsgCentreMetaData->second.ushFieldType == MC_TYPE_STR)||(pstMsgCentreMetaData->second.ushFieldType == MC_TYPE_BIN))&&(vstMsgCentreMsg[i].ushMsgValLen > pstMsgCentreMetaData->second.ushFieldMaxLen))
                ||(((pstMsgCentreMetaData->second.ushFieldType != MC_TYPE_STR)&&((pstMsgCentreMetaData->second.ushFieldType != MC_TYPE_BIN))) && (vstMsgCentreMsg[i].ushMsgValLen != pstMsgCentreMetaData->second.ushFieldMaxLen)))
        {
            //字段长度有问题
            return -4;
        }

        iOffSet += CBuffTool::WriteShort((char *)pvBuffer + iOffSet, vstMsgCentreMsg[i].ushMsgValLen);
        switch(pstMsgCentreMetaData->second.ushFieldType)
        {
            case MC_TYPE_BYTE:
            case MC_TYPE_STR:
            case MC_TYPE_LL:
            case MC_TYPE_BIN:
            {
                //不需要转换网络序，直接写BUFF
                iOffSet += CBuffTool::WriteString((char *)pvBuffer + iOffSet, vstMsgCentreMsg[i].szMsgVal, vstMsgCentreMsg[i].ushMsgValLen);
                break;
            }
            case MC_TYPE_SHORT:
            {
                //需要转换网络序
                iOffSet += CBuffTool::WriteShort((char *)pvBuffer + iOffSet, *((short *)vstMsgCentreMsg[i].szMsgVal));
                break;
            }

            case MC_TYPE_INT:
            {
                //需要转换网络序
                iOffSet += CBuffTool::WriteInt((char *)pvBuffer + iOffSet, *((int *)vstMsgCentreMsg[i].szMsgVal));
                break;
            }
            case MC_TYPE_NOR:
            {
                break;
            }
            default:
            {
                //字段类型有问题
                return -5;
                break;
            }
        }

    }

    *piPackLen = iOffSet;

    return 0;
}

/**
 * @brief UnpackMCPkg 将Buff中的数据解包，存放入vstMsgCentreMsg中
 * @param pvBuffer 外部分配的存储空间指针
 * @param piUnpackLen[IN/OUT] 需要解包的长度；传出实际解包的长度
 * @param vstMsgCentreMsg 解包以后的信息
 * @return 0-成功 其他-失败
 */
int CMCUtil::UnpackMCPkg(void* pvBuffer, int *piUnpackLen, std::vector<MsgCentreMsg> &vstMsgCentreMsg)
{
    Init();

    int iOffSet = 0;

    short shMsgCentreMsgNum = 0;

    int iMCMagicNum = 0;
    MsgCentreMsg stMsgCentreMsg;

    if ((pvBuffer == NULL)||(*piUnpackLen < 2))
    {
        //传入参数错误
        return -1;
    }

    iOffSet += CBuffTool::ReadShort((char *)pvBuffer + iOffSet, shMsgCentreMsgNum);

    if ((shMsgCentreMsgNum < 0)||(shMsgCentreMsgNum > 100))
    {
        return -2;
    }

    vstMsgCentreMsg.clear();

    for(int i=0; i<shMsgCentreMsgNum; i++)
    {
        int iMsgReqLen = 8; //下一个消息需要的长度
        int iBuffRemainLen = *piUnpackLen - iOffSet;    //剩余未解部分的长度

        if (iBuffRemainLen < iMsgReqLen)
        {
            //剩余长度不够解
            return -3;
        }

        memset(&stMsgCentreMsg, 0x0, sizeof(stMsgCentreMsg));

        iOffSet += CBuffTool::ReadInt((char *)pvBuffer + iOffSet, iMCMagicNum);
        if (iMCMagicNum != MC_MSG_MAGIC_NUM)
        {
            //MAGICNUM不正确
            return -4;
        }

        iOffSet += CBuffTool::ReadShort((char *)pvBuffer + iOffSet, stMsgCentreMsg.ushMsgType);
        iOffSet += CBuffTool::ReadShort((char *)pvBuffer + iOffSet, stMsgCentreMsg.ushMsgValLen);

        std::map<unsigned short, MsgCentreMetaData>::iterator pstMsgCentreMetaData = m_mMsgCentreMetaData.find(stMsgCentreMsg.ushMsgType);
        if (pstMsgCentreMetaData == m_mMsgCentreMetaData.end())
        {
            //没有找到对应的消息类型
            return -5;
        }

        if ((((pstMsgCentreMetaData->second.ushFieldType == MC_TYPE_STR)||(pstMsgCentreMetaData->second.ushFieldType == MC_TYPE_BIN))&&(stMsgCentreMsg.ushMsgValLen > pstMsgCentreMetaData->second.ushFieldMaxLen))
            ||(((pstMsgCentreMetaData->second.ushFieldType != MC_TYPE_STR)&&(pstMsgCentreMetaData->second.ushFieldType != MC_TYPE_BIN)) && (stMsgCentreMsg.ushMsgValLen != pstMsgCentreMetaData->second.ushFieldMaxLen)))
        {
            //字段长度有问题
            return -6;
        }

        iMsgReqLen += stMsgCentreMsg.ushMsgValLen;
        if (iBuffRemainLen < iMsgReqLen)
        {
            //剩余长度不够解
            return -7;
        }

        switch(pstMsgCentreMetaData->second.ushFieldType)
        {
            case MC_TYPE_BYTE:
            case MC_TYPE_STR:
            case MC_TYPE_BIN:
            case MC_TYPE_LL:
            {
                //不需要转换网络序，直接写BUFF
                iOffSet += CBuffTool::ReadString((char *)pvBuffer + iOffSet, stMsgCentreMsg.szMsgVal, stMsgCentreMsg.ushMsgValLen);
                break;
            }
            case MC_TYPE_SHORT:
            {
                //需要转换网络序
                unsigned short ushVal;
                iOffSet += CBuffTool::ReadShort((char *)pvBuffer + iOffSet, ushVal);
                memcpy(stMsgCentreMsg.szMsgVal, &ushVal, sizeof(ushVal));
                break;
            }

            case MC_TYPE_INT:
            {
                //需要转换网络序
                unsigned int uiVal;
                iOffSet += CBuffTool::ReadInt((char *)pvBuffer + iOffSet, uiVal);
                memcpy(stMsgCentreMsg.szMsgVal, &uiVal, sizeof(uiVal));
                break;
            }
            case MC_TYPE_NOR:
            {
                break;
            }
            default:
            {
                //字段类型有问题
                return -8;
                break;
            }
        }

        vstMsgCentreMsg.push_back(stMsgCentreMsg);

    }

    *piUnpackLen = iOffSet;

    return 0;
}

const char *CMCUtil::FormatMCPkg(std::vector<MsgCentreMsg> &vstMsgCentreMsg)
{
    Init();

    static char szMCPkgOutputStr[10240];

    int iOffSet = 0;
    std::vector<MsgCentreMsg>::iterator pstMCPkg = vstMsgCentreMsg.begin();

    for (pstMCPkg = vstMsgCentreMsg.begin();pstMCPkg != vstMsgCentreMsg.end();pstMCPkg++)
    {
        std::map<unsigned short, MsgCentreMetaData>::iterator pstMsgCentreMetaData = m_mMsgCentreMetaData.find(pstMCPkg->ushMsgType);
        if (pstMsgCentreMetaData == m_mMsgCentreMetaData.end())
        {
            //没有找到对应的消息类型
            iOffSet+=snprintf(szMCPkgOutputStr+iOffSet, sizeof(szMCPkgOutputStr)-iOffSet, "[%d|UNKNOW_MSG]", pstMCPkg->ushMsgType);
            continue;
        }

        switch(pstMsgCentreMetaData->second.ushFieldType)
        {
            case MC_TYPE_NOR:
            {
                iOffSet+=snprintf(szMCPkgOutputStr+iOffSet, sizeof(szMCPkgOutputStr)-iOffSet, "[%s|%d|%d|]", pstMsgCentreMetaData->second.szFieldName, pstMCPkg->ushMsgType, pstMCPkg->ushMsgValLen);
                break;
            }
            case MC_TYPE_BYTE:
            {
                iOffSet+=snprintf(szMCPkgOutputStr+iOffSet, sizeof(szMCPkgOutputStr)-iOffSet, "[%s|%d|%d|%d]", pstMsgCentreMetaData->second.szFieldName, pstMCPkg->ushMsgType, pstMCPkg->ushMsgValLen, *((unsigned char *)pstMCPkg->szMsgVal));
                break;
            }
            case MC_TYPE_SHORT:
            {
                iOffSet+=snprintf(szMCPkgOutputStr+iOffSet, sizeof(szMCPkgOutputStr)-iOffSet, "[%s|%d|%d|%d]", pstMsgCentreMetaData->second.szFieldName, pstMCPkg->ushMsgType, pstMCPkg->ushMsgValLen, *((unsigned short *)pstMCPkg->szMsgVal));
                break;
            }
            case MC_TYPE_INT:
            {
                iOffSet+=snprintf(szMCPkgOutputStr+iOffSet, sizeof(szMCPkgOutputStr)-iOffSet, "[%s|%d|%d|%d]", pstMsgCentreMetaData->second.szFieldName, pstMCPkg->ushMsgType, pstMCPkg->ushMsgValLen, *((unsigned int *)pstMCPkg->szMsgVal));
                break;
            }
            case MC_TYPE_LL:
            {
                iOffSet+=snprintf(szMCPkgOutputStr+iOffSet, sizeof(szMCPkgOutputStr)-iOffSet, "[%s|%d|%d|%llu]", pstMsgCentreMetaData->second.szFieldName, pstMCPkg->ushMsgType, pstMCPkg->ushMsgValLen, *((unsigned long long *)pstMCPkg->szMsgVal));
                break;
            }
            case MC_TYPE_STR:
            {
                iOffSet+=snprintf(szMCPkgOutputStr+iOffSet, sizeof(szMCPkgOutputStr)-iOffSet, "[%s|%d|%d|%s]", pstMsgCentreMetaData->second.szFieldName, pstMCPkg->ushMsgType, pstMCPkg->ushMsgValLen, pstMCPkg->szMsgVal);
                break;
            }
            case MC_TYPE_BIN:
            {
                iOffSet+=snprintf(szMCPkgOutputStr+iOffSet, sizeof(szMCPkgOutputStr)-iOffSet, "[%s|%d|%d|%s]", pstMsgCentreMetaData->second.szFieldName, pstMCPkg->ushMsgType, pstMCPkg->ushMsgValLen, CStrTool::Str2Hex(pstMCPkg->szMsgVal, pstMCPkg->ushMsgValLen));
                break;
            }

            default:
            {
                iOffSet+=snprintf(szMCPkgOutputStr+iOffSet, sizeof(szMCPkgOutputStr)-iOffSet, "[%d|%d|UNKNOW_FIELD]", pstMCPkg->ushMsgType, pstMsgCentreMetaData->second.ushFieldType);
                break;
            }
        }
    }

    return szMCPkgOutputStr;
}


#ifndef _BUS_API_H_
#define _BUS_API_H_

#include "comm/util/pet_util.h"
#include "api/include/sns_protocol.h"

namespace snslib
{

const unsigned int ROUTER_ID_FOR_HEART_BEAT = 0xFFFFFFFF;

class CBusApi
{
public:
    const static int SUCCESS = 0;
    const static int ERROR = -1;

    const static int ERR_BASE = -200;
    const static int CONF_FILE_NOT_VALID = ERR_BASE - 1;
    const static int CONFIG_ITEM_ERR = ERR_BASE - 2;
    const static int TBUS_INIT_FAILED = ERR_BASE - 3;
    const static int TBUS_SEND_FAILED = ERR_BASE - 4;
    const static int TBUS_RECV_FAILED = ERR_BASE - 5;
    const static int BUS_FULL = ERR_BASE - 6;
    const static int BUS_EMPTY = ERR_BASE - 7;
    const static int RECV_BUFF_TOO_SMALL = ERR_BASE - 8;
    const static int SEND_PKG_TOO_LARGE = ERR_BASE - 9;
    const static int TBUS_NO_CHANNEL_MATCHED = ERR_BASE - 10;
    const static int TBUS_PEEK_FAILED = ERR_BASE - 11;
    const static int TBUS_POP_FAILED = ERR_BASE - 12;

    const static int CLUSTER_BUS_MAX = 20;  //可以热备的BUS数量
    const static int ONE_CLUSTER_BUS_NUM_MAX = 20;  //每个热备BUS中BusID的最多数量

    typedef struct tagClusterBusInfo
    {
        char szModuleName[64];
        unsigned int uiBusDst;
        int iBusIDNum;
        unsigned int auiBusID[ONE_CLUSTER_BUS_NUM_MAX];
        unsigned int uiLastActiveBusID;
    }ClusterBusInfo;

public:
    CBusApi();
    ~CBusApi();

    /**
     * @brief 初始化BUS接口，通过配置文件初始化
     */
    int Init(const char *pszConfFile, unsigned int uiProcID);

    /**
     * @brief 通过BUS发送数据
     * @param:uiDstBusID 目标BUSID，如果是发送到Router，但是不知道发送到哪一台Router，可以填4.0.0.0
     * @param:pvData 需要发送数据的Buff
     * @param:iDataLen 需要发送数据的长度
     * @param:iFlag 暂时不使用
     *
     * @note:
     * 1)就算你知道发送到哪一台Router，由于Bus接口的配置文件中配置了Router接口使用热备的模式工作，该接口也会将uiDstBusID替换为最后一次收到数据的那个Router
     */
    int Send(unsigned int uiDstBusID, const void *pvData, int iDataLen, int iFlag = 0);

    /**
     * @brief 通过BUS接收数据
     * @param:puiSrcBusID 源BUSID
     * @param:pvData 需要接收数据的缓冲区，需要有外部分配
     * @param:piDataLen 存储区的长度，同时作为接收到数据包长度的返回值
     * @param:iFlag 暂时不使用
     *
     * @note:
     * 1)该接口逻辑中将对Router的检测包进行处理，Router的检测包不会返回给用户
     */
    int Recv(unsigned int *puiSrcBusID, void *pvData, int *piDataLen, int iFlag = 0);

    /**
     * @brief 通过BUS接收数据,数据任然保存在bus中，需要调用Pop接口清掉数据
     * @param:puiSrcBusID 源BUSID
     * @param:puiDstBUsID 目的BusID
     * @param:pvData 需要接收数据的缓冲区，需要有外部分配
     * @param:piDataLen 存储区的长度，同时作为接收到数据包长度的返回值
     * @param:iFlag 暂时不使用
     *
     * @note:
     * 1)该接口逻辑中将对Router的检测包进行处理，Router的检测包不会返回给用户
     */
    int Peek(unsigned int *puiSrcBusID, unsigned int *puiDstBusID, void **ppvData, int *piDataLen, int iFlag = 0);

    /**
     * @brief 移除Bus中的第一个数据包。
     * @param:uiSrcBusID 源BUSID
     * @param:uiDstBusID 目的BusID
     */
    int Pop(unsigned int uiSrcBusID, unsigned uiDstBusID);

    /**
         * @brief 自动选择BUS通道发送数据。根据dst id判断，如果dst id为0或者router的bus id则发给router，否则根据真实dst id转发
         * @param:pvData 需要发送数据的Buff
         * @param:iDataLen 需要发送数据的长度
         */
    int AutoSend(void *pvData, int iDataLen);

    /**
     * @brief 将单个数据包发送到所有的Router
     * @param:pvData 需要发送数据的Buff
     * @param:iDataLen 需要发送数据的长度
     * @param:iFlag 暂时不使用
     *
     * @note BUSHEADER中的RouterID会被修改为特定Router的BUSID
     */
    int SendToAllRouter(void *pvData, int iDataLen, int iFlag = 0);

    /**
     * @brief 将单个数据包直接发送到所有的Router
     * @param:pvData 需要发送数据的Buff
     * @param:iDataLen 需要发送数据的长度
     * @param:iFlag 暂时不使用
     *
     * @note BUSHEADER中的RouterID不会被修改，由于心跳包发送的时候，以ROUTERID为特殊标志发送的
     */
    int SendToAllRouterDirect(void *pvData, int iDataLen, int iFlag = 0);

    unsigned int GetProcID()
    {
        return m_uiProcID;
    }

    const char *GetErrMsg()
    {
        return m_szErrMsg;
    }

private:
    int m_iGCIMKey;
    unsigned int m_uiProcID;
    unsigned int m_uiRouterBusID;
    unsigned int m_auiAllRouterBusID[ONE_CLUSTER_BUS_NUM_MAX];
    int m_iRouterBusNum;

    int m_iClusterBusNum;
    ClusterBusInfo m_astClusterBusInfo[CLUSTER_BUS_MAX];

    int m_iBusHandle;
    char m_szErrMsg[256];
    int m_iInitFlag;

    BusHeader *m_pstBusHeader;
};

}
#endif



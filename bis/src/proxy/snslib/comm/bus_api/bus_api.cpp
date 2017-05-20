#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

#include "comm/bus_api/bus_api.h"
#include "comm/ini_file/ini_file.h"

#include "comm/log/pet_log.h"

#include "tbus/tbus.h"
#include "tbus/tbus_error.h"

using namespace snslib;

CBusApi::CBusApi()
{
    m_iGCIMKey = 0;
    m_uiProcID = 0;

    m_iClusterBusNum = 0;
    memset(m_astClusterBusInfo, 0x0, sizeof(m_astClusterBusInfo));

    m_iBusHandle = 0;
    memset(m_szErrMsg, 0x0, sizeof(m_szErrMsg));

    m_iInitFlag = 0;

    memset(m_auiAllRouterBusID, 0x0, sizeof(m_auiAllRouterBusID));
    m_iRouterBusNum = 0;
}

CBusApi::~CBusApi()
{
    if (m_iInitFlag == 1)
    {
        tbus_delete(&m_iBusHandle);
    }
}

/**
 * @brief 初始化BUS接口，通过配置文件初始化
 */
int CBusApi::Init(const char *pszConfFile, unsigned int uiProcID)
{
    int iRetVal = 0;
    CIniFile objIniFile(pszConfFile);
    char szSecName[64] = {0};
    char szSecVal[4096] = {0};
    char szRouterBusID[16] = {0};

    if (objIniFile.IsValid())
    {
        objIniFile.GetInt("BUS_API", "GCIMKey", 0, &m_iGCIMKey);
        objIniFile.GetInt("BUS_API", "ClusterBusNum", 0, &m_iClusterBusNum);
        objIniFile.GetString("BUS_API", "RouterBusID", "", szRouterBusID, sizeof(szRouterBusID));

        m_uiRouterBusID = inet_addr(szRouterBusID);
        if (m_uiRouterBusID == 0)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "RouterBusID[%s] is not valid", szRouterBusID);
            return TBUS_INIT_FAILED;
        }

        if (m_iClusterBusNum > CLUSTER_BUS_MAX)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config item %s/%s[%d] is not valid", "BUS_API", "BusDst", m_iClusterBusNum);
            return CONFIG_ITEM_ERR;
        }

        PetLog(0, 0, PETLOG_DEBUG, "CBusApi|%s|%d ClusterBusNum=%d", __FUNCTION__, __LINE__, m_iClusterBusNum);

        for(int i=0; i<m_iClusterBusNum; i++)
        {
            snprintf(szSecName, sizeof(szSecName), "CLUSTER_BUS_%d", i+1);
            objIniFile.GetString(szSecName, "ModuleName", "", m_astClusterBusInfo[i].szModuleName, sizeof(m_astClusterBusInfo[i].szModuleName));
            objIniFile.GetString(szSecName, "BusDst", "", szSecVal, sizeof(szSecVal));
            if (szSecVal[0] == '\0')
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config item %s/%s is null", szSecName, "BusDst");
                return CONFIG_ITEM_ERR;
            }
            if (inet_aton(szSecVal, (struct in_addr *)&m_astClusterBusInfo[i].uiBusDst) == 0)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config item %s/%s[%s] is not valid", szSecName, "BusDst", szSecVal);
                return CONFIG_ITEM_ERR;
            }
            if (m_astClusterBusInfo[i].uiBusDst == 0)
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config item %s/%s[%s] is not valid", szSecName, "BusDst", szSecVal);
                return CONFIG_ITEM_ERR;
            }

            m_astClusterBusInfo[i].uiLastActiveBusID = 0;

            objIniFile.GetString(szSecName, "BusIDList", "", szSecVal, sizeof(szSecVal));
            if (szSecVal[0] == '\0')
            {
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config item %s/%s is null", szSecName, "BusIDList");
                return CONFIG_ITEM_ERR;
            }

            int iCurBusIDNum = 0;
            char *pszOneBusID = NULL;
            for(char *pszSecVal = szSecVal; (pszOneBusID = strtok(pszSecVal, ",")) != NULL; pszSecVal=NULL)
            {
                m_astClusterBusInfo[i].auiBusID[iCurBusIDNum] = inet_addr(pszOneBusID);
                if (m_astClusterBusInfo[i].auiBusID[iCurBusIDNum] == 0)
                {
                    snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config item %s/BusIDList [%s] is not valid", szSecName, szSecVal);
                    return CONFIG_ITEM_ERR;
                }

                iCurBusIDNum++;

                if (iCurBusIDNum >= ONE_CLUSTER_BUS_NUM_MAX)
                {
                    snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config item %s/BusIDList [%s] is not valid, id_num [%d] is too big", szSecName, szSecVal, iCurBusIDNum);
                    return CONFIG_ITEM_ERR;
                }
            }

            m_astClusterBusInfo[i].iBusIDNum = iCurBusIDNum;

            if ((m_uiRouterBusID&(htonl(0xFFFFFF00))) == m_astClusterBusInfo[i].uiBusDst)
            {
                PetLog(0, 0, PETLOG_DEBUG, "CBusApi|%s|%d ROUTER_CLUSTER|0x%08X", __FUNCTION__, __LINE__, m_astClusterBusInfo[i].uiBusDst);

                for(int j=0; j<m_astClusterBusInfo[i].iBusIDNum; j++)
                {
                    m_auiAllRouterBusID[j]=m_astClusterBusInfo[i].auiBusID[j];
                }

                m_iRouterBusNum = m_astClusterBusInfo[i].iBusIDNum;
            }

            PetLog(0, 0, PETLOG_DEBUG, "CBusApi|%s|%d CLUSTER_BUS[%d]|DST|0x%08X", __FUNCTION__, __LINE__, i, m_astClusterBusInfo[i].uiBusDst);
            for(int k=0; k<m_astClusterBusInfo[i].iBusIDNum; k++)
            {
                PetLog(0, 0, PETLOG_DEBUG, "CBusApi|%s|%d CLUSTER_BUS[%d]|BUSID[%d]|0x%08X", __FUNCTION__, __LINE__, i, k, m_astClusterBusInfo[i].auiBusID[k]);
            }

        }
    }
    else
    {
        return CONF_FILE_NOT_VALID;
    }

    if (m_iRouterBusNum == 0)
    {
        //对于RouterBUS没有配置到Cluster里面的情况
        m_auiAllRouterBusID[0] = m_uiRouterBusID;
        m_iRouterBusNum = 1;
    }

    iRetVal = tbus_init(m_iGCIMKey, 0);
    if (iRetVal != TBUS_SUCCESS)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus_init(%d, 0) failed, ret=%d", m_iGCIMKey, iRetVal);
        return TBUS_INIT_FAILED;
    }

    iRetVal = tbus_new(&m_iBusHandle);
    if (iRetVal != TBUS_SUCCESS)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus_new failed, ret=%d", iRetVal);
        return TBUS_INIT_FAILED;
    }

    m_iInitFlag = 1;

    iRetVal = tbus_bind(m_iBusHandle, uiProcID);
    if (iRetVal != TBUS_SUCCESS)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus_bind(%d, 0x%x) failed, ret=%d, err=%s", m_iBusHandle, uiProcID, iRetVal,
            tbus_error_string(iRetVal));
        return TBUS_INIT_FAILED;
    }

    m_uiProcID = uiProcID;

    srandom(time(NULL) + getpid());

    return SUCCESS;
}

/**
 * @brief 通过BUS发送数据
 * @param:uiDstBusID 目标BUSID，如果是发送到Router，但是不知道发送到哪一台Router，可以填0.0.3.0
 * @param:pvData 需要发送数据的Buff
 * @param:iDataLen 需要发送数据的长度
 * @param:iFlag 暂时不使用
 *
 * @note:
 * 1)就算你知道发送到哪一台Router，由于Bus接口的配置文件中配置了Router接口使用热备的模式工作，该接口也会将uiDstBusID替换为最后一次收到数据的那个Router
 */
int CBusApi::Send(unsigned int uiDstBusID, const void *pvData, int iDataLen, int iFlag/* = 0*/)
{
    int iRetVal = 0;

    unsigned int uiRealDstBusID = uiDstBusID;
    unsigned int uiRealSrcBusID = m_uiProcID;

    // 2014年05月23日 16:35:16/shimmeryang
    // 100K对于好友列表来说不够用了，调整为1M
    if (iDataLen > 1024 * 1000){
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "send pkg too large, handle=%d, src=0x%08X, dst=0x%08X, pkg_len=%d", m_iBusHandle, uiRealSrcBusID, uiRealDstBusID, iDataLen);
        return SEND_PKG_TOO_LARGE;
    }

    for (int i=0; i<m_iClusterBusNum; i++){
        if ((uiDstBusID&(htonl(0xFFFFFF00))) == m_astClusterBusInfo[i].uiBusDst){
            //需要热备的BUS
            //PetLog(0, 0, PETLOG_DEBUG, "CBusApi|%s|%d this pkg is send to cluster_bus[0x%08X], last_active_busid=0x%08X", __FUNCTION__, __LINE__, m_astClusterBusInfo[i].uiBusDst, m_astClusterBusInfo[i].uiLastActiveBusID);

            if (m_astClusterBusInfo[i].uiLastActiveBusID != 0){
                uiRealDstBusID=m_astClusterBusInfo[i].uiLastActiveBusID;
            }else{
                uiRealDstBusID=m_astClusterBusInfo[i].auiBusID[random()%m_astClusterBusInfo[i].iBusIDNum];
            }
        }
    }

    iRetVal = tbus_send(m_iBusHandle, (TBUSADDR *)&uiRealSrcBusID, (TBUSADDR *)&uiRealDstBusID, pvData, iDataLen, iFlag);
    if (iRetVal == (int)TBUS_ERR_CHANNEL_FULL) {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "bus_send failed, handle=%d, src=0x%08X, dst=0x%08X, ret=%d, bus full", m_iBusHandle, uiRealSrcBusID, uiRealDstBusID, iRetVal);
        return BUS_FULL;
    }else if (iRetVal == static_cast<int>(TBUS_ERR_MAKE_ERROR(TBUS_ERROR_NO_CHANNEL_MATCHED))){
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "bus_send failed, handle=%d, src=0x%08X, dst=0x%08X, ret=%d, no channel matched", m_iBusHandle, uiRealSrcBusID, uiRealDstBusID, iRetVal);
        return TBUS_NO_CHANNEL_MATCHED;
    }else if (iRetVal != TBUS_SUCCESS) {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "bus_send failed, handle=%d, src=0x%08X, dst=0x%08X, ret=%d, errmsg=%s", m_iBusHandle, uiRealSrcBusID, uiRealDstBusID, iRetVal, tbus_error_string(iRetVal));
        return TBUS_SEND_FAILED;
    }

    return SUCCESS;
}

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

int CBusApi::Recv(unsigned int *puiSrcBusID, void *pvData, int *piDataLen, int iFlag/* = 0*/)
{
    int iRetVal = 0;
    int iBuffLen = *piDataLen;
    const char* pvPeekData = 0;
    unsigned int uiRealDstBusID = m_uiProcID;
    unsigned int uiRealSrcBusID = 0;
    int iHeartBeatPkgFlag = 0;

    /*
    if (*piDataLen < 8192){
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "recv buff too small, handle=%d, src=0x%08X, buff_len=%d", m_iBusHandle, uiRealSrcBusID, *piDataLen);
        return RECV_BUFF_TOO_SMALL;
    }
    */

    while (true){
        iHeartBeatPkgFlag = 0;
        *piDataLen = iBuffLen;
        iRetVal = tbus_recv(m_iBusHandle, (TBUSADDR *)&uiRealSrcBusID, (TBUSADDR *)&uiRealDstBusID, pvData, (size_t *)piDataLen, iFlag);
        if (iRetVal == (int)TBUS_ERR_CHANNEL_EMPTY){
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "bus_recv failed, handle=%d, src=0x%08X, ret=%d, bus empty", m_iBusHandle, uiRealSrcBusID, iRetVal);
            return BUS_EMPTY;
        }else if (iRetVal == static_cast<int>(TBUS_ERR_MAKE_ERROR(TBUS_ERROR_RECV_BUFFER_LIMITED))){
        	iRetVal = tbus_peek_msg(m_iBusHandle, (TBUSADDR *)&uiRealSrcBusID, (TBUSADDR *)&uiRealDstBusID, &pvPeekData, (size_t *)piDataLen, iFlag);
        	if (iRetVal != TBUS_SUCCESS){
        		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "bus_recv failed, handle=%d, src=0x%08X, buff length too small, peek the message failed, ret=%d, errmsg=%s", m_iBusHandle, uiRealSrcBusID, iRetVal, tbus_error_string(iRetVal));
        	}else{
				iRetVal = tbus_delete_msg(m_iBusHandle, uiRealSrcBusID, uiRealDstBusID);
				if (iRetVal != TBUS_SUCCESS){
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "bus_recv failed, handle=%d, src=0x%08X, buff length too small, drop the message failed, ret=%d, errmsg=%s", m_iBusHandle, uiRealSrcBusID, iRetVal, tbus_error_string(iRetVal));
				}else{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "bus_recv failed, handle=%d, src=0x%08X, buff length too small, drop the message", m_iBusHandle, uiRealSrcBusID);
				}
        	}
            return TBUS_RECV_FAILED;
        }else if (iRetVal != TBUS_SUCCESS){
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "bus_recv failed, handle=%d, src=0x%08X, ret=%d, errmsg=%s", m_iBusHandle, uiRealSrcBusID, iRetVal, tbus_error_string(iRetVal));
            return TBUS_RECV_FAILED;
        }

        //PetLog(0, 0, PETLOG_TRACE, "CBusApi|%s|%d 0x%08X|0x%08X BUS_RECV(%d)[%s]", __FUNCTION__, __LINE__, uiRealSrcBusID, (uiRealSrcBusID)&(htonl(0xFFFFFF00)), *piDataLen, CStrTool::Str2Hex(pvData, *piDataLen));

        for (int i=0; i<m_iClusterBusNum; i++){
            if (((uiRealSrcBusID)&(htonl(0xFFFFFF00))) == m_astClusterBusInfo[i].uiBusDst) {
                //需要热备的BUS
                //PetLog(0, 0, PETLOG_DEBUG, "CBusApi|%s|%d this pkg is recv from cluster_bus, DST=0x%08X, SRC_BUSID=0x%08X", __FUNCTION__, __LINE__, m_astClusterBusInfo[i].uiBusDst, uiRealSrcBusID);

                BusHeader *pstBusHeader = (BusHeader *)pvData;
                int iTimeStamp = 0;
                memcpy(&iTimeStamp, ((char *)pvData)+sizeof(BusHeader), sizeof(iTimeStamp));

                if (pstBusHeader->uiRouterID == ROUTER_ID_FOR_HEART_BEAT){
                    //这是一个心跳包，
                    //TODO 需要给对端回包
                    PetLog(0, 0, PETLOG_TRACE, "CBusApi|%s|%d HEART_BEAT_PKG DST=0x%08X, SRC=0x%08X, REAL_SRC=0x%08X", __FUNCTION__, __LINE__, pstBusHeader->uiDestID, pstBusHeader->uiSrcID, uiRealSrcBusID);
                    m_astClusterBusInfo[i].uiLastActiveBusID = uiRealSrcBusID;
                    iHeartBeatPkgFlag = 1;
                }

                break;
            }
        }

        if (iHeartBeatPkgFlag == 0){
            *puiSrcBusID = uiRealSrcBusID;
            break;
        }
    }

    return SUCCESS;
}

int CBusApi::Peek(unsigned int *puiSrcBusID, unsigned int *puiDstBusID, void **ppvData, int *piDataLen, int iFlag)
{
    int iRetVal = 0;
    *piDataLen = 0;
    unsigned int uiRealDstBusID = m_uiProcID;
    unsigned int uiRealSrcBusID = 0;
    int iHeartBeatPkgFlag = 0;

    while (true){
        iHeartBeatPkgFlag = 0;
        iRetVal = tbus_peek_msg(m_iBusHandle, (TBUSADDR *)&uiRealSrcBusID, (TBUSADDR *)&uiRealDstBusID, (const char**)ppvData, (size_t *)piDataLen, iFlag);
        if (iRetVal == (int)TBUS_ERR_CHANNEL_EMPTY){
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus_peek_msg failed, handle=%d, src=0x%08X, ret=%d, bus empty", m_iBusHandle, uiRealSrcBusID, iRetVal);
            return BUS_EMPTY;
        }else if (iRetVal != TBUS_SUCCESS){
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus_peek_msg failed, handle=%d, src=0x%08X, ret=%d, errmsg=%s", m_iBusHandle, uiRealSrcBusID, iRetVal, tbus_error_string(iRetVal));
            return TBUS_PEEK_FAILED;
        }

        //PetLog(0, 0, PETLOG_TRACE, "CBusApi|%s|%d 0x%08X|0x%08X BUS_RECV(%d)[%s]", __FUNCTION__, __LINE__, uiRealSrcBusID, (uiRealSrcBusID)&(htonl(0xFFFFFF00)), *piDataLen, CStrTool::Str2Hex(pvData, *piDataLen));

        for (int i=0; i<m_iClusterBusNum; i++){
            if (((uiRealSrcBusID)&(htonl(0xFFFFFF00))) == m_astClusterBusInfo[i].uiBusDst) {
                //需要热备的BUS
                //PetLog(0, 0, PETLOG_DEBUG, "CBusApi|%s|%d this pkg is recv from cluster_bus, DST=0x%08X, SRC_BUSID=0x%08X", __FUNCTION__, __LINE__, m_astClusterBusInfo[i].uiBusDst, uiRealSrcBusID);

                BusHeader *pstBusHeader = (BusHeader *)*ppvData;
                int iTimeStamp = 0;
                memcpy(&iTimeStamp, ((char *)*ppvData)+sizeof(BusHeader), sizeof(iTimeStamp));

                if (pstBusHeader->uiRouterID == ROUTER_ID_FOR_HEART_BEAT){
                    //这是一个心跳包，
                    //TODO 需要给对端回包
                    PetLog(0, 0, PETLOG_TRACE, "CBusApi|%s|%d HEART_BEAT_PKG DST=0x%08X, SRC=0x%08X, REAL_SRC=0x%08X", __FUNCTION__, __LINE__, pstBusHeader->uiDestID, pstBusHeader->uiSrcID, uiRealSrcBusID);
                    m_astClusterBusInfo[i].uiLastActiveBusID = uiRealSrcBusID;
                    iHeartBeatPkgFlag = 1;
                }

                break;
            }
        }

        if (iHeartBeatPkgFlag == 0){
            break;
        }

        // Pop out the hello msg
    	iRetVal = tbus_delete_msg(m_iBusHandle, uiRealSrcBusID, uiRealDstBusID);
    	if (iRetVal != TBUS_SUCCESS){
    		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus_delete_msg failed, handle=%d, src=0x%08X, ret=%d, errmsg=%s", m_iBusHandle, uiRealSrcBusID, iRetVal, tbus_error_string(iRetVal));
    		// NOTE: Should never go here
    		PetLog(0, 0, PETLOG_ERR, "CBusApi|%s|tbus_delete_msg failed, handle=%d, src=0x%08X, ret=%d, errmsg=%s", __func__, m_iBusHandle, uiRealSrcBusID, iRetVal, tbus_error_string(iRetVal));
    	}
    }

    *puiSrcBusID = uiRealSrcBusID;
    *puiDstBusID = uiRealDstBusID;

    return SUCCESS;
}

int CBusApi::Pop(unsigned int uiSrcBusID, unsigned uiDstBusID)
{
	int iRetVal = tbus_delete_msg(m_iBusHandle, uiSrcBusID, uiDstBusID);
	if (iRetVal == (int)TBUS_ERR_CHANNEL_EMPTY){
		return 0;
	}else if (iRetVal != TBUS_SUCCESS){
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus_delete_msg failed, handle=%d, src=0x%08X, ret=%d, errmsg=%s", m_iBusHandle, uiSrcBusID, iRetVal, tbus_error_string(iRetVal));
		// NOTE: Should never go here
		PetLog(0, 0, PETLOG_ERR, "CBusApi|%s|tbus_delete_msg failed, handle=%d, src=0x%08X, ret=%d, errmsg=%s", __func__, m_iBusHandle, uiSrcBusID, iRetVal, tbus_error_string(iRetVal));
		return TBUS_POP_FAILED;
    }

    return SUCCESS;
}


int CBusApi::AutoSend(void *pvData, int iDataLen)
{
    if (pvData == NULL){
        return TBUS_SEND_FAILED;
    }

    m_pstBusHeader = (BusHeader *)pvData;
    if ( ( m_pstBusHeader->uiDestID == 0 ) || ( m_pstBusHeader->uiDestID &(htonl(0xFFFFFF00)) == m_uiRouterBusID ) ){
        m_pstBusHeader->uiRouterID = m_uiRouterBusID;
        return Send(m_uiRouterBusID, pvData, iDataLen);
    }

    int iRetVal = Send( m_pstBusHeader->uiDestID, pvData, iDataLen);
    if (iRetVal == TBUS_NO_CHANNEL_MATCHED){
        m_pstBusHeader->uiRouterID = m_uiRouterBusID;
        return Send( m_uiRouterBusID, pvData, iDataLen);
    }

    return iRetVal;
}


int CBusApi::SendToAllRouter(void *pvData, int iDataLen, int iFlag/* = 0*/)
{
    int iRetVal = 0;

    for (int i=0; i<m_iRouterBusNum; i++)
    {
        unsigned int uiRealDstBusID = m_auiAllRouterBusID[i];
        unsigned int uiRealSrcBusID = m_uiProcID;

        /*
        if (iDataLen > 81920)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "send pkg too large, handle=%d, src=0x%08X, dst=0x%08X, pkg_len=%d", m_iBusHandle, uiRealSrcBusID, uiRealDstBusID, iDataLen);
            return SEND_PKG_TOO_LARGE;
        }
        */

        m_pstBusHeader = (BusHeader *)pvData;
        m_pstBusHeader->uiRouterID = uiRealDstBusID;

        iRetVal = tbus_send(m_iBusHandle, (TBUSADDR *)&uiRealSrcBusID, (TBUSADDR *)&uiRealDstBusID, pvData, iDataLen, iFlag);
        if (iRetVal == (int)TBUS_ERR_CHANNEL_FULL)
        {
            PetLog(0, 0, PETLOG_WARN, "BusAPI|SendToAllRouter bus_send failed, handle=%d, src=0x%08X, dst=0x%08X, ret=%d, bus full", m_iBusHandle, uiRealSrcBusID, uiRealDstBusID, iRetVal);
            continue;
        }
        else if (iRetVal != TBUS_SUCCESS)
        {
            PetLog(0, 0, PETLOG_WARN, "BusAPI|SendToAllRouter bus_send failed, handle=%d, src=0x%08X, dst=0x%08X, ret=%d", m_iBusHandle, uiRealSrcBusID, uiRealDstBusID, iRetVal);
            continue;
        }

    }

    return SUCCESS;
}

int CBusApi::SendToAllRouterDirect(void *pvData, int iDataLen, int iFlag/* = 0*/)
{
    int iRetVal = 0;

    for (int i=0; i<m_iRouterBusNum; i++)
    {
        unsigned int uiRealDstBusID = m_auiAllRouterBusID[i];
        unsigned int uiRealSrcBusID = m_uiProcID;

        /*
        if (iDataLen > 81920)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "send pkg too large, handle=%d, src=0x%08X, dst=0x%08X, pkg_len=%d", m_iBusHandle, uiRealSrcBusID, uiRealDstBusID, iDataLen);
            return SEND_PKG_TOO_LARGE;
        }
        */

        m_pstBusHeader = (BusHeader *)pvData;

        iRetVal = tbus_send(m_iBusHandle, (TBUSADDR *)&uiRealSrcBusID, (TBUSADDR *)&uiRealDstBusID, pvData, iDataLen, iFlag);
        if (iRetVal == (int)TBUS_ERR_CHANNEL_FULL)
        {
            PetLog(0, 0, PETLOG_WARN, "BusAPI|%s bus_send failed, handle=%d, src=0x%08X, dst=0x%08X, ret=%d, bus full", __func__, m_iBusHandle, uiRealSrcBusID, uiRealDstBusID, iRetVal);
            continue;
        }
        else if (iRetVal != TBUS_SUCCESS)
        {
            PetLog(0, 0, PETLOG_WARN, "BusAPI|%s bus_send failed, handle=%d, src=0x%08X, dst=0x%08X, ret=%d", __func__, m_iBusHandle, uiRealSrcBusID, uiRealDstBusID, iRetVal);
            continue;
        }

    }

    return SUCCESS;
}


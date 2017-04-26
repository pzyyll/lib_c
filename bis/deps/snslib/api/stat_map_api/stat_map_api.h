#ifndef _STAT_MAP_HASH_LIST_H_
#define _STAT_MAP_HASH_LIST_H_

#include "api/stat_map_api/stat_map_info_def.h"
#include "comm/ini_file/ini_file.h"
#include "comm/hash_list/hash_list_mmap.h"
#include "comm/log/pet_log.h"

namespace snslib{

template<class DATATYPE>
class CStatMapApi
{
public:
    typedef struct tagDATATYPE_WITH_MODIFY_TIME
    {
        DATATYPE stData;
        long lLastModifyTime;
    }DATATYPE_WITH_MODIFY_TIME;

private:
    CHashListMMap<unsigned long long, DATATYPE_WITH_MODIFY_TIME> m_objHashList;
    char m_szErrMsg[256];
    int m_iTimeOut;     //节点超时删除时间，单位秒

public:
    const static int SUCCESS = 0;
    const static int ERROR = -1;

    const static int ERR_BASE = -300;
    const static int NODE_NOT_EXIST = ERR_BASE - 1;
    const static int INIT_HASH_LIST_FAILED = ERR_BASE - 2;
    const static int VERIFY_HASH_LIST_FAILED = ERR_BASE - 3;
    const static int CLEAR_HASH_LIST_FAILED = ERR_BASE - 4;
    const static int NODE_EXIST = ERR_BASE - 5;


public:
    int Init(const char *pszConfFile)
    {
        int iRetVal = 0;

        char szStatMapMapFile[MAX_FILE_PATH_LEN] = {0};
        int iStatMapIndexNum;
        int iStatMapNodeNum;
        int iStatMapVerifyFlag;
        int iStatMapClearFlag;

        CIniFile *pobjIniFile = new CIniFile(pszConfFile);
        pobjIniFile->GetString("STAT_MAP_API", "MapFile", "", szStatMapMapFile, sizeof(szStatMapMapFile));
        pobjIniFile->GetInt("STAT_MAP_API", "TimeOut", 1200, &m_iTimeOut);
        pobjIniFile->GetInt("STAT_MAP_API", "IndexNum", 0, &iStatMapIndexNum);
        pobjIniFile->GetInt("STAT_MAP_API", "NodeNum", 0, &iStatMapNodeNum);
        pobjIniFile->GetInt("STAT_MAP_API", "VerifyFlag", 0, &iStatMapVerifyFlag);
        pobjIniFile->GetInt("STAT_MAP_API", "ClearFlag", 0, &iStatMapClearFlag);
        delete pobjIniFile;

        int iStatMapMemSize = m_objHashList.CalcSize(iStatMapIndexNum, iStatMapNodeNum);
        iRetVal = m_objHashList.Init(szStatMapMapFile, iStatMapMemSize, iStatMapIndexNum, iStatMapNodeNum);
        if (iRetVal != m_objHashList.SUCCESS)
        {
            PetLog(0, 0, PETLOG_ERR, "CStatMapApi|%s init hash list failed, mapfile=%s, mem_size=%d, index_num=%d, node_num=%d, ret=%d",
                    __func__,
                    szStatMapMapFile,
                    iStatMapMemSize,
                    iStatMapIndexNum,
                    iStatMapNodeNum,
                    iRetVal);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "init hash list failed, mapfile=%s, mem_size=%d, index_num=%d, node_num=%d, ret=%d",
                    szStatMapMapFile,
                    iStatMapMemSize,
                    iStatMapIndexNum,
                    iStatMapNodeNum,
                    iRetVal);

            return INIT_HASH_LIST_FAILED;
        }

        PetLog(0, 0, PETLOG_INFO, "CStatMapApi|%s init hash list succ, mapfile=%s, mem_size=%d, index_num=%d, node_num=%d",
                __func__,
                szStatMapMapFile,
                iStatMapMemSize,
                iStatMapIndexNum,
                iStatMapNodeNum);

        if (iStatMapClearFlag == 1)
        {
            iRetVal = m_objHashList.Clear();
            if (iRetVal != m_objHashList.SUCCESS)
            {
                PetLog(0, 0, PETLOG_ERR, "CStatMapApi|%s clear hash list failed, mapfile=%s, mem_size=%d, index_num=%d, node_num=%d, ret=%d",
                        __func__,
                        szStatMapMapFile,
                        iStatMapMemSize,
                        iStatMapIndexNum,
                        iStatMapNodeNum,
                        iRetVal);

                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "clear hash list failed, mapfile=%s, mem_size=%d, index_num=%d, node_num=%d, ret=%d",
                        szStatMapMapFile,
                        iStatMapMemSize,
                        iStatMapIndexNum,
                        iStatMapNodeNum,
                        iRetVal);

                return CLEAR_HASH_LIST_FAILED;
            }

            PetLog(0, 0, PETLOG_INFO, "CStatMapApi|%s clear hash list succ, mapfile=%s, mem_size=%d, index_num=%d, node_num=%d",
                    __func__,
                    szStatMapMapFile,
                    iStatMapMemSize,
                    iStatMapIndexNum,
                    iStatMapNodeNum);
        }

        if (iStatMapVerifyFlag == 1)
        {
            iRetVal = m_objHashList.Verify();
            if (iRetVal != m_objHashList.SUCCESS)
            {
                PetLog(0, 0, PETLOG_ERR, "CStatMapApi|%s verify hash list failed, mapfile=%s, mem_size=%d, index_num=%d, node_num=%d, ret=%d",
                        __func__,
                        szStatMapMapFile,
                        iStatMapMemSize,
                        iStatMapIndexNum,
                        iStatMapNodeNum,
                        iRetVal);

                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "verify hash list failed, mapfile=%s, mem_size=%d, index_num=%d, node_num=%d, ret=%d",
                        szStatMapMapFile,
                        iStatMapMemSize,
                        iStatMapIndexNum,
                        iStatMapNodeNum,
                        iRetVal);

                return VERIFY_HASH_LIST_FAILED;
            }

            PetLog(0, 0, PETLOG_INFO, "CStatMapApi|%s verify hash list succ, mapfile=%s, mem_size=%d, index_num=%d, node_num=%d",
                    __func__,
                    szStatMapMapFile,
                    iStatMapMemSize,
                    iStatMapIndexNum,
                    iStatMapNodeNum);
        }

        return SUCCESS;
    }

    int GetNode(unsigned long long ullPetID, DATATYPE &Data)
    {
        int iRetVal = 0;
        DATATYPE_WITH_MODIFY_TIME stDataWithModifyTime;
        iRetVal = m_objHashList.Get(ullPetID, stDataWithModifyTime);

        if (iRetVal == m_objHashList.E_HASH_LIST_NO_NODE)
        {
            PetLog(0, ullPetID, PETLOG_TRACE, "CStatMapApi|%s get node failed, node not exist, pet_id=%llu, ret=%d",
                    __func__,
                    ullPetID,
                    iRetVal);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "get node failed, node not exist, pet_id=%llu, ret=%d",
                    ullPetID,
                    iRetVal);

            return NODE_NOT_EXIST;

        }
        else if (iRetVal != m_objHashList.SUCCESS)
        {
            PetLog(0, ullPetID, PETLOG_WARN, "CStatMapApi|%s get node failed, pet_id=%llu, ret=%d",
                    __func__,
                    ullPetID,
                    iRetVal);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "get node failed, pet_id=%llu, ret=%d",
                    ullPetID,
                    iRetVal);

            return ERROR;
        }
        else if ((stDataWithModifyTime.lLastModifyTime + m_iTimeOut) < time(NULL))
        {
            char szTmpTime[64] = {0};
            strftime(szTmpTime, sizeof(szTmpTime), "%Y-%m-%d %H:%M:%S", localtime((time_t*)&stDataWithModifyTime.lLastModifyTime));
            PetLog(0, ullPetID, PETLOG_WARN, "CStatMapApi|%s get node failed, node is time out, pet_id=%llu, last_modify_time=%s",
                    __func__,
                    ullPetID,
                    szTmpTime);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "get node failed, node is time out, pet_id=%llu, last_modify_time=%s",
                    ullPetID,
                    szTmpTime);

            iRetVal = m_objHashList.Remove(ullPetID);
            if (iRetVal != m_objHashList.SUCCESS)
            {
                PetLog(0, ullPetID, PETLOG_WARN, "CStatMapApi|%s remove node failed, pet_id=%llu, ret=%d",
                        __func__,
                        ullPetID,
                        iRetVal);
            }

            return NODE_NOT_EXIST;

        }

        memcpy(&Data, &stDataWithModifyTime.stData, sizeof(Data));

        return SUCCESS;
    }

    int RemoveNode(unsigned long long ullPetID)
    {
        int iRetVal = 0;
        iRetVal = m_objHashList.Remove(ullPetID);

        if (iRetVal == m_objHashList.E_HASH_LIST_NO_NODE)
        {
            PetLog(0, ullPetID, PETLOG_WARN, "CStatMapApi|%s remove node failed, node not exist, pet_id=%llu, ret=%d",
                    __func__,
                    ullPetID,
                    iRetVal);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "remove node failed, node not exist, pet_id=%llu, ret=%d",
                    ullPetID,
                    iRetVal);

            return NODE_NOT_EXIST;

        }
        else if (iRetVal != m_objHashList.SUCCESS)
        {
            PetLog(0, ullPetID, PETLOG_WARN, "CStatMapApi|%s remove node failed, pet_id=%llu, ret=%d",
                    __func__,
                    ullPetID,
                    iRetVal);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "remove node failed, pet_id=%llu, ret=%d",
                    ullPetID,
                    iRetVal);

            return ERROR;
        }

        return SUCCESS;
    }

    int UpdateNode(unsigned long long ullPetID, DATATYPE &Data)
    {
        int iRetVal = 0;

        DATATYPE_WITH_MODIFY_TIME stDataWithModifyTime;

        stDataWithModifyTime.lLastModifyTime = time(NULL);
        memcpy(&stDataWithModifyTime.stData, &Data, sizeof(stDataWithModifyTime.stData));

        iRetVal = m_objHashList.Update(ullPetID, stDataWithModifyTime);
        if (iRetVal == m_objHashList.E_HASH_LIST_NO_NODE)
        {
            PetLog(0, ullPetID, PETLOG_WARN, "CStatMapApi|%s update node failed, node not exist, pet_id=%llu, ret=%d",
                    __func__,
                    ullPetID,
                    iRetVal);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "update node failed, node not exist, pet_id=%llu, ret=%d",
                    ullPetID,
                    iRetVal);

            return NODE_NOT_EXIST;
        }
        else if (iRetVal != m_objHashList.SUCCESS)
        {
            PetLog(0, ullPetID, PETLOG_WARN, "CStatMapApi|%s update node failed, pet_id=%llu, ret=%d",
                    __func__,
                    ullPetID,
                    iRetVal);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "update node failed, pet_id=%llu, ret=%d",
                    ullPetID,
                    iRetVal);

            return ERROR;
        }

        return SUCCESS;
    }

    int InsertNode(unsigned long long ullPetID, DATATYPE &Data)
    {
        int iRetVal = 0;

        DATATYPE_WITH_MODIFY_TIME stDataWithModifyTime;

        stDataWithModifyTime.lLastModifyTime = time(NULL);
        memcpy(&stDataWithModifyTime.stData, &Data, sizeof(stDataWithModifyTime.stData));

        iRetVal = m_objHashList.Insert(ullPetID, stDataWithModifyTime);

        if (iRetVal == m_objHashList.E_HASH_LIST_NODE_EXIST)
        {
            PetLog(0, ullPetID, PETLOG_WARN, "CStatMapApi|%s insert node failed, node exist, pet_id=%llu, ret=%d",
                    __func__,
                    ullPetID,
                    iRetVal);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "insert node failed, node exist, pet_id=%llu, ret=%d",
                    ullPetID,
                    iRetVal);

            return NODE_EXIST;
        }
        else if (iRetVal != m_objHashList.SUCCESS)
        {
            PetLog(0, ullPetID, PETLOG_WARN, "CStatMapApi|%s insert node failed, pet_id=%llu, ret=%d",
                    __func__,
                    ullPetID,
                    iRetVal);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "insert node failed, pet_id=%llu, ret=%d",
                    ullPetID,
                    iRetVal);

            return ERROR;
        }

        return SUCCESS;
    }

    int GetRandomNode(unsigned long long &ullPetID, DATATYPE &Data)
    {
        int iRetVal = 0;

        DATATYPE_WITH_MODIFY_TIME stDataWithModifyTime;

        iRetVal = m_objHashList.GetRandomNode(ullPetID, stDataWithModifyTime);

        if (iRetVal == m_objHashList.E_HASH_LIST_NO_NODE)
        {
            PetLog(0, 0, PETLOG_WARN, "CStatMapApi|%s get random_node failed, node not exist, ret=%d",
                    __func__,
                    iRetVal);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "get random node failed, node not exist, ret=%d",
                    iRetVal);

            return NODE_NOT_EXIST;

        }
        else if (iRetVal != m_objHashList.SUCCESS)
        {
            PetLog(0, 0, PETLOG_WARN, "CStatMapApi|%s get random_node failed, ret=%d",
                    __func__,
                    iRetVal);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "get random_node failed, ret=%d",
                    iRetVal);

            return ERROR;
        }
        else if ((stDataWithModifyTime.lLastModifyTime + m_iTimeOut) < time(NULL))
        {
            char szTmpTime[64] = {0};
            strftime(szTmpTime, sizeof(szTmpTime), "%Y-%m-%d %H:%M:%S", localtime((time_t*)&stDataWithModifyTime.lLastModifyTime));
            PetLog(0, ullPetID, PETLOG_WARN, "CStatMapApi|%s get random_node failed, node is time out, pet_id=%llu, last_modify_time=%s",
                    __func__,
                    ullPetID,
                    szTmpTime);

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "get random_node failed, node is time out, pet_id=%llu, last_modify_time=%s",
                    ullPetID,
                    szTmpTime);

            iRetVal = m_objHashList.Remove(ullPetID);
            if (iRetVal != m_objHashList.SUCCESS)
            {
                PetLog(0, ullPetID, PETLOG_WARN, "CStatMapApi|%s remove node failed, pet_id=%llu, ret=%d",
                        __func__,
                        ullPetID,
                        iRetVal);
            }

            return NODE_NOT_EXIST;

        }

        memcpy(&Data, &stDataWithModifyTime.stData, sizeof(Data));

        return SUCCESS;
    }

    const char *GetErrMsg()
    {
        return m_szErrMsg;
    }

};

}
#endif

/**
 * @file    pet_log.h
 * @brief   写日志类，从QSLog改造而来
 * @author  jamieli@tencent.com
 * @date    2008-12-05
 *
 * @change log
 * 2010-06-23 jamieli
 *   1)取消PetLog接口写本地文件的功能，如果QUEUE满，直接丢掉日志
 *   2)增加将日志写入SNS框架的Queue，然后发送至日志中心的功能
 *   3)取消PETLOG_QUEUE_KEY_V1的支持，只支持64MB的QUEUE
 *   4)日志参数由PETID改为UIN
 *
 */

#ifndef _PET_LOG_H_
#define _PET_LOG_H_
#include <stdio.h>

#include "comm/hash_list/hash_list.h"
#include "comm/shm_queue/shm_queue.h"
#include "comm/share_mem/share_mem.h"
#include "api/include/sns_log_def.h"

const int PETLOG_INFO  = 1;
const int PETLOG_ERR   = 2;
const int PETLOG_WARN  = 3;
const int PETLOG_DEBUG = 4;
const int PETLOG_TRACE = 5;

const int PETLOG_MODULE_NAME_LEN = 16;
const int PETLOG_LENGTH_MAX = 4096 * 8;

const int PETLOG_QUEUE_KEY = 0x200007;
const int PETLOG_QUEUE_SIZE = 67108864; //64M

const int PETLOG_CONF_SHM_KEY = 0x200009;
const int PETLOG_CONF_SHM_SIZE = 1048576; //1M

const int PETLOG_SNS_QUEUE_KEY = 0x200010;
const int PETLOG_SNS_QUEUE_SIZE = 67108864; //64M

const int PETLOG_MAX_SPEC_MODULE_NUM = 100;
const int PETLOG_MAX_SPEC_LOGID_NUM = 100;
const int PETLOG_MAX_SPEC_UIN_NUM = 100;
const int PETLOG_MAX_LOG_CENTRE_LOGID_NUM = 100;

const int PETLOG_CONFSHM_INIT_FLAG = 0x1AFFAAFF;    //用于标识内存是否被初始化过

const int PETLOG_DEFAULT_LOG_LEVEL = PETLOG_WARN;
const char PETLOG_DEFAULT_LOG_PATH[] = "/data/home/user00/ml/log";

const int PETLOG_SIZE_CHECK_TIMEVAL = 10;   //每10秒检测一次文件大小，是否超过1G
const int PETLOG_DEFAULT_MAX_FILE_NUM = 16;
const int PETLOG_DEFAULT_MAX_FILE_SIZE = 1024000000;
const int PETLOG_DEFAULT_MIN_FILE_SIZE = 10240000;

const int PETLOG_OUTPUT_TYPE_NORMAL = 1;
const int PETLOG_OUTPUT_TYPE_SPEC_MODULE = 2;
const int PETLOG_OUTPUT_TYPE_SPEC_LOGID = 3;
const int PETLOG_OUTPUT_TYPE_SPEC_UIN = 4;
const int PETLOG_OUTPUT_TYPE_LOG_CENTRE = 5;


// 2014年03月13日 16:30:42/shimmeryang
// 模块写入的日志都有这个头部
typedef struct tagPetLogHeader
{
    unsigned int uiLogTimeSec;
    unsigned int uiLogTimeUSec;
    int iLogLevel;
    int iLogID;
    int iProcID;
    char szModuleName[PETLOG_MODULE_NAME_LEN+1];
    unsigned int uiUin;
    unsigned int uiUNUSED;  //为了保持与旧的日志兼容
    char szLogContent[0];
}PetLogHeader;

typedef struct tagPetLogOutputConf
{
    int iLogConfShmInitFlag;
    int iOutputLogLevel;
    int iOutputLogType;     //1-normal 2-spec_module 3-spec_log_id 4-spec_uin
    char szOutputPathDir[256];
    int iMaxFileSize;
    int iFlushTime;
    int iMaxFileNum;        //循环日志中，一个小时最多日志文件数量
    int iFlushFlag;
    char szReserved[1020];
}PetLogOutputConf;

class CModuleName
{
    //对字符串做了一层封装，主要是用于在HashList中间做Key
public:
    char m_szModuleName[PETLOG_MODULE_NAME_LEN + 1];

    bool operator==(const CModuleName &objModuleName)
    {
        return (strncmp(m_szModuleName, objModuleName.m_szModuleName, sizeof(m_szModuleName)-1) == 0);
    }

    bool operator!=(const CModuleName &objModuleName)
    {
        return (strncmp(m_szModuleName, objModuleName.m_szModuleName, sizeof(m_szModuleName)-1) != 0);
    }

    CModuleName& operator=(const char *pszModuleName)
    {
        memset(this->m_szModuleName, 0x0, sizeof(this->m_szModuleName));
        strncpy(this->m_szModuleName, pszModuleName, sizeof(this->m_szModuleName)-1);
        return *this;
    }

    int Hash() const
    {
        int h = 0;
        const char *key = m_szModuleName;
        while(*key)
        {
            h = (h << 4) + *key++;
            int g = h & 0xF0000000;
            if(g)
                h ^= g >> 24;
            h &= ~g;
        }
        return h;
    }

    int operator%(int iHashIndexNum) const
    {
        return Hash()%iHashIndexNum;
    }
};

class CPetLogAllConf
{
public:
    PetLogOutputConf *m_pstGlobalConf;
    snslib::CHashList<CModuleName, PetLogOutputConf> m_objModuleConf; //用于指定某个特殊模块的日志输出等级
    snslib::CHashList<unsigned int, PetLogOutputConf> m_objLogIDConf; //用于将某个特殊日志ID输出到特定位置
    snslib::CHashList<unsigned int, PetLogOutputConf> m_objUINConf;   //用于将某个特殊UIN相关的日志输出到特定位置
    snslib::CHashList<unsigned int, PetLogOutputConf> m_objLogCentreLogIDConf;   //用于将某个特殊日志ID发送到消息中心

    CPetLogAllConf()
    {
        m_pstGlobalConf = NULL;
    }
};

class CPetLog
{
public:
    const static int SUCCESS = 0;
    const static int ERROR = -1;
    const static int SELFLOG_MAX_SIZE = 1024000000;
    const static int SELFLOG_SIZE_CHECK_TIMEVAL = 10;
    char m_szPetLogPath[64];

public:
    CPetLog();
    ~CPetLog();

    int Open(const char *pszModuleName, int iLogLevel);
    void Log(int iLogID, unsigned int uiUin, int iPriority, const char* pszLogContent);
    int GetLogLevel();
    void WriteMyLog(const char* pszLogContent);

    /*
     * 判断是否要输出该日志
     * true-需要输出
     * false-不需要输出
     */
    bool IsShouldWrite(int iLogID, unsigned int uiUin, int iPriority);

private:
    int m_iInitFlag;
    CModuleName m_objModuleName;
    CPetLogAllConf m_objLogConf;
    snslib::CShmQueue m_objLogQueue;
    snslib::CShmQueue m_objSnsLogQueue;
    snslib::CShareMem m_objLogConfShm;

    bool m_bSnsLogQueueFlag;
    char m_szLogBuff[sizeof(PetLogHeader)+PETLOG_LENGTH_MAX+1];

    //用于自身写日志
    FILE *m_pfSelfLogFile;
    int m_iSelfLogTime;
    time_t m_tSelfLogLastSizeCheckTime;
    int m_iSelfLogLevel;

private:
    const char *GetLogHeadTime(time_t tSec, time_t tUsec);
    const char *GetFileNameTime(time_t tSec);
    int CheckDir(const char * szPath);
    void CheckDirAll(const char * szPath);
    int MakeDir(const char * szPath,bool bIsFilePath = false);
    int CheckFileSize(FILE *fpFile, int iFileSize);
    int RenameFile(const char *pszFileName, int iMaxFileNum);

    void SelfLog(int iLogID, unsigned int uiUin, int iPriority, const char* pszFormat, ...);
    FILE *OpenFile(const char *pszFileName, const char *pszMode);
    bool IsShouldSnsWrite(int iLogID, unsigned int uiUin, int iPriority);

};

extern CPetLog g_objPetLog;

#ifdef __cplusplus
extern "C"
{
#endif

int OpenPetLog(const char* pszModuleName, int iLogLevel = 0, const char* pszLogFilePath = NULL);
void PetLogInternal(int iLogID, unsigned int uiUin, int iPriority, const char* pszFormat, ...)
	__attribute__((format(printf,4,5)));
void ClosePetLog();

int GetLogLevel();

#ifdef __cplusplus
}
#endif


#define PetLog(iLogID, uiUin, iLogLevel, szFormat, args...) \
{ \
    if (g_objPetLog.IsShouldWrite(iLogID, uiUin, iLogLevel)) \
    { \
        PetLogInternal(iLogID, uiUin, iLogLevel, szFormat, ##args); \
    } \
} while(false)

#ifdef PETLOG_CHECK_PARAM
#undef PetLog
#define PetLog(iLogID, uiUin, iLogLevel, szFormat, args...) printf(szFormat, ##args);
#endif

#endif

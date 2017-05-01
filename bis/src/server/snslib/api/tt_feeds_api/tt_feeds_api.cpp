#include <sys/time.h>

#include "tt_feeds_api.h"
#include "tcrdb.h"

#include "comm/util/pet_util.h"
#include "comm/ini_file/ini_file.h"
#include "comm/log/pet_log.h"

const unsigned short TT_FEEDS_MAGIC_NUMBER = 0xAAAA;

using namespace snslib;

CTTFeedsApi::CTTFeedsApi()
{
    for(int i=0; i<MAX_TT_HOST_NUM; i++)
    {
        m_pstRDBMS[i] = (void *)tcrdbnew();
    }

    memset(m_szFeedsTemplatesConf, 0x0, sizeof(m_szFeedsTemplatesConf));
    m_mapFeedsTemplates.clear();
}

CTTFeedsApi::~CTTFeedsApi()
{
    for(int i=0; i<MAX_TT_HOST_NUM; i++)
    {
        tcrdbdel((TCRDB *)(m_pstRDBMS[i]));
    }
}

int CTTFeedsApi::GetHostIdx(unsigned int uiUin)
{
    int m_iHostIdx = uiUin%100/(100/m_iHostNum);
    if (m_iHostIdx >= m_iHostNum)
    {
        m_iHostIdx = m_iHostNum-1;
    }
    return m_iHostIdx;
}

int CTTFeedsApi::Init(const char *pszConfFile)
{
    CIniFile objIniFile(pszConfFile);

    if (objIniFile.IsValid())
    {
        objIniFile.GetInt("TT_FEEDS_API", "HostNum", 0, &m_iHostNum);
        objIniFile.GetInt("TT_FEEDS_API", "CountFlag", 0, &m_iCountFlag);
        objIniFile.GetString("TT_FEEDS_API", "FeedsTemplates", "", m_szFeedsTemplatesConf, sizeof(m_szFeedsTemplatesConf));

        objIniFile.GetInt("KNOCK_OUT", "KeepNum", 0, &m_iKeepNum);
        objIniFile.GetInt("KNOCK_OUT", "KeepDay", 0, &m_iKeepSec);

        m_iKeepSec *= 86400;

        if (m_szFeedsTemplatesConf[0] != '\0')
        {
        	if (ReadFeedsTemplates() != 0)
        	{
        		return -1;
        	}
        }

        if ((m_iHostNum <= 0)||(m_iHostNum > MAX_TT_HOST_NUM))
        {
            PetLog(0, 0, PETLOG_ERR, "%s|conf TT_FEEDS_API/HostNum [%d] is not valid", __FUNCTION__, m_iHostNum);
            return -1;
        }

        char szSecName[64] = {0};
        char szHost[64] = {0};
        int iPort = 0;

        for(int i=1; i<=m_iHostNum; i++)
        {
            snprintf(szSecName, sizeof(szSecName), "TT_FEEDS_HOST_%d", i);
            objIniFile.GetString(szSecName, "Host", "", szHost, sizeof(szHost));
            objIniFile.GetInt(szSecName, "Port", 0, &iPort);

            if (szHost == '\0')
            {
                PetLog(0, 0, PETLOG_ERR, "%s|conf %s/Host [%s] is not valid", __FUNCTION__, szSecName, szHost);
                return -1;
            }

            if (iPort == 0)
            {
                PetLog(0, 0, PETLOG_ERR, "%s|conf %s/Port [%d] is not valid", __FUNCTION__, szSecName, iPort);
                return -1;
            }

            TCRDB *pstRDBMS = (TCRDB *)(m_pstRDBMS[i-1]);
            if (!tcrdbtune(pstRDBMS, 3, RDBTRECON))
            {
                PetLog(0, 0, PETLOG_ERR, "%s|tcrdbtune failed, timeout=%d, errno=%d, errmsg=%s", __FUNCTION__, 3, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                return -1;
            }

            if (!tcrdbopen(pstRDBMS, szHost, iPort))
            {
                PetLog(0, 0, PETLOG_ERR, "%s|tcrdbopen failed, host=%s, port=%d, errno=%d, errmsg=%s", __FUNCTION__, szHost, iPort, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                return -1;
            }

        }
    }
    else
    {
        PetLog(0, 0, PETLOG_ERR, "%s|conf file is not valid, conf_file=%s", __FUNCTION__, pszConfFile);
        return -1;
    }

    return 0;
}

int CTTFeedsApi::CheckUserFeeds(unsigned int uiUin)
{
    if ((m_iKeepNum <= 0) && (m_iKeepSec <= 0))
    {
        return 0;
    }

    TCRDB *pstRDBMS = (TCRDB *)(m_pstRDBMS[GetHostIdx(uiUin)]);
    time_t tTimeNow = time(NULL);

    char szKeyVal[256];
    int iKeyLen = 0;

    char *pszVal;
    int iValLen = 0;

    iKeyLen = snprintf(szKeyVal, sizeof(szKeyVal), "%u", uiUin);

    pszVal = (char *)tcrdbget(pstRDBMS, szKeyVal, iKeyLen, &iValLen);
    if (pszVal == NULL)
    {
        if (tcrdbecode(pstRDBMS) == TTENOREC)
        {
            return 0;
        }
        else
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbget failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
            return -1;
        }
    }

    PetLog(0, uiUin, PETLOG_TRACE, "%s|FEEDS_INFO(%d)[%s]", __FUNCTION__, iValLen, CStrTool::Str2Hex(pszVal, iValLen));

    char *pszFeeds = pszVal;
    char *pszRemainFeeds = pszVal;
    int iRemainFeedsLen = iValLen;
    int iRemainFeedsNum = 0;

    while(true)
    {
        unsigned int uiFeedsTimeSec = 0;
        unsigned int uiFeedsTimeUsec = 0;
        unsigned short ushMagicNumber = 0;
        unsigned short ushFeedsLen = 0;

        //判断数据包是否解析完毕
        if ((pszFeeds-pszVal) == iValLen)
        {
            break;
        }

        //判断头长度是否够
        if((pszVal+iValLen-pszFeeds) < 12)
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|feeds buff check failed, len=%d, len_remain=%lu", __FUNCTION__, iValLen, pszVal+iValLen-pszFeeds);
            free(pszVal);
            return -2;
        }

        memcpy(&uiFeedsTimeSec, pszFeeds, 4);
        pszFeeds += 4;
        memcpy(&uiFeedsTimeUsec, pszFeeds, 4);
        pszFeeds += 4;
        memcpy(&ushMagicNumber, pszFeeds, 2);
        pszFeeds += 2;
        memcpy(&ushFeedsLen, pszFeeds, 2);
        pszFeeds += 2;

        //校验MagicNumber
        if(ushMagicNumber != TT_FEEDS_MAGIC_NUMBER)
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|magic number is not valid, magic=0x%x", __FUNCTION__, ushMagicNumber);
            free(pszVal);
            return -3;
        }

        //判断Feeds信息的内容长度是否够
        if((ushFeedsLen == 0)||((pszVal+iValLen-pszFeeds) < ushFeedsLen))
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|feeds buff check failed, len=%d, len_remain=%lu, feeds_len=%d", __FUNCTION__, iValLen, pszVal+iValLen-pszFeeds, ushFeedsLen);
            free(pszVal);
            return -4;
        }

        if (((int)(tTimeNow - uiFeedsTimeSec)) > m_iKeepSec)
        {
            //需要淘汰
            if (iRemainFeedsNum == 0)
            {
                pszRemainFeeds += (ushFeedsLen+12);
                iRemainFeedsLen -= (ushFeedsLen+12);
            }
        }
        else
        {
            iRemainFeedsNum++;
        }

        PetLog(0, uiUin, PETLOG_TRACE, "%s|remain_len|%d|remain_num|%d|keep_num|%d", __FUNCTION__, iRemainFeedsLen, iRemainFeedsNum, m_iKeepNum);

        pszFeeds += ushFeedsLen;
    }

    if (iRemainFeedsNum > m_iKeepNum)
    {
        pszFeeds = pszRemainFeeds;
        for(int i=0; i<(iRemainFeedsNum-m_iKeepNum); i++)
        {
            unsigned int uiFeedsTimeSec = 0;
            unsigned int uiFeedsTimeUsec = 0;
            unsigned short ushMagicNumber = 0;
            unsigned short ushFeedsLen = 0;

            //判断数据包是否解析完毕
            if ((pszFeeds-pszVal) == iValLen)
            {
                break;
            }

            //判断头长度是否够
            if((pszVal+iValLen-pszFeeds) < 12)
            {
                PetLog(0, uiUin, PETLOG_WARN, "%s|feeds buff check failed, len=%d, len_remain=%lu", __FUNCTION__, iValLen, pszVal+iValLen-pszFeeds);
                free(pszVal);
                return -2;
            }

            memcpy(&uiFeedsTimeSec, pszFeeds, 4);
            pszFeeds += 4;
            memcpy(&uiFeedsTimeUsec, pszFeeds, 4);
            pszFeeds += 4;
            memcpy(&ushMagicNumber, pszFeeds, 2);
            pszFeeds += 2;
            memcpy(&ushFeedsLen, pszFeeds, 2);
            pszFeeds += 2;

            //判断Feeds信息的内容长度是否够
            if((ushFeedsLen == 0)||((pszVal+iValLen-pszFeeds) < ushFeedsLen))
            {
                PetLog(0, uiUin, PETLOG_WARN, "%s|feeds buff check failed, len=%d, len_remain=%lu, feeds_len=%d", __FUNCTION__, iValLen, pszVal+iValLen-pszFeeds, ushFeedsLen);
                free(pszVal);
                return -4;
            }


            pszRemainFeeds += (ushFeedsLen+12);
            iRemainFeedsLen -= (ushFeedsLen+12);
            pszFeeds += ushFeedsLen;
        }

        iRemainFeedsNum = m_iKeepNum;
    }

    if ((iRemainFeedsLen > 0) && (iRemainFeedsLen < iValLen))
    {
        if (!tcrdbput(pstRDBMS, szKeyVal, iKeyLen, pszRemainFeeds, iRemainFeedsLen))
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|feeds update failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
            free(pszVal);
            return -5;
        }

        if (m_iCountFlag == 1)
        {
            char szNCKeyVal[64];
            int iNCKeyLen = 0;

            char *pszNCVal = NULL;
            int iNCValLen = 0;
            int iFeedsNC = 0;

            iNCKeyLen = snprintf(szNCKeyVal, sizeof(szNCKeyVal), "%u_nc", uiUin);

            pszNCVal = (char *)tcrdbget(pstRDBMS, szNCKeyVal, iNCKeyLen, &iNCValLen);
            if (pszNCVal == NULL)
            {
                PetLog(0, uiUin, PETLOG_WARN, "%s|feeds_nc get failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                iFeedsNC = 0;
            }
            else
            {
                memcpy(&iFeedsNC, pszNCVal, 4);
            }

            PetLog(0, uiUin, PETLOG_DEBUG, "%s|feeds_nc|%d|remain_nc|%d", __FUNCTION__, iFeedsNC, iRemainFeedsNum);

            if (iFeedsNC > iRemainFeedsNum)
            {
                if(!tcrdbaddint(pstRDBMS, szNCKeyVal, iNCKeyLen, (iRemainFeedsNum - iFeedsNC)))
                {
                    PetLog(0, uiUin, PETLOG_WARN, "%s|feeds_nc add failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                }
            }

            delete pszNCVal;
        }

    }
    else if (iRemainFeedsLen == 0)
    {
        if (!tcrdbout(pstRDBMS, szKeyVal, iKeyLen))
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|feeds remove failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
            free(pszVal);
            return -6;
        }

        if (m_iCountFlag == 1)
        {
            char szNCKeyVal[64];
            int iNCKeyLen = 0;

            iNCKeyLen = snprintf(szNCKeyVal, sizeof(szNCKeyVal), "%u_nc", uiUin);

            if (!tcrdbout(pstRDBMS, szNCKeyVal, iNCKeyLen))
            {
                PetLog(0, uiUin, PETLOG_WARN, "%s|feeds_nc remove failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                free(pszVal);
                return -7;
            }
        }
    }
    else if (iRemainFeedsLen == iValLen)
    {
        PetLog(0, uiUin, PETLOG_DEBUG, "%s|feed not need change", __FUNCTION__);
    }
    else
    {
        PetLog(0, uiUin, PETLOG_WARN, "%s|feeds remain len is not valid, remain_len=%d, len=%d", __FUNCTION__, iRemainFeedsLen, iValLen);
        free(pszVal);
        return -8;
    }

    free(pszVal);

    return 0;

}


int CTTFeedsApi::AddFeeds(unsigned int uiUin, const std::string sFeedsInfo)
{
    //FEEDS INFO
    //[8Byte:TimeStamp/FeedsID][2Byte:MagicNumber][2Byte:FeedsInfoLen][FeedsInfoLen Byte:FeedsInfo]

    PetLog(0, uiUin, PETLOG_TRACE, "%s|FEEDS_INFO(%lu)[%s]", __FUNCTION__, sFeedsInfo.size(), CStrTool::Str2Hex(sFeedsInfo.c_str(), sFeedsInfo.size()));

    CheckUserFeeds(uiUin);

    if (sFeedsInfo.size() > (unsigned int)MAX_TT_FEEDS_LEN)
    {
        PetLog(0, uiUin, PETLOG_WARN, "%s|feed_len is not valid, len=%lu", __FUNCTION__, sFeedsInfo.size());
        return -1;
    }

    TCRDB *pstRDBMS = (TCRDB *)(m_pstRDBMS[GetHostIdx(uiUin)]);

    char szFeedsInfo[MAX_TT_BUFF_LEN];

    struct timeval stTimeNow;
    gettimeofday(&stTimeNow, NULL);

    unsigned short ushFeedsLen = sFeedsInfo.size();

    memcpy(szFeedsInfo, &stTimeNow.tv_sec, 4);
    memcpy(szFeedsInfo+4, &stTimeNow.tv_usec, 4);
    memcpy(szFeedsInfo+8, &TT_FEEDS_MAGIC_NUMBER, 2);
    memcpy(szFeedsInfo+10, &ushFeedsLen, 2);
    memcpy(szFeedsInfo+12, sFeedsInfo.c_str(), sFeedsInfo.size());

    m_iKeyLen = snprintf(m_szKeyVal, sizeof(m_szKeyVal), "%u", uiUin);

    if (!tcrdbputcat(pstRDBMS, m_szKeyVal, m_iKeyLen, szFeedsInfo, sFeedsInfo.size()+12))
    {
        if ((tcrdbecode(pstRDBMS) == TTESEND)||(tcrdbecode(pstRDBMS) == TTERECV))
        {
            if (!tcrdbputcat(pstRDBMS, m_szKeyVal, m_iKeyLen, szFeedsInfo, sFeedsInfo.size()+12))
            {
                PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbputcat retry failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                return -1;
            }
        }
        else
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbputcat failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
            return -1;
        }
    }

    if (m_iCountFlag == 1)
    {
        int iCurrNewFeedsCount = 0;
        m_iKeyLen = snprintf(m_szKeyVal, sizeof(m_szKeyVal), "%u_nc", uiUin);
        if ((iCurrNewFeedsCount = tcrdbaddint(pstRDBMS, m_szKeyVal, m_iKeyLen, 1)) == INT_MIN)
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbaddint failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
        }
        else
        {
            PetLog(0, uiUin, PETLOG_DEBUG, "%s|%d|add new_feeds_count succ", __FUNCTION__, iCurrNewFeedsCount);
        }
    }

    return 0;
}

int CTTFeedsApi::GetFeeds(unsigned int uiUin, unsigned long long ullFeedsID, std::string &sFeedsInfo)
{
    TCRDB *pstRDBMS = (TCRDB *)(m_pstRDBMS[GetHostIdx(uiUin)]);

    char *pszVal = NULL;
    int iValLen = 0;

    m_iKeyLen = snprintf(m_szKeyVal, sizeof(m_szKeyVal), "%u", uiUin);

    pszVal = (char *)tcrdbget(pstRDBMS, m_szKeyVal, m_iKeyLen, &iValLen);
    if (pszVal == NULL)
    {
        if (tcrdbecode(pstRDBMS) == TTENOREC)
        {
            return TT_FEEDS_ERR_NORECORED;
        }
        else if ((tcrdbecode(pstRDBMS) == TTESEND)||(tcrdbecode(pstRDBMS) == TTERECV))
        {
            pszVal = (char *)tcrdbget(pstRDBMS, m_szKeyVal, m_iKeyLen, &iValLen);
            if (pszVal == NULL)
            {
                if (tcrdbecode(pstRDBMS) == TTENOREC)
                {
                    return TT_FEEDS_ERR_NORECORED;
                }
                else
                {
                    PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbget retry failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                    return -1;
                }
            }
        }

        else
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbget failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
            return -1;
        }
    }

    PetLog(0, uiUin, PETLOG_TRACE, "%s|FEEDS_INFO(%d)[%s]", __FUNCTION__, iValLen, CStrTool::Str2Hex(pszVal, iValLen));

    char *pszFeeds = pszVal;

    sFeedsInfo.clear();

    while(true)
    {
        unsigned long long ullTmpFeedsID = 0;
        unsigned int uiFeedsTimeSec = 0;
        unsigned int uiFeedsTimeUsec = 0;
        unsigned short ushMagicNumber = 0;
        unsigned short ushFeedsLen = 0;

        //判断数据包是否解析完毕
        if ((pszFeeds-pszVal) == iValLen)
        {
            break;
        }

        //判断头长度是否够
        if((pszVal+iValLen-pszFeeds) < 12)
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|feeds buff check failed, len=%d, len_remain=%lu", __FUNCTION__, iValLen, pszVal+iValLen-pszFeeds);
            free(pszVal);
            return -2;
        }

        memcpy(&ullTmpFeedsID, pszFeeds, 8);
        memcpy(&uiFeedsTimeSec, pszFeeds, 4);
        pszFeeds += 4;
        memcpy(&uiFeedsTimeUsec, pszFeeds, 4);
        pszFeeds += 4;
        memcpy(&ushMagicNumber, pszFeeds, 2);
        pszFeeds += 2;
        memcpy(&ushFeedsLen, pszFeeds, 2);
        pszFeeds += 2;

        //校验MagicNumber
        if(ushMagicNumber != TT_FEEDS_MAGIC_NUMBER)
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|magic number is not valid, magic=0x%x", __FUNCTION__, ushMagicNumber);
            free(pszVal);
            return -3;
        }

        //判断Feeds信息的内容长度是否够
        if((ushFeedsLen == 0)||((pszVal+iValLen-pszFeeds) < ushFeedsLen))
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|feeds buff check failed, len=%d, len_remain=%lu, feeds_len=%d", __FUNCTION__, iValLen, pszVal+iValLen-pszFeeds, ushFeedsLen);
            free(pszVal);
            return -4;
        }

        if (ullTmpFeedsID == ullFeedsID)
        {
            sFeedsInfo.assign(pszFeeds, ushFeedsLen);
            break;
        }
        else
        {
            pszFeeds += ushFeedsLen;
        }
    }

    free(pszVal);

    if (sFeedsInfo.size() == 0)
    {
        return TT_FEEDS_ERR_NORECORED;
    }

    return 0;
}

int CTTFeedsApi::GetAllFeeds(unsigned int uiUin, std::vector<unsigned long long> &vullFeedsID, std::vector<std::string> &vsFeedsInfo, int iCleanNewCountFlag/* = 0*/)
{
    //FEEDS INFO
    //[8Byte:TimeStamp/FeedsID][2Byte:MagicNumber][2Byte:FeedsInfoLen][FeedsInfoLen Byte:FeedsInfo]

    TCRDB *pstRDBMS = (TCRDB *)(m_pstRDBMS[GetHostIdx(uiUin)]);

    char *pszVal = NULL;
    int iValLen = 0;

    m_iKeyLen = snprintf(m_szKeyVal, sizeof(m_szKeyVal), "%u", uiUin);

    pszVal = (char *)tcrdbget(pstRDBMS, m_szKeyVal, m_iKeyLen, &iValLen);
    if (pszVal == NULL)
    {
        if (tcrdbecode(pstRDBMS) == TTENOREC)
        {
            return TT_FEEDS_ERR_NORECORED;
        }
        else if ((tcrdbecode(pstRDBMS) == TTESEND)||(tcrdbecode(pstRDBMS) == TTERECV))
        {
            pszVal = (char *)tcrdbget(pstRDBMS, m_szKeyVal, m_iKeyLen, &iValLen);
            if (pszVal == NULL)
            {
                if (tcrdbecode(pstRDBMS) == TTENOREC)
                {
                    return TT_FEEDS_ERR_NORECORED;
                }
                else
                {
                    PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbget retry failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                    return -1;
                }
            }

        }
        else
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbget failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
            return -1;
        }
    }

    PetLog(0, uiUin, PETLOG_TRACE, "%s|FEEDS_INFO(%d)[%s]", __FUNCTION__, iValLen, CStrTool::Str2Hex(pszVal, iValLen));

    vullFeedsID.clear();
    vsFeedsInfo.clear();

    char *pszFeeds = pszVal;

    while(true)
    {
        unsigned long long ullFeedsID = 0;
        unsigned int uiFeedsTimeSec = 0;
        unsigned int uiFeedsTimeUsec = 0;
        unsigned short ushMagicNumber = 0;
        unsigned short ushFeedsLen = 0;
        std::string sFeedsInfo;

        //判断数据包是否解析完毕
        if ((pszFeeds-pszVal) == iValLen)
        {
            break;
        }

        //判断头长度是否够
        if((pszVal+iValLen-pszFeeds) < 12)
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|feeds buff check failed, len=%d, len_remain=%lu", __FUNCTION__, iValLen, pszVal+iValLen-pszFeeds);
            free(pszVal);
            return -2;
        }

        memcpy(&ullFeedsID, pszFeeds, 8);
        memcpy(&uiFeedsTimeSec, pszFeeds, 4);
        pszFeeds += 4;
        memcpy(&uiFeedsTimeUsec, pszFeeds, 4);
        pszFeeds += 4;
        memcpy(&ushMagicNumber, pszFeeds, 2);
        pszFeeds += 2;
        memcpy(&ushFeedsLen, pszFeeds, 2);
        pszFeeds += 2;

        //校验MagicNumber
        if(ushMagicNumber != TT_FEEDS_MAGIC_NUMBER)
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|magic number is not valid, magic=0x%x", __FUNCTION__, ushMagicNumber);
            free(pszVal);
            return -3;
        }

        //判断Feeds信息的内容长度是否够
        if((ushFeedsLen == 0)||((pszVal+iValLen-pszFeeds) < ushFeedsLen))
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|feeds buff check failed, len=%d, len_remain=%lu, feeds_len=%d", __FUNCTION__, iValLen, pszVal+iValLen-pszFeeds, ushFeedsLen);
            free(pszVal);
            return -4;
        }

        sFeedsInfo.assign(pszFeeds, ushFeedsLen);
        pszFeeds += ushFeedsLen;

        vullFeedsID.push_back(ullFeedsID);
        vsFeedsInfo.push_back(sFeedsInfo);
    }

    free(pszVal);

    if ((m_iCountFlag == 1) && (iCleanNewCountFlag == 1))
    {
        m_iKeyLen = snprintf(m_szKeyVal, sizeof(m_szKeyVal), "%u_nc", uiUin);
        if (!tcrdbout(pstRDBMS, m_szKeyVal, m_iKeyLen))
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbout nc failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
        }
        else
        {
            PetLog(0, uiUin, PETLOG_DEBUG, "%s|del new_feeds_count succ", __FUNCTION__);
        }
    }

    return 0;
}

int CTTFeedsApi::DelFeeds(unsigned int uiUin, unsigned long long ullFeedsID)
{
    //FEEDS INFO
    //[8Byte:TimeStamp/FeedsID][2Byte:MagicNumber][2Byte:FeedsInfoLen][FeedsInfoLen Byte:FeedsInfo]

    TCRDB *pstRDBMS = (TCRDB *)(m_pstRDBMS[GetHostIdx(uiUin)]);

    char *pszVal = NULL;
    int iValLen = 0;

    m_iKeyLen = snprintf(m_szKeyVal, sizeof(m_szKeyVal), "%u", uiUin);

    pszVal = (char *)tcrdbget(pstRDBMS, m_szKeyVal, m_iKeyLen, &iValLen);
    if (pszVal == NULL)
    {
        if (tcrdbecode(pstRDBMS) == TTENOREC)
        {
            return TT_FEEDS_ERR_NORECORED;
        }
        else if ((tcrdbecode(pstRDBMS) == TTESEND)||(tcrdbecode(pstRDBMS) == TTERECV))
        {
            pszVal = (char *)tcrdbget(pstRDBMS, m_szKeyVal, m_iKeyLen, &iValLen);
            if (pszVal == NULL)
            {
                if (tcrdbecode(pstRDBMS) == TTENOREC)
                {
                    return TT_FEEDS_ERR_NORECORED;
                }
                else
                {
                    PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbget retry failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                    return -1;
                }
            }
        }
        else
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbget failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
            return -1;
        }
    }

    PetLog(0, uiUin, PETLOG_TRACE, "%s|FEEDS_INFO(%d)[%s]", __FUNCTION__, iValLen, CStrTool::Str2Hex(pszVal, iValLen));

    char *pszFeeds = pszVal;

    int iDelFlag = 0;
    while(true)
    {
        unsigned long long ullTmpFeedsID = 0;
        unsigned int uiFeedsTimeSec = 0;
        unsigned int uiFeedsTimeUsec = 0;
        unsigned short ushMagicNumber = 0;
        unsigned short ushFeedsLen = 0;
        std::string sFeedsInfo;

        //判断数据包是否解析完毕
        if ((pszFeeds-pszVal) == iValLen)
        {
            break;
        }

        //判断头长度是否够
        if((pszVal+iValLen-pszFeeds) < 12)
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|feeds buff check failed, len=%d, len_remain=%lu", __FUNCTION__, iValLen, pszVal+iValLen-pszFeeds);
            free(pszVal);
            return -2;
        }

        memcpy(&ullTmpFeedsID, pszFeeds, 8);
        memcpy(&uiFeedsTimeSec, pszFeeds, 4);
        pszFeeds += 4;
        memcpy(&uiFeedsTimeUsec, pszFeeds, 4);
        pszFeeds += 4;
        memcpy(&ushMagicNumber, pszFeeds, 2);
        pszFeeds += 2;
        memcpy(&ushFeedsLen, pszFeeds, 2);
        pszFeeds += 2;

        //校验MagicNumber
        if(ushMagicNumber != TT_FEEDS_MAGIC_NUMBER)
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|magic number is not valid, magic=0x%x", __FUNCTION__, ushMagicNumber);
            free(pszVal);
            return -3;
        }

        //判断Feeds信息的内容长度是否够
        if((ushFeedsLen == 0)||((pszVal+iValLen-pszFeeds) < ushFeedsLen))
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|feeds buff check failed, len=%d, len_remain=%lu, feeds_len=%d", __FUNCTION__, iValLen, pszVal+iValLen-pszFeeds, ushFeedsLen);
            free(pszVal);
            return -4;
        }

        if(ullTmpFeedsID == ullFeedsID)
        {
            iDelFlag = 1;
            memmove(pszFeeds-12, pszFeeds+ushFeedsLen, iValLen-(pszFeeds-pszVal+ushFeedsLen));
            if ((12+ushFeedsLen) == iValLen)
            {
                //直接删除该节点
                if (!tcrdbout(pstRDBMS, m_szKeyVal, m_iKeyLen))
                {
                    PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbout failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                    free(pszVal);
                    return -5;
                }
            }
            else
            {
                //更新该节点
                if (!tcrdbput(pstRDBMS, m_szKeyVal, m_iKeyLen, pszVal, (iValLen - 12 - ushFeedsLen)))
                {
                    PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbput failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                    free(pszVal);
                    return -6;
                }
            }

            break;
        }
        pszFeeds += ushFeedsLen;
    }

    free(pszVal);

    if (iDelFlag == 0)
    {
        return TT_FEEDS_ERR_NORECORED;
    }

    return 0;
}

int CTTFeedsApi::DelAllFeeds(unsigned int uiUin)
{
    TCRDB *pstRDBMS = (TCRDB *)(m_pstRDBMS[GetHostIdx(uiUin)]);

    m_iKeyLen = snprintf(m_szKeyVal, sizeof(m_szKeyVal), "%u", uiUin);

    if (!tcrdbout(pstRDBMS, m_szKeyVal, m_iKeyLen))
    {
        if (tcrdbecode(pstRDBMS) == TTENOREC)
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbout failed, no recored", __FUNCTION__);
        }
        else if ((tcrdbecode(pstRDBMS) == TTESEND)||(tcrdbecode(pstRDBMS) == TTERECV))
        {
            if (!tcrdbout(pstRDBMS, m_szKeyVal, m_iKeyLen))
            {
                if (tcrdbecode(pstRDBMS) == TTENOREC)
                {
                    PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbout failed, no recored", __FUNCTION__);
                }
                else
                {
                    PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbout retry failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                    return -1;
                }
            }
        }
        else
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbout failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
            return -1;
        }
    }

    if (m_iCountFlag == 1)
    {
        m_iKeyLen = snprintf(m_szKeyVal, sizeof(m_szKeyVal), "%u_nc", uiUin);
        if (!tcrdbout(pstRDBMS, m_szKeyVal, m_iKeyLen))
        {
            PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbout nc failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
        }
        else
        {
            PetLog(0, uiUin, PETLOG_DEBUG, "%s|del new_feeds_count succ", __FUNCTION__);
        }
    }

    return 0;
}

int CTTFeedsApi::GetNewFeedsCount(unsigned int uiUin, unsigned int *puiNewFeedsCount)
{
    if (m_iCountFlag == 1)
    {
        char *pszNewFeedsCount = NULL;
        int iNewFeedsCountLen = 0;
        TCRDB *pstRDBMS = (TCRDB *)(m_pstRDBMS[GetHostIdx(uiUin)]);
        m_iKeyLen = snprintf(m_szKeyVal, sizeof(m_szKeyVal), "%u_nc", uiUin);

        pszNewFeedsCount = (char *)tcrdbget(pstRDBMS, m_szKeyVal, m_iKeyLen, &iNewFeedsCountLen);
        if (pszNewFeedsCount == NULL)
        {
            if (tcrdbecode(pstRDBMS) == TTENOREC)
            {
                *puiNewFeedsCount = 0;
            }
            else if ((tcrdbecode(pstRDBMS) == TTESEND)||(tcrdbecode(pstRDBMS) == TTERECV))
            {
                pszNewFeedsCount = (char *)tcrdbget(pstRDBMS, m_szKeyVal, m_iKeyLen, &iNewFeedsCountLen);
                if (pszNewFeedsCount == NULL)
                {
                    if (tcrdbecode(pstRDBMS) == TTENOREC)
                    {
                        *puiNewFeedsCount = 0;
                    }
                    else
                    {
                        PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbget nc retry failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                        return -1;
                    }
                }
            }
            else
            {
                PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbget nc failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                return -1;
            }
        }
        else
        {
            memcpy(puiNewFeedsCount, pszNewFeedsCount, sizeof(unsigned int));
            free(pszNewFeedsCount);
        }

        return 0;
    }

    else
    {
        return TT_FEEDS_ERR_NOTSUPPORT;
    }
}

int CTTFeedsApi::CleanNewFeedsCount(unsigned int uiUin)
{
    if (m_iCountFlag == 1)
    {
        TCRDB *pstRDBMS = (TCRDB *)(m_pstRDBMS[GetHostIdx(uiUin)]);
        m_iKeyLen = snprintf(m_szKeyVal, sizeof(m_szKeyVal), "%u_nc", uiUin);

        if (!tcrdbout(pstRDBMS, m_szKeyVal, m_iKeyLen))
        {
            if ((tcrdbecode(pstRDBMS) == TTESEND)||(tcrdbecode(pstRDBMS) == TTERECV))
            {
                if (!tcrdbout(pstRDBMS, m_szKeyVal, m_iKeyLen))
                {
                    PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbout nc retry failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
                }
            }
            else
            {
                PetLog(0, uiUin, PETLOG_WARN, "%s|tcrdbout nc failed, errno=%d, errmsg=%s", __FUNCTION__, tcrdbecode(pstRDBMS), tcrdberrmsg(tcrdbecode(pstRDBMS)));
            }
        }
        return 0;
    }

    else
    {
        return TT_FEEDS_ERR_NOTSUPPORT;
    }
}

// add by channerli

int CTTFeedsApi::ReadFeedsTemplates()
{
	FILE *pFile = fopen(m_szFeedsTemplatesConf, "r");
	if(pFile == NULL)
	{
		PetLog(0, 0, PETLOG_WARN, "Invalid FeedsTemplates Config, file=%s", m_szFeedsTemplatesConf);
		return -1;
	}

	char szLine[MAX_TT_FEEDS_LEN];
	char *pcStart 	= NULL;
	char *pcEnd	  	= NULL;
    char *pchSave 	= NULL;
    char *pszField 	= NULL;
	const char cszSeparator[] = "\t";

	while(fgets(szLine, sizeof(szLine), pFile) != NULL)
	{
		std::string strID;
		pcStart = szLine;
        while (isblank(*pcStart))
        {
        	++pcStart;
        }
        // 空行或者注释
        if (*pcStart == '\0' || *pcStart == '#')
        {
        	continue;
        }

        for (pcEnd = pcStart; *pcEnd != '\0'; ++pcEnd)
        {
            if (*pcEnd == '\r' || *pcEnd == '\n')
            {
                *pcEnd = '\0';
                break;
            }
        }
        // AppID
        if (NULL == (pszField = strtok_r(pcStart, cszSeparator, &pchSave)))
        {
        	PetLog(0, 0, PETLOG_WARN, "FeedsTemplates AppID err");
        	return -1;
        }
        strID = pszField;

        // MsgID
        if (NULL == (pszField = strtok_r(NULL, cszSeparator, &pchSave)))
        {
        	PetLog(0, 0, PETLOG_WARN, "FeedsTemplates MsgID err");
        	return -1;
        }
        strID += pszField;

        // feeds templates
        if (NULL == (pszField = strtok_r(NULL, cszSeparator, &pchSave)))
        {
        	PetLog(0, 0, PETLOG_WARN, "FeedsTemplates feeds err");
        	return -1;
        }
        m_mapFeedsTemplates[strID] = pszField;
	}

	if (m_mapFeedsTemplates.empty())
	{
    	PetLog(0, 0, PETLOG_WARN, "FeedsTemplates is empty");
    	return -1;
	}

	return 0;
}

int CTTFeedsApi::AddFeedsKeyVal(unsigned int uiUin, int nAppID, int nMsgID, keyval &MapKeyVal)
{
	// KEYVAL INFO
	// [1Byte:KEYVAL size][1Byte:KEYLen][KEYLen Byte:KEYString][2Byte:VALLen][VALLen Byte:ValString]...

	int nOffSet					= 0;
	unsigned char uchSize		= 0;
	unsigned short ushValueSize	= 0;
	char szConvesion[MAX_TT_KEY_LEN];
	char szStringBuf[MAX_TT_BUFF_LEN];
	std::string strID;

	snprintf(szConvesion, sizeof(szConvesion), "%d", nAppID);
	MapKeyVal["$APPID$"] = szConvesion;
	strID = szConvesion;

	snprintf(szConvesion, sizeof(szConvesion), "%d", nMsgID);
	MapKeyVal["$MSGID$"] = szConvesion;
	strID += szConvesion;

	if (!m_mapFeedsTemplates.empty() && m_mapFeedsTemplates.find(strID) == m_mapFeedsTemplates.end())
	{
		PetLog(0, uiUin, PETLOG_WARN, "%s, %d|APPID(%d) and MSGID(%d) maybe Invalid， FeedsTemplates not find",
				__FUNCTION__, __LINE__, nAppID, nMsgID);

		return -1;
	}

	uchSize = MapKeyVal.size();
	nOffSet += CBuffTool::WriteByte(szStringBuf + nOffSet, uchSize);

	for (keyval::iterator it = MapKeyVal.begin(); it != MapKeyVal.end(); ++it)
	{
		// key
		uchSize = it->first.size();
		if (sizeof(uchSize) + uchSize + nOffSet >= sizeof(szStringBuf))
		{
			PetLog(0, uiUin, PETLOG_WARN, "%s, %d|string buff is not enough!", __FUNCTION__, __LINE__);
			return -1;
		}
		nOffSet += CBuffTool::WriteByte(szStringBuf + nOffSet, uchSize);
		nOffSet += CBuffTool::WriteString(szStringBuf + nOffSet, it->first.c_str(), uchSize);

		// value
		ushValueSize = it->second.size();
		if (sizeof(ushValueSize) + ushValueSize + nOffSet > sizeof(szStringBuf))
		{
			PetLog(0, uiUin, PETLOG_WARN, "%s, %d|string buff is not enough!", __FUNCTION__, __LINE__);
			return -1;
		}
		nOffSet += CBuffTool::WriteShort(szStringBuf + nOffSet, ushValueSize);
		nOffSet += CBuffTool::WriteString(szStringBuf + nOffSet, it->second.c_str(), ushValueSize);
	}

	std::string sFeedsInfo;
	sFeedsInfo.assign(szStringBuf, nOffSet);

	return AddFeeds(uiUin, sFeedsInfo);
}

int CTTFeedsApi::GetAllFeedsKeyVal(
		unsigned int uiUin,
		std::vector<unsigned long long> &vullFeedsID,
		std::vector<keyval> &vecFeedsKeyVal,
		std::vector<std::string> &vsFeeds,
		int iCleanNewCountFlag
)
{
	// KEYVAL INFO
	// [1Byte:KEYVAL size][1Byte:KEYLen][KEYLen Byte:KEYString][2Byte:VALLen][VALLen Byte:ValString]...

	int nRet = 0;
	std::vector<std::string> vsFeedsInfo;

	nRet = GetAllFeeds(uiUin, vullFeedsID, vsFeedsInfo, iCleanNewCountFlag);
	if (nRet != 0)
	{
		PetLog(0, uiUin, PETLOG_WARN, "%s|GetAllFeeds err, ret = %d", __FUNCTION__, nRet);
		return nRet;
	}

	vecFeedsKeyVal.clear();
	vsFeeds.clear();

	for (std::vector<std::string>::iterator it = vsFeedsInfo.begin(); it != vsFeedsInfo.end(); ++it)
	{
		int nOffSet					= 0;
		unsigned char uchSize		= 0;
		unsigned short ushValueSize	= 0;
		char szStringBuf[MAX_TT_BUFF_LEN];
		char szKey[MAX_TT_KEY_LEN];
		char szVal[MAX_TT_FEEDS_LEN];
		keyval MapKeyVal;

		if ((*it).size() > sizeof(szStringBuf))
		{
			PetLog(0, uiUin, PETLOG_WARN, "%s|get Feeds is too long", __FUNCTION__);
			return -1;
		}
		memcpy(szStringBuf, (*it).c_str(), (*it).size());

		nOffSet += CBuffTool::ReadByte(szStringBuf + nOffSet, uchSize);
		const int cnSize = uchSize;

		for (int i = 0; i < cnSize; ++i)
		{
			// key
			nOffSet += CBuffTool::ReadByte(szStringBuf + nOffSet, uchSize);
			if (uchSize + (unsigned int)nOffSet >= sizeof(szStringBuf))
			{
				PetLog(0, uiUin, PETLOG_WARN, "%s, %d|string buff is not enough!", __FUNCTION__, __LINE__);
				return -1;
			}
			nOffSet += CBuffTool::ReadString(szStringBuf + nOffSet, szKey, uchSize);
			szKey[uchSize] = '\0';

			// value
			nOffSet += CBuffTool::ReadShort(szStringBuf + nOffSet, ushValueSize);
			if (ushValueSize + (unsigned int)nOffSet > sizeof(szStringBuf))
			{
				PetLog(0, uiUin, PETLOG_WARN, "%s, %d|string buff is not enough!", __FUNCTION__, __LINE__);
				return -1;
			}
			nOffSet += CBuffTool::ReadString(szStringBuf + nOffSet, szVal, ushValueSize);
			szVal[ushValueSize] = '\0';

			MapKeyVal[szKey] = szVal;
		}

		vecFeedsKeyVal.push_back(MapKeyVal);
	}

	if (!m_mapFeedsTemplates.empty())
	{
		return GenFeedsInfo(vecFeedsKeyVal, vsFeeds);
	}

	return 0;
}

int CTTFeedsApi::GenFeedsInfo(std::vector<keyval> &vecFeedsKeyVal, std::vector<std::string> &vsFeeds)
{
	if (vecFeedsKeyVal.empty())
	{
		return 0;
	}

	for (std::vector<keyval>::iterator itVec = vecFeedsKeyVal.begin(); itVec != vecFeedsKeyVal.end(); ++itVec)
	{
		keyval &MapKeyVal = (*itVec);
		std::string strID;
		std::string strFeedsTemplates;
		strID = MapKeyVal["$APPID$"];
		strID += MapKeyVal["$MSGID$"];

		strFeedsTemplates = m_mapFeedsTemplates[strID];
		if (strFeedsTemplates.empty())
		{
			PetLog(0, 0, PETLOG_WARN, "%s, %d|can't find feeds templates, APPID = %s, MSGID = %s",
					__FUNCTION__, __LINE__, MapKeyVal["$APPID$"].c_str(), MapKeyVal["$MSGID$"].c_str());

			return -1;
		}

		for (keyval::iterator itMap = MapKeyVal.begin(); itMap != MapKeyVal.end(); ++itMap)
		{
			if ("$APPID$" == itMap->first || "$MSGID$" == itMap->first)
			{
				continue;
			}

			strFeedsTemplates = CStrTool::StringReplace(strFeedsTemplates, itMap->first, itMap->second, true);
		}

		vsFeeds.push_back(strFeedsTemplates);
	}

	if (vsFeeds.size() != vecFeedsKeyVal.size())
	{
		PetLog(0, 0, PETLOG_WARN, "%s, %d|the feeds size(%lu) is not equal KeyVal size(%lu)",
					__FUNCTION__, __LINE__, vsFeeds.size(), vecFeedsKeyVal.size());

		return -1;
	}

	return 0;
}

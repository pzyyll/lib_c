#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "comm/hash_db/hash_db.h"
#include "comm/ini_file/ini_file.h"
#include "comm/log/pet_log.h"

using namespace snslib;

CHashDB::CHashDB()
{
    m_pstTCHDB = tchdbnew();
    memset(m_szErrMsg, 0x0, sizeof(m_szErrMsg));
}

CHashDB::~CHashDB()
{
    if(!tchdbclose(m_pstTCHDB))
    {
        PetLog(0, 0, PETLOG_ERR, "close hdb failed, ret=%d, errmsg=%s", tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
    }

    printf("close hdb over\n");

    if(m_pstTCHDB)
    {
        tchdbdel(m_pstTCHDB);
    }
}

int CHashDB::Init(const char *pszConfFile, const char * pszSection )
{
    CIniFile objIniFile(pszConfFile);

    int iDBOption = 0;
    int iDBOpenMode = 0;
    int iMtFlag = 0;
    unsigned long long ullXMSize = 0;
    unsigned long long ullMaxFileSize = 0;
    int iReadCacheNum = 0;
    int iDFUnit = 0;
    int iBucketNum = 0;
    int iAlignPower = -1;
    int iFreeBlockNumPower = 0;
    char szDBFileName[1024] = {0};
    char szDBOption[1024] = {0};
    char szOpenMode[1024] = {0};
    char szXMSize[64] = {0};
    char szMaxFileSize[64] = {0};

    if (objIniFile.IsValid())
    {
        objIniFile.GetString(pszSection, "DBFileName", "", szDBFileName, sizeof(szDBFileName));
        objIniFile.GetString(pszSection, "DBOption", "", szDBOption, sizeof(szDBOption));
        objIniFile.GetString(pszSection, "OpenMode", "", szOpenMode, sizeof(szOpenMode));

        objIniFile.GetInt(pszSection, "ReadCacheNum", 0, &iReadCacheNum);
        objIniFile.GetString(pszSection, "XMSize", "0", szXMSize, sizeof(szXMSize));
        objIniFile.GetString(pszSection, "MaxFileSize", "0", szMaxFileSize, sizeof(szMaxFileSize));
        objIniFile.GetInt(pszSection, "DFUnit", 0, &iDFUnit);
        objIniFile.GetInt(pszSection, "BucketNum", 0, &iBucketNum);
        objIniFile.GetInt(pszSection, "AlignPower", -1, &iAlignPower);
        objIniFile.GetInt(pszSection, "FreeBlockNumPower", 0, &iFreeBlockNumPower);
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "ConfFile[%s] is not valid", pszConfFile);
        return ERROR;
    }

    //读取DBOption
    if (strstr(szDBOption, "64BIT_BUCKET")!= NULL)
    {
        iDBOption|=HDBTLARGE;
    }

    if (strstr(szDBOption, "COMPRESS_BZ2")!= NULL)
    {
        iDBOption|=HDBTBZIP;
    }

    if (strstr(szDBOption, "COMPRESS_DEFLATE")!= NULL)
    {
        iDBOption|=HDBTDEFLATE;
    }

    if (strstr(szDBOption, "COMPRESS_CBS")!= NULL)
    {
        iDBOption|=HDBTTCBS;
    }

    //读取OpenMode
    if (strstr(szOpenMode, "READONLY")!= NULL)
    {
        iDBOpenMode|=HDBOREADER;
    }
    else
    {
        iDBOpenMode|=HDBOCREAT;
        iDBOpenMode|=HDBOWRITER;
    }

    if (strstr(szOpenMode, "SYNCMODE")!= NULL)
    {
        iDBOpenMode|=HDBOTSYNC;
    }

    if (strstr(szOpenMode, "WITHMUTEX")!= NULL)
    {
        iMtFlag = 1;
    }

    if (strstr(szOpenMode, "TRUNC")!= NULL)
    {
        iDBOpenMode|=HDBOTRUNC;
    }

    if (iAlignPower == 0)
    {
        iAlignPower = -1;
    }

    //解析XMSize和MaxFileSize
    if (szXMSize[0] == '\0')
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config item HASH_DB/XMSize is not valid");
        return ERROR;
    }

    if (szMaxFileSize[0] == '\0')
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "config item HASH_DB/MaxFileSize is not valid");
        return ERROR;
    }

    ullXMSize = strtoull(szXMSize, NULL, 10);
    ullMaxFileSize = strtoull(szMaxFileSize, NULL, 10);
    switch(szXMSize[strlen(szXMSize)-1])
    {
        case 'k':
        case 'K':
        {
            ullXMSize = ullXMSize <<10;
            break;
        }
        case 'm':
        case 'M':
        {
            ullXMSize = ullXMSize << 20;
            break;
        }
        case 'g':
        case 'G':
        {
            ullXMSize = ullXMSize << 30;
            break;
        }
    }

    switch(szMaxFileSize[strlen(szMaxFileSize)-1])
    {
        case 'k':
        case 'K':
        {
            ullMaxFileSize = ullMaxFileSize << 10;
            break;
        }
        case 'm':
        case 'M':
        {
            ullMaxFileSize = ullMaxFileSize << 20;
            break;
        }
        case 'g':
        case 'G':
        {
            ullMaxFileSize = ullMaxFileSize << 30;
            break;
        }
    }

    PetLog(0, 0, PETLOG_DEBUG, "HASH_DB_CONF|DBFileName|%s", szDBFileName);
    PetLog(0, 0, PETLOG_DEBUG, "HASH_DB_CONF|ReadCacheNum|%d", iReadCacheNum);
    PetLog(0, 0, PETLOG_DEBUG, "HASH_DB_CONF|XMSize|%llu", ullXMSize);
    PetLog(0, 0, PETLOG_DEBUG, "HASH_DB_CONF|MaxFileSize|%llu", ullMaxFileSize);
    PetLog(0, 0, PETLOG_DEBUG, "HASH_DB_CONF|DFUnit|%d", iDFUnit);
    PetLog(0, 0, PETLOG_DEBUG, "HASH_DB_CONF|BucketNum|%d", iBucketNum);
    PetLog(0, 0, PETLOG_DEBUG, "HASH_DB_CONF|AlignPower|%d", iAlignPower);
    PetLog(0, 0, PETLOG_DEBUG, "HASH_DB_CONF|FreeBlockNumPower|%d", iFreeBlockNumPower);
    PetLog(0, 0, PETLOG_DEBUG, "HASH_DB_CONF|DBOption|0x%x", iDBOption);
    PetLog(0, 0, PETLOG_DEBUG, "HASH_DB_CONF|DBOpenMode|0x%x", iDBOpenMode);
    PetLog(0, 0, PETLOG_DEBUG, "HASH_DB_CONF|MtFlag|%d", iMtFlag);

    if ((iMtFlag != 0) && (!tchdbsetmutex(m_pstTCHDB)))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "SetMutex failed|%d|%s", tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    if(!tchdbtune(m_pstTCHDB, iBucketNum, iAlignPower, iFreeBlockNumPower, iDBOption))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "TuneDB failed, bnum=%d, apwr=%d, fpwr=%d, opt=0x%x|%d|%s", iBucketNum, iAlignPower, iFreeBlockNumPower, iDBOption, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    if ((iReadCacheNum > 0) && (!tchdbsetcache(m_pstTCHDB, iReadCacheNum)))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "SetRCache failed, rcnum=%d|%d|%s", iReadCacheNum, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    if ((ullXMSize > 0) &&(!tchdbsetxmsiz(m_pstTCHDB, ullXMSize)))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "SetXMSize failed, xmsize=%llu|%d|%s", ullXMSize, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

//    if ((ullMaxFileSize > 0) &&(!tchdbsetmaxfsiz(m_pstTCHDB, ullMaxFileSize)))
//    {
//        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "SetMaxFileSize failed, maxfsiz=%llu|%d|%s", ullMaxFileSize, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
//        return ERROR;
//    }

    if ((iDFUnit > 0) &&(!tchdbsetdfunit(m_pstTCHDB, iDFUnit)))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "SetDFUnit failed, dfunit=%d|%d|%s", iDFUnit, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    if(!tchdbopen(m_pstTCHDB, szDBFileName, iDBOpenMode))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "OpenDB failed|%d|%s", tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    return SUCCESS;
}

int CHashDB::Put(const void *pvKeyBuff, int iKeySize, const void *pvValBuff, int iValSize)
{
    if(!tchdbput(m_pstTCHDB, pvKeyBuff, iKeySize, pvValBuff, iValSize))
    {
        if (tchdbecode(m_pstTCHDB) == TCEDBFULL)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return HDBE_DBFULL;
        }

        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    return SUCCESS;
}

int CHashDB::Put2(const char *pszKey, const char *pszVal)
{
    return Put(pszKey, strlen(pszKey), pszVal, strlen(pszVal));
}

int CHashDB::PutKeep(const void *pvKeyBuff, int iKeySize, const void *pvValBuff, int iValSize)
{
    if(!tchdbputkeep(m_pstTCHDB, pvKeyBuff, iKeySize, pvValBuff, iValSize))
    {
        if (tchdbecode(m_pstTCHDB) == TCEDBFULL)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return HDBE_DBFULL;
        }

        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    return SUCCESS;
}

int CHashDB::PutKeep2(const char *pszKey, const char *pszVal)
{
    return PutKeep(pszKey, strlen(pszKey), pszVal, strlen(pszVal));
}

int CHashDB::PutCat(const void *pvKeyBuff, int iKeySize, const void *pvValBuff, int iValSize)
{
    if(!tchdbputcat(m_pstTCHDB, pvKeyBuff, iKeySize, pvValBuff, iValSize))
    {
        if (tchdbecode(m_pstTCHDB) == TCEDBFULL)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return HDBE_DBFULL;
        }

        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    return SUCCESS;
}

int CHashDB::PutCat2(const char *pszKey, const char *pszVal)
{
    return PutCat(pszKey, strlen(pszKey), pszVal, strlen(pszVal));;
}

int CHashDB::Out(const void *pvKeyBuff, int iKeySize)
{
    if(!tchdbout(m_pstTCHDB, pvKeyBuff, iKeySize))
    {
        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }

        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    return SUCCESS;
}

int CHashDB::Out2(const char *pszKey)
{
    return Out(pszKey, strlen(pszKey));
}

int CHashDB::Get(const void *pvKeyBuff, int iKeySize, void **ppvValBuff, int *piValSize)
{
    *ppvValBuff = tchdbget(m_pstTCHDB, pvKeyBuff, iKeySize, piValSize);
    if ((*ppvValBuff) == NULL)
    {
        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return ERROR;
        }
    }

    return SUCCESS;
}

int CHashDB::Get2(const char *pszKey, char **ppszVal)
{
    *ppszVal = tchdbget2(m_pstTCHDB, pszKey);
    if (*ppszVal == NULL)
    {
        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return ERROR;
        }
    }

    return SUCCESS;
}

int CHashDB::Get3(const void *pvKeyBuff, int iKeySize, void *pvValBuff, int *piValBuffSize)
{
    int iValSize = tchdbget3(m_pstTCHDB, pvKeyBuff, iKeySize, pvValBuff, *piValBuffSize);
    if (iValSize < 0)
    {
        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%d|%s", __func__, iValSize, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return ERROR;
        }
    }

    *piValBuffSize = iValSize;

    return SUCCESS;
}

int CHashDB::GetVSize(const void *pvKeyBuff, int iKeySize, int *piValSize)
{
    *piValSize = tchdbvsiz(m_pstTCHDB, pvKeyBuff, iKeySize);
    if (*piValSize < 0)
    {
        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return ERROR;
        }
    }

    return SUCCESS;
}


int CHashDB::GetVSize2(const char *pszKey, int *piValSize)
{
    *piValSize = tchdbvsiz2(m_pstTCHDB, pszKey);
    if (*piValSize < 0)
    {
        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }
        else
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return ERROR;
        }
    }

    return SUCCESS;
}

int CHashDB::IterInit(const void *pvKeyBuff /*= NULL*/, int iKeySize /*= 0*/)
{
    if ((iKeySize == 0) || (pvKeyBuff == NULL))
    {
        if (!tchdbiterinit(m_pstTCHDB))
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return ERROR;
        }
    }
    else
    {
        if (!tchdbiterinit2(m_pstTCHDB, pvKeyBuff, iKeySize))
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return ERROR;
        }
    }

    return SUCCESS;
}


int CHashDB::IterInit2(const char *pszKey /*= NULL*/)
{
    if (pszKey == NULL)
    {
        if (!tchdbiterinit(m_pstTCHDB))
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return ERROR;
        }
    }
    else
    {
        if (!tchdbiterinit3(m_pstTCHDB, pszKey))
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
            return ERROR;
        }
    }

    return SUCCESS;

}

int CHashDB::IterNextKey(void **ppvKeyBuff, int *piKeySize)
{
    *ppvKeyBuff = tchdbiternext(m_pstTCHDB, piKeySize);
    if (*ppvKeyBuff == NULL)
    {
        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }

        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    return SUCCESS;
}

int CHashDB::IterNextKey2(char **ppszKeyBuff)
{
    *ppszKeyBuff = tchdbiternext2(m_pstTCHDB);
    if (*ppszKeyBuff == NULL)
    {
        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }

        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    return SUCCESS;
}

int CHashDB::IterNextKey3(void *pvKeyBuff, int *piKeyBuffSize)
{
    void *pvKeyBuffTmp = NULL;
    int iKeyBuffSizeTmp = 0;
    pvKeyBuffTmp = tchdbiternext(m_pstTCHDB, &iKeyBuffSizeTmp);
    if (pvKeyBuffTmp == NULL)
    {
        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }

        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    if (*piKeyBuffSize < iKeyBuffSizeTmp)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|buffer not enough", __func__);
        free(pvKeyBuffTmp);
        return ERROR;
    }

    memcpy(pvKeyBuff, pvKeyBuffTmp, iKeyBuffSizeTmp);
    *piKeyBuffSize = iKeyBuffSizeTmp;

    free(pvKeyBuffTmp);

    return SUCCESS;
}

int CHashDB::IterNext(void **ppvKeyBuff, int *piKeySize, void **ppvValBuff, int *piValSize)
{
    TCXSTR *pstKey = tcxstrnew();
    TCXSTR *pstVal = tcxstrnew();
    if (!tchdbiternext3(m_pstTCHDB, pstKey, pstVal))
    {
        tcxstrdel(pstKey);
        tcxstrdel(pstVal);

        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|buffer not enough", __func__);
        return ERROR;
    }

    *ppvKeyBuff = malloc(pstKey->size);
    *ppvValBuff = malloc(pstVal->size);

    memcpy(*ppvKeyBuff, pstKey->ptr, pstKey->size);
    memcpy(*ppvValBuff, pstVal->ptr, pstVal->size);

    *piKeySize = pstKey->size;
    *piValSize = pstVal->size;

    tcxstrdel(pstKey);
    tcxstrdel(pstVal);

    return SUCCESS;
}

int CHashDB::IterNext2(char **ppszKeyBuff, char **ppszValBuff)
{
    TCXSTR *pstKey = tcxstrnew();
    TCXSTR *pstVal = tcxstrnew();
    if (!tchdbiternext3(m_pstTCHDB, pstKey, pstVal))
    {
        tcxstrdel(pstKey);
        tcxstrdel(pstVal);

        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }

        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    *ppszKeyBuff = (char *)malloc(pstKey->size+1);
    *ppszValBuff = (char *)malloc(pstVal->size+1);

    memcpy(*ppszKeyBuff, pstKey->ptr, pstKey->size);
    memcpy(*ppszValBuff, pstVal->ptr, pstVal->size);

    (*ppszKeyBuff)[pstKey->size] = '\0';
    (*ppszValBuff)[pstVal->size] = '\0';

    tcxstrdel(pstKey);
    tcxstrdel(pstVal);

    return SUCCESS;
}

int CHashDB::IterNext3(void *pvKeyBuff, int *piKeyBuffSize, void *pvValBuff, int *piValBuffSize)
{
    TCXSTR *pstKey = tcxstrnew();
    TCXSTR *pstVal = tcxstrnew();
    if (!tchdbiternext3(m_pstTCHDB, pstKey, pstVal))
    {
        tcxstrdel(pstKey);
        tcxstrdel(pstVal);

        if (tchdbecode(m_pstTCHDB) == TCENOREC)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no record");
            return HDBE_NORECORD;
        }

        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    if ((*piKeyBuffSize < pstKey->size)||(*piValBuffSize < pstVal->size))
    {
        tcxstrdel(pstKey);
        tcxstrdel(pstVal);
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|buffer not enough", __func__);
        return ERROR;
    }

    memcpy(pvKeyBuff, pstKey->ptr, pstKey->size);
    memcpy(pvValBuff, pstVal->ptr, pstVal->size);

    *piKeyBuffSize = pstKey->size;
    *piValBuffSize = pstVal->size;

    tcxstrdel(pstKey);
    tcxstrdel(pstVal);

    return SUCCESS;
}

int CHashDB::VanishDB()
{
    if (!tchdbvanish(m_pstTCHDB))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s failed|%d|%s", __func__, tchdbecode(m_pstTCHDB), tchdberrmsg(tchdbecode(m_pstTCHDB)));
        return ERROR;
    }

    return SUCCESS;
}

unsigned long long CHashDB::GetFileSize()
{
    return tchdbfsiz(m_pstTCHDB);
}

unsigned long long CHashDB::GetRecordNum()
{
    return tchdbrnum(m_pstTCHDB);
}

void CHashDB::ShowDBInfo()
{
    printf("DBINFO[%s]\n", m_pstTCHDB->path);
    printf(" BucketNum  = %llu\n", (unsigned long long)m_pstTCHDB->bnum);
    printf(" AlignPow   = %d\n", m_pstTCHDB->apow);
    printf(" FBNumPow   = %d\n", m_pstTCHDB->fpow);
    printf(" RecordNum  = %llu\n", (unsigned long long)m_pstTCHDB->rnum);
    printf(" FileSize   = %llu\n", (unsigned long long)m_pstTCHDB->fsiz);
    printf(" FstRecord  = 0x%08llX\n", (unsigned long long)m_pstTCHDB->frec);
    printf(" DFCursor   = 0x%08llX\n", (unsigned long long)m_pstTCHDB->dfcur);
    printf(" MapSize    = %llu\n", (unsigned long long)m_pstTCHDB->msiz);
    printf(" XMapSize   = %llu\n", (unsigned long long)m_pstTCHDB->xmsiz);
    printf(" Align      = %d\n", m_pstTCHDB->align);
    printf(" FBPMax     = %d\n", m_pstTCHDB->fbpmax);
    printf(" FBPNum     = %d\n", m_pstTCHDB->fbpnum);
    printf(" FBPMis     = %d\n", m_pstTCHDB->fbpmis);
    printf(" DFUnit     = %d\n", m_pstTCHDB->dfunit);
    printf(" DFCNT      = %d\n", m_pstTCHDB->dfcnt);

    char szTmp[1024000] = {0};
    int iBuffLen = 0;
    HDBFB *cur = (HDBFB *)m_pstTCHDB->fbpool;
    for (int i=0; i<m_pstTCHDB->fbpnum; i++)
    {
        iBuffLen += snprintf(szTmp+iBuffLen, sizeof(szTmp)-iBuffLen, "0x%llX|%d ", cur->off, cur->rsiz);
        cur++;
    }
    printf(" FBLIST     = %s\n", szTmp);

    return;
}



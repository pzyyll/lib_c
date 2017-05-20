/**
 * @file    pet_util.h
 * @brief   一些封装好的使用工具函数
 * @author  jamieli@tencent.com
 * @date    2009-03-03
 */

#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <iconv.h>
#include <limits.h>
#include "pet_util.h"
#include "net/if.h"
#include "sys/ioctl.h"
#include <sys/time.h>
#include <time.h>
#include <algorithm>
#include <sstream>


using namespace snslib;
using namespace std;

inline long long htonll(long long llHost)
{
    long long llNet = (long long)ntohl((int)llHost) << 32;
    llNet += ntohl((int)(llHost >> 32));

    return llNet;
}

inline long long ntohll(long long llNet)
{
    long long llHost = (long long)ntohl((int)llNet) << 32;
    llHost += ntohl((int)(llNet >> 32));

    return llHost;
}

int CBuffTool::ReadByte(const void* pvBuffer, unsigned char &ucVal)
{
    memcpy(&ucVal, pvBuffer, sizeof(unsigned char));
    return sizeof(unsigned char);
}

int CBuffTool::ReadByte(const void* pvBuffer, char &cVal)
{
    memcpy(&cVal, pvBuffer, sizeof(char));
    return sizeof(char);
}

int CBuffTool::WriteByte(void* pvBuffer, unsigned char ucVal)
{
    memcpy(pvBuffer, &ucVal, sizeof(unsigned char));
    return sizeof(unsigned char);
}

int CBuffTool::WriteByte(void* pvBuffer, char cVal)
{
    memcpy(pvBuffer, &cVal, sizeof(char));
    return sizeof(char);
}


int CBuffTool::ReadShort(const void* pvBuffer, unsigned short &ushVal, int iToHostOrder/* = 1*/)
{
    memcpy(&ushVal, pvBuffer, sizeof(unsigned short));
    if (iToHostOrder == 1)
    {
        ushVal = ntohs(ushVal);
    }
    return sizeof(unsigned short);
}

int CBuffTool::ReadShort(const void* pvBuffer, short &shVal, int iToHostOrder/* = 1*/)
{
    memcpy(&shVal, pvBuffer, sizeof(short));
    if (iToHostOrder == 1)
    {
        shVal = ntohs(shVal);
    }
    return sizeof(short);
}

int CBuffTool::WriteShort(void* pvBuffer, unsigned short ushVal, int iToNetOrder/* = 1*/)
{
    if (iToNetOrder == 1)
    {
        ushVal = htons(ushVal);
    }
    memcpy(pvBuffer, &ushVal, sizeof(unsigned short));
    return sizeof(unsigned short);
}

int CBuffTool::WriteShort(void* pvBuffer, short shVal, int iToNetOrder/* = 1*/)
{
    if (iToNetOrder == 1)
    {
        shVal = htons(shVal);
    }
    memcpy(pvBuffer, &shVal, sizeof(short));
    return sizeof(short);
}


int CBuffTool::ReadInt(const void* pvBuffer, unsigned int &uiVal, int iToHostOrder/* = 1*/)
{
    memcpy(&uiVal, pvBuffer, sizeof(unsigned int));
    if (iToHostOrder == 1)
    {
        uiVal = ntohl(uiVal);
    }
    return sizeof(unsigned int);
}

int CBuffTool::ReadInt(const void* pvBuffer, int &iVal, int iToHostOrder/* = 1*/)
{
    memcpy(&iVal, pvBuffer, sizeof(int));
    if (iToHostOrder == 1)
    {
        iVal = ntohl(iVal);
    }
    return sizeof(int);
}

int CBuffTool::WriteInt(void* pvBuffer, unsigned int uiVal, int iToNetOrder/* = 1*/)
{
    if (iToNetOrder == 1)
    {
        uiVal = htonl(uiVal);
    }
    memcpy(pvBuffer, &uiVal, sizeof(unsigned int));
    return sizeof(unsigned int);
}

int CBuffTool::WriteInt(void* pvBuffer, int iVal, int iToNetOrder/* = 1*/)
{
    if (iToNetOrder == 1)
    {
        iVal = htonl(iVal);
    }
    memcpy(pvBuffer, &iVal, sizeof(int));
    return sizeof(int);
}


int CBuffTool::ReadLong(const void* pvBuffer, unsigned long &ulVal, int iToHostOrder/* = 1*/)
{
    memcpy(&ulVal, pvBuffer, sizeof(unsigned long));
    if (iToHostOrder == 1)
    {
        ulVal = ntohl(ulVal);
    }
    return sizeof(unsigned long);
}

int CBuffTool::ReadLong(const void* pvBuffer, long &lVal, int iToHostOrder/* = 1*/)
{
    memcpy(&lVal, pvBuffer, sizeof(long));
    if (iToHostOrder == 1)
    {
        lVal = ntohl(lVal);
    }
    return sizeof(long);
}

int CBuffTool::WriteLong(void* pvBuffer, unsigned long ulVal, int iToNetOrder/* = 1 */)
{
    if (iToNetOrder == 1)
    {
        ulVal = htonl(ulVal);
    }
    memcpy(pvBuffer, &ulVal, sizeof(unsigned long));
    return sizeof(unsigned long);
}

int CBuffTool::WriteLong(void* pvBuffer, long lVal, int iToNetOrder/* = 1 */)
{
    if (iToNetOrder == 1)
    {
        lVal = htonl(lVal);
    }
    memcpy(pvBuffer, &lVal, sizeof(long));
    return sizeof(long);
}


int CBuffTool::ReadLongLong(const void* pvBuffer, unsigned long long &ullVal, int iToHostOrder/* = 0*/)
{
    memcpy(&ullVal, pvBuffer, sizeof(unsigned long long));
    if (iToHostOrder == 1)
    {
        ullVal = ntohll(ullVal);
    }
    return sizeof(unsigned long long);
}

int CBuffTool::ReadLongLong(const void* pvBuffer, long long &llVal, int iToHostOrder/* = 0*/)
{
    memcpy(&llVal, pvBuffer, sizeof(long long));
    if (iToHostOrder == 1)
    {
        llVal = ntohll(llVal);
    }
    return sizeof(long long);
}

#if __WORDSIZE == 64
int CBuffTool::ReadLongLong(const void* pvBuffer, uint64_t &ullVal, int iToHostOrder/* = 0*/)
{
    memcpy(&ullVal, pvBuffer, sizeof(unsigned long long));
    if (iToHostOrder == 1)
    {
        ullVal = ntohll(ullVal);
    }
    return sizeof(unsigned long long);
}

int CBuffTool::WriteLongLong(void* pvBuffer, uint64_t ullVal, int iToNetOrder/* = 0*/)
{
    if (iToNetOrder == 1)
    {
        ullVal = htonll(ullVal);
    }
    memcpy(pvBuffer, &ullVal, sizeof(unsigned long long));
    return sizeof(unsigned long long);
}
#endif

int CBuffTool::WriteLongLong(void* pvBuffer, unsigned long long ullVal, int iToNetOrder/* = 0*/)
{
    if (iToNetOrder == 1)
    {
        ullVal = htonll(ullVal);
    }
    memcpy(pvBuffer, &ullVal, sizeof(unsigned long long));
    return sizeof(unsigned long long);
}

int CBuffTool::WriteLongLong(void* pvBuffer, long long llVal, int iToNetOrder/* = 0*/)
{
    if (iToNetOrder == 1)
    {
        llVal = htonll(llVal);
    }
    memcpy(pvBuffer, &llVal, sizeof(long long));
    return sizeof(long long);
}


int CBuffTool::ReadString(const void* pvBuffer, char *pszVal, int iStrLen)
{
    memcpy(pszVal, pvBuffer, iStrLen);
    return iStrLen;
}

int CBuffTool::WriteString(void* pvBuffer, const char *pszVal, int iStrLen)
{
    memcpy(pvBuffer, pszVal, iStrLen);
    return iStrLen;
}

int CBuffTool::ReadBuf(const void* pSrc, void *pDest, int iLen)
{
    memcpy(pDest, pSrc, iLen);
    return iLen;
}

int CBuffTool::WriteBuf(void* pDest, const void *pSrc, int iLen)
{
    memcpy(pDest, pSrc, iLen);
    return iLen;
}

bool CFileUtil::ReadFile(const std::string &path, std::string &data)
{
  FILE * file = fopen(path.c_str(), "rb");
  if (!file)
    return false;

  fseek(file, 0, SEEK_END);
  size_t len = ftell(file);
  rewind(file);

  data.resize(len);

  size_t ret = fread((char*)data.data()[0], 1, len, file);
  fclose(file);

  if (ret < len)
    return false;

  return true;
}

bool CFileUtil::ReadFile(const std::string &path, char *ptr, size_t &len)
{
  FILE * file = fopen(path.c_str(), "rb");
  if (!file) 
    return false;

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  fseek(file, 0L, SEEK_SET);

  if (size > len)
    size = len;

  len = size;

  size_t ret = fread(ptr, 1, len, file);
  fclose(file);

  if (ret < len)
    return false;

  return true;
}


bool CFileUtil::FileExists(const std::string &path)
{
  return access(path.c_str(), F_OK) == 0;
}

const char *CStrTool::Str2Hex(const void *pvBuff, int iSize)
{
    const int STR2HEX_MAX_BUFF_LEN = 10240;
    const int CHAR_NUM_PER_BYTE = 2;
    static char szStr2HexBuff[(STR2HEX_MAX_BUFF_LEN+1) * CHAR_NUM_PER_BYTE] = {0};

    if (iSize > STR2HEX_MAX_BUFF_LEN)
    {
        iSize = STR2HEX_MAX_BUFF_LEN;
    }

    for (int i=0; i<iSize; i++)
    {
        sprintf(szStr2HexBuff + (i * CHAR_NUM_PER_BYTE), "%02X", ((unsigned char*)pvBuff)[i]);
    }

    if (iSize == 0)
    {
        szStr2HexBuff[0] = 0;
    }

    return szStr2HexBuff;
}

const char *CStrTool::Hex2Str(const char *pBuff, int iSize, int *pSize){
	static vector<char> buffer;
	buffer.resize(iSize);

	int idx = 0;
	char one_byte[3];
	memset(one_byte, 0x0, sizeof(one_byte));

	for(int i = 0; i < iSize/2; ++i){
		memcpy(one_byte, &pBuff[i * 2], 2); 
		buffer[idx++] = strtol(one_byte, NULL, 16);
	}   
	buffer[idx] = 0;

	if(pSize) *pSize = idx;

	return &buffer[0];
}

bool CStrTool::CheckValidNameGBK(const char *pszInStr, char *pszOutStr)
{
    if ((pszInStr == NULL)||(pszOutStr == NULL))
    {
        return false;
    }

    const unsigned char *pcNowChar = (const unsigned char *)pszInStr;
    const int MAX_CHECK_NAME_LEN = 1024;

    bool bRetVal = true;

    int iStrLen = 0;
    while(*pcNowChar != '\0')
    {
        if(*pcNowChar < 0x80)
        {
            if (((*pcNowChar >= '0')&&(*pcNowChar <= '9'))
                ||((*pcNowChar >= 'A')&&(*pcNowChar <= 'Z'))
                ||((*pcNowChar >= 'a')&&(*pcNowChar <= 'z'))
                ||(*pcNowChar == '_'))
            {
                pszOutStr[iStrLen] = *pcNowChar;
            }
            else
            {
                pszOutStr[iStrLen] = '_';
                bRetVal = false;
            }
            pcNowChar++;
            iStrLen++;
        }
        else
        {
            const unsigned char *pcNextChar = pcNowChar+1;
            if (((*pcNowChar >= 0xA1)&&(*pcNowChar <= 0xA9)&&(*pcNextChar >= 0xA1)&&(*pcNextChar <= 0xFE))
                ||((*pcNowChar >= 0xB0)&&(*pcNowChar <= 0xF7)&&(*pcNextChar >= 0xA1)&&(*pcNextChar <= 0xFE))
                ||((*pcNowChar >= 0x81)&&(*pcNowChar <= 0xA0)&&(*pcNextChar >= 0x40)&&(*pcNextChar <= 0xFE))
                ||((*pcNowChar >= 0xAA)&&(*pcNowChar <= 0xFE)&&(*pcNextChar >= 0x40)&&(*pcNextChar <= 0xA0))
                ||((*pcNowChar >= 0xA8)&&(*pcNowChar <= 0xA9)&&(*pcNextChar >= 0x40)&&(*pcNextChar <= 0xA0)))
            {
                //GBK字符
                if (((*pcNowChar == 0xA1)&&(*pcNextChar == 0xA1))
                    ||((*pcNowChar == 0xA1)&&(*pcNextChar == 0xB2))
                    ||((*pcNowChar == 0xA1)&&(*pcNextChar == 0xB3))
                    ||((*pcNowChar == 0xA3)&&(*pcNextChar == 0xA8))
                    ||((*pcNowChar == 0xA3)&&(*pcNextChar == 0xA9))
                    ||((*pcNowChar == 0xA9)&&(*pcNextChar == 0x76))
                    ||((*pcNowChar == 0xA9)&&(*pcNextChar == 0x77))
                    ||((*pcNowChar == 0xA9)&&(*pcNextChar == 0x7A))
                    ||((*pcNowChar == 0xA9)&&(*pcNextChar == 0x7B))
                    ||((*pcNowChar >= 0x81)&&(*pcNowChar <= 0xA0)&&(*pcNextChar == 0x7F))
                    ||((*pcNowChar >= 0xAA)&&(*pcNowChar <= 0xFE)&&(*pcNextChar == 0x7F))
                    ||((*pcNowChar >= 0xA8)&&(*pcNowChar <= 0xA9)&&(*pcNextChar == 0x7F)))
                {
                    pszOutStr[iStrLen] = 0xA1;
                    pszOutStr[iStrLen+1] = 0xF5;
                    bRetVal = false;
                }
                else
                {
                    pszOutStr[iStrLen] = *pcNowChar;
                    pszOutStr[iStrLen+1] = *pcNextChar;
                }

                pcNowChar+=2;
                iStrLen+=2;
            }
            else
            {
                pszOutStr[iStrLen] = '_';
                bRetVal = false;
                pcNowChar++;
                iStrLen++;
            }
        }

        if (iStrLen >= MAX_CHECK_NAME_LEN)
        {
            bRetVal = false;
            break;
        }
    }

    pszOutStr[iStrLen] = '\0';

    return bRetVal;
}

const char *CStrTool::IPString(unsigned int uiIP)
{
    //允许最多20个参数
    static char szIPStringBuff[20][16];
    static unsigned char cIPStringIdx;
    cIPStringIdx++;
    if (cIPStringIdx >= 20)
    {
        cIPStringIdx = 0;
    }
    snprintf(szIPStringBuff[cIPStringIdx], sizeof(szIPStringBuff[cIPStringIdx]), "%s", inet_ntoa(*(struct in_addr *)&uiIP));
    return szIPStringBuff[cIPStringIdx];
}

void CStrTool::StripString(string* psStr, const char* pszRemove, char chReplaceWith)
{
    const char * pszStart = psStr->c_str();
    const char * p = pszStart;
    for (p = strpbrk(p, pszRemove); p != NULL; p = strpbrk(p + 1, pszRemove))
    {
        (*psStr)[p - pszStart] = chReplaceWith;
    }
}

void CStrTool::StringReplace(const string& sStr, const string& sOldSub,
    const string& sNewSub, bool bReplaceAll, string* psRes)
{
    if (sOldSub.empty())
    {
        psRes->append(sStr);  // if empty, append the given string.
        return;
    }

    std::string::size_type iStartPos = 0;
    std::string::size_type iPos;
    do
    {
        iPos = sStr.find(sOldSub, iStartPos);
        if (iPos == string::npos)
        {
            break;
        }
        psRes->append(sStr, iStartPos, iPos - iStartPos);
        psRes->append(sNewSub);
        iStartPos = iPos + sOldSub.size();
    } while (bReplaceAll);
    psRes->append(sStr, iStartPos, sStr.length() - iStartPos);
}

string CStrTool::StringReplace(const string& sStr, const string& sOldSub,
                     const string& sNewSub, bool bReplaceAll)
{
    string sRet;
    StringReplace(sStr, sOldSub, sNewSub, bReplaceAll, &sRet);
    return sRet;
}

void CStrTool::SplitStringUsing(const string& sFull,
    const char* pszDelim, vector<string>* pvsResult)
{
    SplitStringUsing(sFull, pszDelim, pvsResult, 0);
}

void CStrTool::SplitStringUsing(const string& sFull,
    const char* pszDelim, vector<string>* pvsResult, int iMaxSplitTimes)
{
    if (iMaxSplitTimes <= 0)
    {
        iMaxSplitTimes = INT_MAX;
    }

    int iCurSplitTimes = 0;

    if (sFull.empty())
    {
        pvsResult->push_back(sFull);
        return;
    }

    // Check if the delimiter is a single character
    if (pszDelim[0] != '\0' && pszDelim[1] == '\0')
    {
        char c = pszDelim[0];
        const char* p = sFull.data();
        const char* pEnd = p + sFull.size();
        const char* pStart = p;
        while (p != pEnd)
        {
            if (*p == c)
            {
                pvsResult->push_back(string(pStart, p - pStart));
                iCurSplitTimes++;
                pStart = p + 1;
                if (iCurSplitTimes >= iMaxSplitTimes)
                {
                    pvsResult->push_back(string(pStart, pEnd - pStart));
                    return;
                }
            }
            ++p;
        }
        pvsResult->push_back(string(pStart, pEnd - pStart));

        return;
    }

    // Process multi-characters delimiter, Split by any occurrance
    string::size_type iBeginIndex, iEndIndex;
    iBeginIndex = 0; // sFull.find_first_not_of(pszDelim);
    while (iBeginIndex != string::npos)
    {
        iEndIndex = sFull.find_first_of(pszDelim, iBeginIndex);
        if (iEndIndex == string::npos)
        {
            pvsResult->push_back(sFull.substr(iBeginIndex));
            return;
        }

        pvsResult->push_back(sFull.substr(iBeginIndex, iEndIndex - iBeginIndex));
        iBeginIndex = iEndIndex + 1;
        iCurSplitTimes++;
        if (iCurSplitTimes >= iMaxSplitTimes)
        {
            pvsResult->push_back(sFull.substr(iBeginIndex));
            return;
        }
    }
}

string CStrTool::SimpleItoa(int i)
{
    static const int ITOA_BUFFER_LEN = 32;
    static const int ITOA_BUFFER_OFFSET = 11;
    char szBuf[ITOA_BUFFER_LEN];

    char* p = szBuf + ITOA_BUFFER_OFFSET;
    *p-- = '\0';
    if (i >= 0)
    {
        do
        {
            *p-- = '0' + i % 10;
            i /= 10;
        } while (i > 0);
        return p + 1;
    }
    else
    {
        if (i > -10)
        {
            i = -i;
            *p-- = '0' + i;
            *p = '-';
            return p;
        }
        else
        {
            // Make sure the value is not MIN_INT
            i = i + 10;
            i = -i;
            *p-- = '0' + i % 10;

            i = i / 10 + 1;
            do
            {
                *p-- = '0' + i % 10;
                i /= 10;
            } while (i > 0);
            *p = '-';
            return p;
        }
    }
}

void CStrTool::JoinStrings(const vector<string>& vsItems, const char* pszDelim,
    string * psResult)
{
    psResult->clear();
    int iDelimLen = strlen(pszDelim);

    typedef vector<string>::const_iterator Iterator;

    // Compute the size first
    int iLength = 0;
    for (Iterator iter = vsItems.begin(); iter != vsItems.end(); ++iter)
    {
        if (iter != vsItems.begin())
        {
            iLength += iDelimLen;
        }
        iLength += iter->size();
    }
    psResult->reserve(iLength);

    for (Iterator iter = vsItems.begin(); iter != vsItems.end(); ++iter)
    {
        if (iter != vsItems.begin())
        {
            psResult->append(pszDelim, iDelimLen);
        }
        psResult->append(iter->data(), iter->size());
    }
}

string CStrTool::JoinStrings(const vector<string>& vsItems, const char* pszDelim)
{
    string sResult;
    JoinStrings(vsItems, pszDelim, &sResult);
    return sResult;
}

void CStrTool::InternalAppend(std::string& sDst, const char* pszFmt, va_list stApList)
{
    // Use 4k bytes for the first try, should be enough for most cases
    char szSpace[4096];

    int iSize = sizeof(szSpace);
    char* pBuff = szSpace;
    int iResult = 0;

    do
    {
        iResult = vsnprintf(pBuff, iSize, pszFmt, stApList);
        va_end(stApList);

        if ((iResult >= 0) && iResult < iSize)
        {
            // Fit the buffer exactly
            break;
        }

        if (iResult < 0)
        {
            // Double the size of buffer
            iSize *= 2;
        }
        else
        {
            // Need result+1 exactly
            iSize = iResult + 1;
        }

        pBuff = (char*) (pBuff == szSpace ? malloc(iSize) : realloc(pBuff, iSize));

        if (!pBuff)
        {
            // Is it ok to throw an exception here?
            throw std::bad_alloc();
        }
    }
    while (true);

    sDst.append(pBuff, iResult);

    if (pBuff != szSpace)
    {
        free(pBuff);
    }

    return;
}

std::string CStrTool::Format(const char* pszFmt, ...)
{
    va_list stApList;
    va_start(stApList, pszFmt);
    std::string sResult;
    InternalAppend(sResult, pszFmt, stApList);
    va_end(stApList);
    return sResult;
}

void CStrTool::Append(std::string& sDst, const char* pszFmt, ...)
{
    va_list stApList;
    va_start(stApList, pszFmt);
    InternalAppend(sDst, pszFmt, stApList);
    va_end(stApList);
}

unsigned short CStrTool::CheckSum(const void *pvBuff, int iSize)
{
    unsigned short ushSum = 0;
    const unsigned char *pszBuff = (unsigned char *)pvBuff;

    for (int i = 0; i < iSize/2; ++i)
    {
        ushSum ^= *(short *)((char *)pszBuff + i * 2);
    }

    return ushSum;
}

const char *CStrTool::TimeString(time_t tTime)
{
    static char szTimeStringBuff[32];
    memset(szTimeStringBuff,0,sizeof(szTimeStringBuff));
    struct tm * ptm = localtime(&tTime);
    strftime(szTimeStringBuff, sizeof(szTimeStringBuff), "%Y-%m-%d %H:%M:%S", ptm);

    return szTimeStringBuff;
}

const char *CStrTool::TimeString(struct timeval stTime)
{
    //允许最多20个参数
    static char szTimeStringBuff[20][32];
    static unsigned char cTimerStringIdx;

    cTimerStringIdx++;

    if (cTimerStringIdx >= 20)
    {
        cTimerStringIdx = 0;
    }

    struct tm * ptm = localtime(&stTime.tv_sec);
    int year = 1900 + ptm->tm_year; //从1900年开始的年数
    int month = ptm->tm_mon + 1; //从0开始的月数
    int day = ptm->tm_mday; //从1开始的天数
    int hour = ptm->tm_hour; //从0开始的小时数
    int min = ptm->tm_min; //从0开始的分钟数
    int sec = ptm->tm_sec; //从0开始的秒数

    snprintf(szTimeStringBuff[cTimerStringIdx], sizeof(szTimeStringBuff[cTimerStringIdx]), "%04d-%02d-%02d %02d:%02d:%02d.%06ld", year, month, day, hour, min, sec, stTime.tv_usec);

    return szTimeStringBuff[cTimerStringIdx];
}

time_t CStrTool::FromTimeString(const char *pszTimeStr)
{
    struct tm stTime;
    memset(&stTime, 0, sizeof(stTime));
    strptime(pszTimeStr, "%Y-%m-%d %H:%M:%S", &stTime);
    return mktime(&stTime);
}

static const char PET_UTIL_B64_NORMAL_STR[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char PET_UTIL_B64_URL_STR[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789*-";

int CStrTool::Base64Encode(const void *pvDataBuff, int iDataLen, void *pvEncodeDataBuff, int *iEncodeDataLen, int iBase64Type/*= BASE64_TYPE_NORMAL*/)
{
    const char *pszB64Str = NULL;
    if (iBase64Type == BASE64_TYPE_NORMAL)
    {
        pszB64Str = PET_UTIL_B64_NORMAL_STR;
    }
    else if (iBase64Type == BASE64_TYPE_URL)
    {
        pszB64Str = PET_UTIL_B64_URL_STR;
    }
    else
    {
        return -1;
    }

    const unsigned char *pucDataIn = (unsigned char *)pvDataBuff;
    int iDataInRemainLen = iDataLen;
    char *pucDataOut = (char *)pvEncodeDataBuff;
    int iDataOutLen = 0;

    while (iDataInRemainLen > 0)
    {
        if (iDataInRemainLen >= 3)
        {
            if ((*iEncodeDataLen - iDataOutLen) < 4)
            {
                return -2;
            }
            pucDataOut[0] = pszB64Str[(pucDataIn[0] >> 2) & 0x3F];
            pucDataOut[1] = pszB64Str[((pucDataIn[0] << 4) + (pucDataIn[1] >> 4)) & 0x3F];
            pucDataOut[2] = pszB64Str[((pucDataIn[1] << 2) + (pucDataIn[2] >> 6)) & 0x3F];
            pucDataOut[3] = pszB64Str[pucDataIn[2]& 0x3F];

            pucDataOut += 4;
            iDataOutLen += 4;

            pucDataIn += 3;
            iDataInRemainLen -= 3;
        }
        else if (iDataInRemainLen == 2)
        {
            if ((*iEncodeDataLen - iDataOutLen) < 3)
            {
                return -2;
            }

            pucDataOut[0] = pszB64Str[(pucDataIn[0] >> 2) & 0x3F];
            pucDataOut[1] = pszB64Str[((pucDataIn[0] << 4) + (pucDataIn[1] >> 4)) & 0x3F];
            pucDataOut[2] = pszB64Str[(pucDataIn[1] << 2) & 0x3F];

            pucDataOut += 3;
            iDataOutLen += 3;

            pucDataIn += 2;
            iDataInRemainLen -= 2;
        }
        else if (iDataInRemainLen == 1)
        {
            if ((*iEncodeDataLen - iDataOutLen) < 2)
            {
                return -2;
            }

            pucDataOut[0] = pszB64Str[(pucDataIn[0] >> 2) & 0x3F];
            pucDataOut[1] = pszB64Str[(pucDataIn[0] << 4) & 0x3F];

            pucDataOut += 2;
            iDataOutLen += 2;

            pucDataIn += 1;
            iDataInRemainLen -= 1;
        }
    }

    *iEncodeDataLen = iDataOutLen;

    return 0;
}


static const char BASE64_TYPE_NORMAL_DE[0x100] =
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static const char BASE64_TYPE_URL_DE[0x100] =
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, 63, -1, -1,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int CStrTool::Base64Decode(const void *pvDataBuff, int iDataLen, void *pvDecodeDataBuff, int *iDecodeDataLen, int iBase64Type/*= BASE64_TYPE_NORMAL*/)
{
    const char *pszB64StrDe = NULL;
    if (iBase64Type == BASE64_TYPE_NORMAL)
    {
        pszB64StrDe = BASE64_TYPE_NORMAL_DE;
    }
    else if (iBase64Type == BASE64_TYPE_URL)
    {
        pszB64StrDe = BASE64_TYPE_URL_DE;
    }
    else
    {
        return -1;
    }

    const unsigned char *pucDataIn = (unsigned char *)pvDataBuff;
    int iDataInRemainLen = iDataLen;
    unsigned char *pucDataOut = (unsigned char *)pvDecodeDataBuff;
    int iDataOutLen = 0;

    while(iDataInRemainLen > 0)
    {
        if (iDataInRemainLen >= 4)
        {
            if ((pszB64StrDe[pucDataIn[0]] < 0) || (pszB64StrDe[pucDataIn[1]] < 0) || (pszB64StrDe[pucDataIn[2]] < 0) || (pszB64StrDe[pucDataIn[3]] < 0))
            {
                return -3;
            }

            if ((*iDecodeDataLen - iDataOutLen) < 3)
            {
                return -2;
            }

            pucDataOut[0] = (pszB64StrDe[pucDataIn[0]] << 2) + (pszB64StrDe[pucDataIn[1]] >> 4);
            pucDataOut[1] = ((pszB64StrDe[pucDataIn[1]] & 0xF) << 4) + (pszB64StrDe[pucDataIn[2]] >> 2);
            pucDataOut[2] = ((pszB64StrDe[pucDataIn[2]] & 0x3) << 6) + (pszB64StrDe[pucDataIn[3]]);

            pucDataOut += 3;
            iDataOutLen += 3;

            pucDataIn += 4;
            iDataInRemainLen -= 4;
        }
        else if (iDataInRemainLen == 3)
        {
            if ((pszB64StrDe[pucDataIn[0]] < 0) || (pszB64StrDe[pucDataIn[1]] < 0) || (pszB64StrDe[pucDataIn[2]] < 0))
            {
                return -3;
            }

            if ((*iDecodeDataLen - iDataOutLen) < 2)
            {
                return -2;
            }

            pucDataOut[0] = (pszB64StrDe[pucDataIn[0]] << 2) + (pszB64StrDe[pucDataIn[1]] >> 4);
            pucDataOut[1] = ((pszB64StrDe[pucDataIn[1]] & 0xF) << 4) + (pszB64StrDe[pucDataIn[2]] >> 2);

            pucDataOut += 2;
            iDataOutLen += 2;

            pucDataIn += 3;
            iDataInRemainLen -= 3;
        }
        else if (iDataInRemainLen == 2)
        {
            if ((pszB64StrDe[pucDataIn[0]] < 0) || (pszB64StrDe[pucDataIn[1]] < 0))
            {
                return -3;
            }

            if ((*iDecodeDataLen - iDataOutLen) < 1)
            {
                return -2;
            }

            pucDataOut[0] = (pszB64StrDe[pucDataIn[0]] << 2) + (pszB64StrDe[pucDataIn[1]] >> 4);

            pucDataOut += 1;
            iDataOutLen += 1;

            pucDataIn += 2;
            iDataInRemainLen -= 2;
        }
        else if (iDataInRemainLen == 1)
        {
            return -4;
        }
    }

    *iDecodeDataLen = iDataOutLen;

    return 0;
}


int CStrTool::CodeConvert(const char* from_charset,const char* to_charset, const char* inbuf, size_t inlen, char* outbuf, size_t& outbyteslef)
{
    char** pin = const_cast<char**>(&inbuf);
    char** pout = &outbuf;
    iconv_t cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return -1;
    memset(outbuf, 0, outbyteslef);
    int ret = 0;
    while (true)
    {
        //printf("before, ret=%d, pin=%x, in=%s, inlen=%d, pout=%x, outlen=%d\n", ret, *pin, Str2Hex(*pin, inlen), inlen, *pout, outbyteslef);
        ret = iconv(cd, pin, &inlen, pout, &outbyteslef);
        //printf("after, ret=%d, pin=%x, inlen=%d, pout=%x, outlen=%d, out=%s\n", ret, *pin, inlen, *pout, outbyteslef, outbuf);
        if (ret==0 || inlen == 0 || outbyteslef == 0)
        {
            break;
        }
        else
        {
            (*pin)++;
            inlen--;
            (*pout)[0]=' ';//如果转换失败，使用空格填充
            (*pout)++;
            outbyteslef--;
        }
    }
    iconv_close(cd);
    return 0;

}

int CStrTool::IsUTF8(const char *pszBuf, size_t uBuflen)
{
    static char szBuf[10240] = {0};
    char *pIn = const_cast<char *>(pszBuf);
    char *pOut = szBuf;
    size_t uOutBufLen = sizeof(szBuf);

    iconv_t cd = iconv_open("utf-8", "utf-8");
    if (cd ==  (iconv_t)(-1))
    {
        return 0;
    }

    int nRet = 0;

    nRet = iconv(cd, &pIn, &uBuflen, &pOut, &uOutBufLen);
    if (nRet == -1)
    {
        iconv_close(cd);
        return 0;
    }

    iconv_close(cd);
    return 1;
}

/*
UTF-8 valid format list:
0xxxxxxx
110xxxxx 10xxxxxx
1110xxxx 10xxxxxx 10xxxxxx
11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx


json中的特殊字符，这些需要过滤或者转换
 \" ， \\， \/， \b， \f， \n， \r， \t， \u

*/
string CStrTool::FilterUtf8ForJson(const string &strIn)
{
    int nStrLen     = strIn.length();
    int i            = 0;
    int nBytes      = 0;
    string strOut     = "";
    unsigned char c    = 0;

    while (i < nStrLen)
    {
        c = strIn[i];

        if (c < 0x80)
        {
            // 如果是控制字符，转换成空格
            // 如果是引号或斜线，则增加转义符
            switch (c)
            {
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
                strOut += ' ';
                break;
            case '\"':
            case '\\':
                strOut += '\\';
                //break;    // 这里不用break，需要继续后面的加上这个字符
            default:
                strOut += c;
                break;
            }

            ++i;
            continue;
        }

        if ((c & 0xE0) == 0xC0)  //110xxxxx
        {
            nBytes = 1;
        }
        else if ((c & 0xF0) == 0xE0) //1110xxxx
        {
            nBytes = 2;
        }
        else if ((c & 0xF8) == 0xF0) //11110xxx
        {
            nBytes = 3;
        }
        else if ((c & 0xFC) == 0xF8) //111110xx
        {
            nBytes = 4;
        }
        else if ((c & 0xFE) == 0xFC) //1111110x
        {
            nBytes = 5;
        }
        else
        {
            // 非法utf8字符过滤掉
            ++i;
            continue;
        }

        // 判断长度是否足够，如果不够则是非法utf8字符过滤掉
        if (nBytes + i + 1 > nStrLen)
        {
            ++i;
            continue;
        }

        // 判断nBytes字符是否符合规范，不符合则过滤掉
        bool bValid = true;
        for (int j = 1; j <= nBytes; ++j)
        {
            c = strIn[i+j];
            if ((c & 0xC0) != 0x80)
            {
                bValid = false;
                break;
            }
        }

        if (!bValid)
        {
            ++i;
            continue;
        }

        strOut.append(strIn, i, 1+nBytes);
        i += (nBytes+1);
    }


    return strOut;
}


int CStrTool::U2G(const char* inbuf,size_t inlen, char* outbuf, size_t& outlen)
{
    return CodeConvert("utf-8","gbk",inbuf,inlen,outbuf,outlen);
}
int CStrTool::G2U(const char* inbuf,size_t inlen, char* outbuf, size_t& outlen)
{
    return CodeConvert("gbk","utf-8",inbuf,inlen,outbuf,outlen);
}

int CStrTool::U2G(string &p_rInOut)
{
    char szTmp[1024] = {0}; //TODO: 长度要有弹性才好
    size_t TmpLen = sizeof(szTmp) - 1;

    int iRet = U2G(p_rInOut.c_str(), p_rInOut.size(), szTmp, TmpLen);
    p_rInOut.assign(szTmp);

    return iRet;
}

int CStrTool::G2U(string &p_rInOut)
{
    char szTmp[1024] = {0};
    size_t TmpLen = sizeof(szTmp) - 1;

    int iRet = G2U(p_rInOut.c_str(), p_rInOut.size(), szTmp, TmpLen);
    p_rInOut.assign(szTmp);

    return iRet;
}

unsigned long long CStrTool::SizeStrToNum(const char *pszSize)
{
    unsigned long long RetSize = strtoull(pszSize, NULL, 10);
    char SizeUnit = pszSize[strlen(pszSize)-1];
    switch(SizeUnit)
    {
        case 'K':
        case 'k':
        {
            RetSize *= 1024;
            break;
        }
        case 'M':
        case 'm':
        {
            RetSize = RetSize * 1024 * 1024;
            break;
        }
        case 'G':
        case 'g':
        {
            RetSize = RetSize * 1024 * 1024 * 1024;
            break;
        }
    }

    return RetSize;
}

int CSysTool::LockProc(const char* pszModuleName)
{
    int iRetVal = 0;

    if ((NULL == pszModuleName)||(pszModuleName[0] == '\0'))
    {
        return E_SYS_TOOL_MODULE_NAME;
    }

    char PET_PID_PATH[1024] = "/tmp/pid";

    int iPidFD;
    char szPidFile[1024] = {0};
    snprintf(szPidFile, sizeof(szPidFile), "%s.%s.pid", PET_PID_PATH, pszModuleName);

    if ((iPidFD = open(szPidFile, O_RDWR | O_CREAT, 0644)) == -1)
    {
        return E_SYS_TOOL_OPEN_FILE;
    }

    struct flock stLock;
    stLock.l_type = F_WRLCK;
    stLock.l_whence = SEEK_SET;
    stLock.l_start = 0;
    stLock.l_len = 0;

    iRetVal = fcntl(iPidFD, F_SETLK, &stLock);
    if (iRetVal == -1)
    {
        return E_SYS_TOOL_LOCK_FILE;
    }

    char szLine[16] = {0};
    snprintf(szLine, sizeof(szLine), "%d\n", getpid());
    ftruncate(iPidFD, 0);
    write(iPidFD, szLine, strlen(szLine));

    return SUCCESS;
}

int CSysTool::DaemonInit()
{
    pid_t pid = fork();

    if (pid == -1)
    {
        return E_SYS_TOOL_FORK;
    }
    else if (pid != 0)
    {
        // Parent exits.
        exit(0);
    }

    // 1st child continues.
    setsid(); // Become session leader.
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid != 0)
    {
        exit(0); // First child terminates.

    }

    // clear our file mode creation mask.
    umask(0);

    return SUCCESS;
}

const char *CSysTool::GetNicAddr(const char *pszIfName)
{
    const int MAX_NIC_NUM = 32;

    static char szNicAddr[20] = { 0 };

    if (pszIfName == NULL)
    {
        return NULL;
    }

    memset(szNicAddr, 0x0, sizeof(szNicAddr));

    int iSocket, iIfNum;

    struct ifreq stIfReqBuff[MAX_NIC_NUM];
    struct ifconf stIfConf;

    if ((iSocket = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        stIfConf.ifc_len = sizeof stIfReqBuff;
        stIfConf.ifc_buf = (caddr_t) stIfReqBuff;
        if (!ioctl(iSocket, SIOCGIFCONF, (char *) &stIfConf))
        {
            iIfNum = stIfConf.ifc_len / sizeof(struct ifreq);

            if ((iIfNum > 0)&&(iIfNum < MAX_NIC_NUM))
            {
                //网卡数量正确
                for (int i=0; i<iIfNum; i++)
                {
                    if (strcmp(stIfReqBuff[i].ifr_name, pszIfName) == 0)
                    {
                        //网卡名称正确
                        if (!(ioctl(iSocket, SIOCGIFADDR, (char *) &stIfReqBuff[i])))
                        {
                            snprintf(szNicAddr, sizeof(szNicAddr), "%s", inet_ntoa(((struct sockaddr_in *) (&stIfReqBuff[i].ifr_addr))->sin_addr));

                            //已经获取到IP地址
                            close(iSocket);
                            return szNicAddr;
                        }
                    }
                }
            }
        }
    }

    close(iSocket);
    return NULL;
}

const char *CSysTool::GetExecutableName(const char *path)
{
    std::vector<std::string> sections;
    CStrTool::SplitStringUsing(path, "/", &sections);
    return sections.back().c_str();
}

long long CSysTool::GetFileSize(const char * pszFileName)
{
    int iRetVal = 0;
    struct stat stFileStat;

    int iFD = open(pszFileName, O_RDONLY);
    if (iFD < 0)
    {
        return -1;
    }
    else
    {
        iRetVal = fstat(iFD, &stFileStat);
        if (iRetVal != 0)
        {
            close(iFD);
            return -1;
        }
        else
        {
            close(iFD);
            return stFileStat.st_size;
        }
    }
}

long long CSysTool::GetFileSize(int iFD)
{
    int iRetVal = 0;
    struct stat stFileStat;

    if (iFD < 0)
    {
        return -1;
    }
    else
    {
        iRetVal = fstat(iFD, &stFileStat);
        if (iRetVal != 0)
        {
            return -1;
        }
        else
        {
            return stFileStat.st_size;
        }
    }
}

time_t CSysTool::GetFileMTime(const char * pszFileName)
{
    int iRetVal = 0;
    struct stat stFileStat;

    iRetVal = stat(pszFileName, &stFileStat);
    if (iRetVal != 0)
    {
        return -1;
    }

    return stFileStat.st_mtim.tv_sec;
}

bool CSysTool::IsSameDay(time_t t1, time_t t2)
{
    struct tm st_tm_t1;
    struct tm st_tm_t2;
    localtime_r(&t1, &st_tm_t1);
    localtime_r(&t2, &st_tm_t2);
    return st_tm_t1.tm_yday == st_tm_t2.tm_yday && st_tm_t1.tm_year == st_tm_t2.tm_year;
}

int CSysTool::diff_days(time_t t1, time_t t2)
{
    //首先搞出本地时间和格林威治时间的差距，北京时间比格林威治时间时间早八个小时
    const time_t day_sec = 24*60*60;
    struct tm st_tm_local;
    struct tm st_tm_gm;
    time_t tm_zero = 0;
    localtime_r(&tm_zero, &st_tm_local);
    gmtime_r(&tm_zero, &st_tm_gm);
    time_t diff_local_gm = (st_tm_local.tm_hour - st_tm_gm.tm_hour)*60*60+
                           (st_tm_local.tm_min  - st_tm_gm.tm_min )*60+
                           (st_tm_local.tm_sec  - st_tm_gm.tm_sec );

    time_t start(0), end(0);
    if(t1 > t2)
    {
        start = t2;
        end = t1;
    }
    else
    {
        start = t1;
        end = t2;
    }
    return ((end+diff_local_gm)/day_sec - (start+diff_local_gm)/day_sec);
}

bool CSysTool::IsSameWeek(time_t t1, time_t t2)
{
    int dfdays = diff_days(t1, t2);
    if(dfdays >= 7)
    {
        return false;
    }
    struct tm st_tm_t1;
    struct tm st_tm_t2;
    localtime_r(&t1, &st_tm_t1);
    localtime_r(&t2, &st_tm_t2);
    if(st_tm_t1.tm_wday==0)
        st_tm_t1.tm_wday = 7;
    if(st_tm_t2.tm_wday==0)
        st_tm_t2.tm_wday = 7;
    if(t1 > t2)
    {
        if(st_tm_t1.tm_wday >= st_tm_t2.tm_wday)
            return true;
    }
    else
    {
        if(st_tm_t2.tm_wday >= st_tm_t1.tm_wday)
            return true;
    }
    return false;
}

time_t CSysTool::NextWeek(time_t timestamp)
{
        time_t tOld = timestamp;
        struct tm tmOld;
        localtime_r(&tOld, &tmOld);
        tmOld.tm_sec = 0;
        tmOld.tm_min = 0;
        tmOld.tm_hour = 0;
        // 这里是精确的以周一作为一周开始的下一周判断
        return mktime(&tmOld) + (((7 - tmOld.tm_wday) % 7) + 1) * (60*60*24);
}


int CRandomTool::Get(int iMin, int iMax)
{
    return (iMin + (int)((iMax - iMin) * (rand() / (RAND_MAX + 1.0))));
}

long CRandomTool::GetLong(long lMin, long lMax)
{
    return (lMin + (long)((lMax - lMin) * ((double)(rand() * rand()) / ((long)RAND_MAX * RAND_MAX + 1.0))));
}

CRandomTool::CRandomTool()
{
    srand(time(NULL) % getpid());
}



unsigned  int   g_random_seedp_01 = 0;

//add by kevin
int CRand::GetRand(int iMin, int iMax)
{
    srand(time(NULL) + getpid());
    return (iMin + (int)((iMax - iMin) * (rand() / (RAND_MAX + 1.0))));
}

//add by kevin
//传入各奖项的概率和共有的奖励等级,返回根据概率得到的奖励等级
//这块是否真的随机了。还真是不敢说yes!!
int CRand::RandNumPrizeLevel(int iarrPosibilities[],  int iCount )
{
    //根据概率算出序列


    for(int i=1; i< iCount; i++)    //从第2个开始, 其值为前一个加本身.如{20,30,50}->{20,50,100}
    {
        iarrPosibilities[i] += iarrPosibilities[i-1];
        //cout<<"iarrPosibilities["<<i<<"] = "<<iarrPosibilities[i]<<"\t";
    }
    const int r = Random(1, iarrPosibilities[iCount-1]);
    //cout<<" r=" << r <<" "<<endl;

    const int ret = lower_bound(iarrPosibilities,iarrPosibilities+iCount-1, r)  - iarrPosibilities;
    //cout<<"ret = "<<ret<<endl;
    return ret;
}


int CRand::Random(int iMax )
{
    struct timeval tpstime;
    float fLocalMax = iMax;

    if (iMax <= 0)  //这里应该不可能出现问题，所以返回0
    {
        return 0;
    }

    if(0 == g_random_seedp_01)
    {
        gettimeofday(&tpstime,NULL);
        g_random_seedp_01 =(unsigned)(tpstime.tv_usec+getpid());

    }

    int iRandomResult =  (int) (fLocalMax * (rand_r(&g_random_seedp_01)/(RAND_MAX + 1.0)));
    return iRandomResult;
}


int CRand::Random(int iMin, int iMax )
{
    struct timeval tpstime;

    if (iMax <= 0)  //这里应该不可能出现问题，所以返回0
    {
        return 0;
    }

    if(0 == g_random_seedp_01)
    {
        gettimeofday(&tpstime,NULL);
        g_random_seedp_01 =(unsigned)(tpstime.tv_usec+getpid());

    }

    int iRandomResult = iMin+ (int) ((iMax-iMin) * (rand_r(&g_random_seedp_01)/(RAND_MAX + 1.0)));
    return iRandomResult;
}

string CHtmlUtil::simpleHtmlEncode(string src)
{
    ostringstream oss;

    char c;
    unsigned int len = src.length();
    for(unsigned int i=0; i < len; ++i)
    {
        c = src[i];
        if (c == '<')
        {
            oss << "&lt;";
        }
        else if (c == '>')
        {
              oss << "&gt;";
        }
        else if (c == '&')
        {
            oss << "&amp;";
        }
        else if (c == '\"')
        {
            oss << "&quot;";
        }
        else if(c == '\'')
        {
            oss << "&#39;";
        }
        else if (c == ' ')
        {
            oss << "&nbsp;";
        }
        else
        {
            oss << c;
        }

    }

    return oss.str();
}

void CHtmlUtil::cutGBRaw( char *szInfo, unsigned int uiMaxLen )
{
    szInfo[uiMaxLen-1] = 0;

    for( unsigned int i = 0;  szInfo[i]!=0; i++ )
    {
        //if( szInfo[i] < 0 )
        if((unsigned char) szInfo[i] > 126) //这样修改，就能把127这个字符扼杀了
        {
            if( (unsigned char)szInfo[i] < 0xA1 || (unsigned char)szInfo[i] > 0xF7 )
            {
                //不是GB码用空格代替
                szInfo[i] = ' ';
                if(szInfo[++i] == 0)
                {
                    break;
                }
                szInfo[i] = ' ';
            }
            else
            {
                if(szInfo[++i] == 0) //是最后1个字符，半中文去掉
                {
                    szInfo[i-1] = 0;
                    break;
                }
            }
        }
    }

}

string CHtmlUtil::cutGB(string str)
{
    unsigned int uiMaxLen = str.length()+1;
    char* buff = new char[uiMaxLen];
    strncpy(buff, str.c_str(), uiMaxLen);
    cutGBRaw(buff, uiMaxLen);
    string ret = buff;
    delete[] buff;
    return ret;
}

unsigned int CTimeTools::DayFrom2010(time_t tTheDay)
{
    tm BASE_TIME_T; //2010-1-1 00:00:00
    memset((char*)&BASE_TIME_T, 0, sizeof(BASE_TIME_T));
    BASE_TIME_T.tm_sec = 0;
    BASE_TIME_T.tm_min = 0;
    BASE_TIME_T.tm_hour = 0;
    BASE_TIME_T.tm_mday = 1;
    BASE_TIME_T.tm_mon = 1-1; //0-11
    BASE_TIME_T.tm_year = 2010 -1900;
    BASE_TIME_T.tm_isdst = 0; //不要夏令时
    time_t tBase = mktime(&BASE_TIME_T);
    return (tTheDay>tBase)?(tTheDay-tBase)/86400:0;
}

unsigned short CTimeTools::UniqDay(time_t t)
{
    struct tm tm;
    localtime_r(&t,&tm);

    return (tm.tm_year-110)*1000 + tm.tm_yday;
}


//add by kevin at 2010-01-28
//函数说明: 判断某个时间是否在两个时间点之间
//函数返回: 0不在活动时间内  <0 错误  >0在活动时间内
//函数参数: 前两个参数表示开始和结束时间，最后一个参数代表需要比较的时间,默认为0指当前时间
//          szActBegin和szActEnd的形式如此: %Y-%m-%d %H:%M:%S 如2010-01-28 11:24:00
int  CTimeTools::BetweenTime(const char*  szActBegin, const char* szActEnd, time_t  timeNow /*= 0*/)
{
    if(szActBegin == NULL || szActEnd == NULL)
    {
        return -1;
    }

    struct tm  act_time;
    time_t  now =  (timeNow == 0 ? time(NULL) : timeNow );

    time_t   act_beg, act_end;

    if(NULL == strptime(szActBegin,"%Y-%m-%d %H:%M:%S",&act_time))
    {
        return -2;
    }
    act_beg = mktime(&act_time);


    if(NULL == strptime(szActEnd,"%Y-%m-%d %H:%M:%S",&act_time))
    {
        return -3;
    }
    act_end = mktime(&act_time);


    //查看是否在活动时间内
    if(now>act_beg  && now < act_end)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//add by marszhang at 2011-02-23
//判断日期间的天数差
//szFrom和szTo%Y-%m-%d 如2010-01-28
//szTo 可以默认为当前时间
int CTimeTools::DayDiff(int* pdaydiff,const char* szTheDay, time_t  timeNow /*= 0*/)
{
    struct tm now,theday;
    time_t t_tmp,t_now,t_theday;
    memset(&now, 0 ,sizeof(now));
    memset(&theday, 0 ,sizeof(theday));
    if(strptime(szTheDay, "%Y-%m-%d", &theday) == NULL)
    {
        return -1;
    }
    t_theday = mktime(&theday);

    if(timeNow == 0)
    {
        t_tmp = time(NULL);
    }
    else
    {
        t_tmp = timeNow;
    }

    //对齐时间
    localtime_r(&t_tmp, &now);
    now.tm_sec = 0;
    now.tm_min = 0;
    now.tm_hour = 0;
    t_now = mktime(&now);


    if(t_theday >= t_now)
    {
        *pdaydiff = (t_theday - t_now)/(24*3600);
    }
    else
    {
        *pdaydiff = 0-(t_now - t_theday)/(24*3600);
    }

    return 0;
}



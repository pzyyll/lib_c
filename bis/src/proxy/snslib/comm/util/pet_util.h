/**
 * @file    pet_util.h
 * @brief   一些封装好的使用工具函数
 */

#ifndef _PET_UTIL_H_
#define _PET_UTIL_H_

#include <stdarg.h>
#include <string>
#include <vector>
#include <inttypes.h>
#include <sys/time.h>

namespace snslib
{

class CBuffTool
{
public:

    static int ReadByte(const void *pvBuffer, unsigned char &ucVal);
    static int ReadByte(const void *pvBuffer, char &cVal);
    static int WriteByte(void *pvBuffer, unsigned char ucVal);
    static int WriteByte(void *pvBuffer, char cVal);

    static int ReadShort(const void *pvBuffer, unsigned short &ushVal, int iToHostOrder = 1);
    static int ReadShort(const void *pvBuffer, short &shVal, int iToHostOrder = 1);
    static int WriteShort(void *pvBuffer, unsigned short ushVal, int iToNetOrder = 1);
    static int WriteShort(void *pvBuffer, short shVal, int iToNetOrder = 1);

    static int ReadInt(const void *pvBuffer, unsigned int &uiVal, int iToHostOrder = 1);
    static int ReadInt(const void *pvBuffer, int &iVal, int iToHostOrder = 1);
    static int WriteInt(void *pvBuffer, unsigned int uiVal, int iToNetOrder = 1);
    static int WriteInt(void *pvBuffer, int iVal, int iToNetOrder = 1);

    static int ReadLong(const void *pvBuffer, unsigned long &ulVal, int iToHostOrder = 1);
    static int ReadLong(const void *pvBuffer, long &lVal, int iToHostOrder = 1);
    static int WriteLong(void *pvBuffer, unsigned long ulVal, int iToNetOrder = 1);
    static int WriteLong(void *pvBuffer, long lVal, int iToNetOrder = 1);

    static int ReadLongLong(const void *pvBuffer, unsigned long long &ullVal, int iToHostOrder = 0);
    static int ReadLongLong(const void *pvBuffer, long long &llVal, int iToHostOrder = 0);
    static int WriteLongLong(void *pvBuffer, unsigned long long ullVal, int iToNetOrder = 0);
    static int WriteLongLong(void *pvBuffer, long long llVal, int iToNetOrder = 0);

#if __WORDSIZE == 64
    static int ReadLongLong(const void *pvBuffer, uint64_t &llVal, int iToHostOrder = 0);
    static int WriteLongLong(void *pvBuffer, uint64_t llVal, int iToNetOrder = 0);
#endif
    static int ReadString(const void *pvBuffer, char *pszVal, int iStrLen);
    static int WriteString(void *pvBuffer, const char *pszVal, int iStrLen);

    static int ReadBuf(const void *pvBuffer, void *pszVal, int iStrLen);
    static int WriteBuf(void *pvBuffer, const void *pszVal, int iStrLen);
};

class CFileUtil
{
public:
  static bool ReadFile(const std::string &path, std::string &data);
  static bool ReadFile(const std::string &path, char *ptr, size_t &len);
  static bool FileExists(const std::string &path);
};

class CStrTool
{
    static void InternalAppend(std::string& sDst, const char* pszFmt, va_list stApList);
public:
    static const char *Str2Hex(const void *pBuff, int iSize);
	// By michaelzhao 2010-08-23 为什么有Str2Hex就没有Hex2Str
	static const char* Hex2Str(const char *pBuff, int iSize, int *pSize);
    static unsigned short CheckSum(const void *pvBuff, int iSize);

    // Added by Jiffychen@2010-03-30
    // Replace any characters in pszRemove with chReplaceWith
    static void StripString(std::string* psStr, const char* pszRemove, char chReplaceWith);

    // Replace any occurance of sOldSub in sStr by sNewSub,
    // The result is appended to psRes, if bReplaceAll is false, only the
    // first occurance will be replaced
    static void StringReplace(const std::string& sStr, const std::string& sOldSub,
        const std::string& sNewSub, bool bReplaceAll, std::string* psRes);

    // See above, the result is returned
    static std::string StringReplace(const std::string& sStr, const std::string& sOldSub,
        const std::string& sNewSub, bool bReplaceAll);

    // Split sFull by any character in pszDelim
    static void SplitStringUsing(const std::string& sFull,
        const char* pszDelim, std::vector<std::string>* pvsResult);

    static void SplitStringUsing(const std::string& sFull,
        const char* pszDelim, std::vector<std::string>* pvsResult, int iMaxSplitTimes);

    // Join the items with pszDelim as a separator
    static void JoinStrings(const std::vector<std::string>& vsItems, const char* pszDelim,
        std::string * psResult);

    static std::string JoinStrings(const std::vector<std::string>& vsItems,
        const char* pszDelim);

    // Convert the given int to its string representation
    static std::string SimpleItoa(int i);

    // Format to string
    static std::string Format(const char* pszFmt, ...);

    // Append mode
    static void Append(std::string& sDst, const char* pszFmt, ...);

    // time_t to str
    static const char *TimeString(time_t tTime);

    // str to time_t
    static time_t FromTimeString(const char *pszTimeStr);

    static const int BASE64_TYPE_NORMAL = 0;
    static const int BASE64_TYPE_URL = 1;
    static int Base64Encode(const void *pvDataBuff, int iDataLen, void *pvEncodeDataBuff, int *iEncodeDataLen, int iBase64Type = BASE64_TYPE_NORMAL);
    static int Base64Decode(const void *pvDataBuff, int iDataLen, void *pvDecodeDataBuff, int *iDecodeDataLen, int iBase64Type = BASE64_TYPE_NORMAL);
    //以下为编码转换函数。如可以将utf8转为gbk或者其它互转。
    static int CodeConvert(const char* from_charset,const char* to_charset, const char* inbuf, size_t inlen, char* outbuf, size_t& outbyteslef);
    static int U2G(const char* inbuf,size_t inlen, char* outbuf, size_t& outlen);
    static int G2U(const char* inbuf,size_t inlen, char* outbuf, size_t& outlen);

    static int U2G(std::string &p_rInOut);
    static int G2U(std::string &p_rInOut);

	static bool CheckValidNameGBK(const char *pszInStr, char *pszOutStr);

    // timeval to str
    static const char *TimeString(struct ::timeval stTime);

    //ip addr to str
    static const char *IPString(unsigned int uiIP);
    static unsigned long long SizeStrToNum(const char *pszSize);
    static int IsUTF8(const char *pszBuf, size_t uBuflen);
    /* 过滤utf8编码字符串中的非法字符和含有引起解析json失败的字符，比如\" ， \\， \/， \b， \f， \n， \r， \t， \u
     * 请确保strIn是utf8编码，返回符合utf8规范的字符串
     */
    static std::string FilterUtf8ForJson(const std::string &strIn);
};

class CSysTool
{
public:
    const static int SUCCESS = 0;
    const static int ERROR = -1;

    const static int E_SYS_TOOL_MODULE_NAME = -101;
    const static int E_SYS_TOOL_OPEN_FILE = -102;
    const static int E_SYS_TOOL_LOCK_FILE = -103;
    const static int E_SYS_TOOL_FORK = -104;

    /*
     * @brief 检测是否存在相同进程
     * @param:pszModuleName 模块名
     *
     * @note 该函数调用会创建pid文件（/usr/local/pet50/pid/<ModuleName>.pid），并且锁定该文件
     */
    static int LockProc(const char* pszModuleName);

    static int DaemonInit();

    static const char *GetNicAddr(const char *pszIfName);
	
	static const char *GetExecutableName(const char *path);


    static long long GetFileSize(const char * pszFileName);

    static long long GetFileSize(int iFD);

    static time_t GetFileMTime(const char * pszFileName);

    static bool IsSameDay(time_t t1, time_t t2);

    static int diff_days(time_t t1, time_t t2);

    static bool IsSameWeek(time_t t1, time_t t2);

    /* 得到下一个星期一的时间戳 */
    static time_t NextWeek(time_t timestamp);
};

class CRandomTool
{
public:
    static CRandomTool * Instance()
    {
        static CRandomTool * p = new CRandomTool;

        return p;
    }

    //return val [iMin iMax)
    int Get(int iMin, int iMax);

    long GetLong(long lMin, long lMax);

protected:
    CRandomTool();
};

/*
 * class CSingleton
 */
template <typename T>
class CSingleton
{
public:
    static T & Instance()
    {
        static T instance;
        return instance;
    }

protected:
    CSingleton() {}
};


/**
 * add by kevin
 *
 *这个类主要是用于常用的一些随机数的生成或者随机概率的产生，
 *如果你有更好或也很常用的方法在这里没有，欢迎讨论并加入进来
 */

class CRand
{
public:

        //产生0-iMax之间的随机整数  [0,iMax)
    static int Random(int iMax );

    //根据iarrPosibilities中的概率列表，返回中第几项奖。[0, iCount-1]
    static int RandNumPrizeLevel(int iarrPosibilities[],  int iCount );

    //这个函数是以time(NULL)+pid为种子的，如果要同时产生很多的随机数，切记不要使用它。请使用Random()
    static int GetRand(int iMin, int iMax);

    static int  Random(int iMin, int iMax );

};

// copy from petlib by shimmeryang
class CHtmlUtil
{
    //编码过的字符串可以放到js变量的字符串中，并显示在页面上
public:
    static std::string simpleHtmlEncode(std::string src);

    /**
    GBK编码范围(ch1为首字符，ch2为第二个字符):
    0x81<=ch1<=0xFE && (0x40<=ch2<=0x7E || 0x7E<=ch2<=0xFE)

    GB2312
    0xA1<=ch1<=0xF7 && (0xA1<=ch2<=0xFE)
    */
    static void cutGBRaw( char *szInfo, unsigned int uiMaxLen );

    static std::string cutGB(std::string str);

};

class  CTimeTools
{
public:
    //从2010到现在经过的天数
    static unsigned int DayFrom2010(time_t tTheDay);

    //根据time_t获得一个唯一的天数
    static unsigned short UniqDay(time_t t);


    //函数说明: 判断某个时间是否在两个时间点之间
    //函数返回: 0不在活动时间内  <0 错误  >0在活动时间内
    //函数参数: 前两个参数表示开始和结束时间，最后一个参数代表需要比较的时间,默认为0指当前时间
    //          szActBegin和szActEnd的形式如此: %Y-%m-%d %H:%M:%S 如2010-01-28 11:24:00
    static int   BetweenTime(const char*  szActBegin, const char* szActEnd, time_t  timeNow = 0);

    //szTheDay 是精确到天的字符串表示 %Y-%m-%d 如 "2011-02-24"
    //timeNow默认是当前时间time(NULL),也可以输入
    //*pdaydiff 返回当前时间过几天到目标时间
    //比如szTheDay="2011-02-24"
    //如果现在是 2011-02-23的任意时刻， 那么返回1
    //如果现在是 2011-02-24的任意时刻， 那么返回0
    //如果现在是 2011-02-25的任意时刻， 那么返回-1
    //返回值 0=ok， -1=szTheDay时间格式有误
    static int DayDiff(int* pdaydiff,const char* szTheDay, time_t  timeNow = 0);

};

}

#endif


/**
 * @file    hash_db.h
 * @brief   对tdb进行封装的接口类
 * @author  jamieli@tencent.com
 * @date    2010-03-30
 */

#ifndef _HASH_DB_H_
#define _HASH_DB_H_

#include "comm/hash_db/tchdb.h"

namespace snslib
{
class CHashDB
{
public:
    const static int SUCCESS = 0;
    const static int ERROR = -1;

    const static int HDBE_NORECORD = 101;
    const static int HDBE_DBFULL = 102;

public:

    /**
     @brief 构造函数
     */
    CHashDB();

    /**
     @brief 析构函数
     */
    ~CHashDB();

    /**
     * brief 初始化
     * param pszConfFile 配置文件名
     */
    int Init(const char *pszConfFile, const char * pszSection = "HASH_DB" );

    /**
     * brief 增加/覆盖记录
     * param pvKeyBuff 存放key的buff指针
     * param iKeySize key的长度
     * param pvValBuff 存放数值的buff指针
     * param iValSize val的长度
     *
     * note 调用该接口，如果已经存在了对应key的记录，将会被覆盖，如果没有对应key的记录，将会新增
     */
    int Put(const void *pvKeyBuff, int iKeySize, const void *pvValBuff, int iValSize);

    /**
     * brief 增加/覆盖记录
     * param pszKey 存放key的字符串，必须以\0结尾
     * param pszVal 存放数值的字符串，必须以\0结尾
     *
     * note 调用该接口，如果已经存在了对应key的记录，将会被覆盖，如果没有对应key的记录，将会新增
     */
    int Put2(const char *pszKey, const char *pszVal);

    /**
     * brief 增加记录
     * param pvKeyBuff 存放key的buff指针
     * param iKeySize key的长度
     * param pvValBuff 存放数值的buff指针
     * param iValSize val的长度
     *
     * note 调用该接口，如果已经存在了对应key的记录，将会提示出错，如果没有对应key的记录，将会新增
     */
    int PutKeep(const void *pvKeyBuff, int iKeySize, const void *pvValBuff, int iValSize);

    /**
     * brief 增加记录
     * param pszKey 存放key的字符串，必须以\0结尾
     * param pszVal 存放数值的字符串，必须以\0结尾
     *
     * note 调用该接口，如果已经存在了对应key的记录，将会提示出错，如果没有对应key的记录，将会新增
     */
    int PutKeep2(const char *pszKey, const char *pszVal);

    /**
     * brief 当前记录追加/新增
     * param pvKeyBuff 存放key的buff指针
     * param iKeySize key的长度
     * param pvValBuff 存放数值的buff指针
     * param iValSize val的长度
     *
     * note 调用该接口，如果已经存在了对应key的记录，将新的内容追加到原有内容后面，如果没有对应key的记录，将新增
     */
    int PutCat(const void *pvKeyBuff, int iKeySize, const void *pvValBuff, int iValSize);

    /**
     * brief 当前记录追加/新增
     * param pszKey 存放key的字符串，必须以\0结尾
     * param pszVal 存放数值的字符串，必须以\0结尾
     *
     * note 调用该接口，如果已经存在了对应key的记录，将新的内容追加到原有内容后面，如果没有对应key的记录，将新增
     */
    int PutCat2(const char *pszKey, const char *pszVal);

    /**
     * brief 删除记录
     * param pvKeyBuff 存放key的buff指针
     * param iKeySize key的长度
     *
     * note 调用该接口，删除指定key的记录，不管存在与否，都会返回成功
     */
    int Out(const void *pvKeyBuff, int iKeySize);

    /**
     * brief 删除记录
     * param pszKey 存放key的字符串，必须以\0结尾
     *
     * note 调用该接口，删除指定key的记录，不管存在与否，都会返回成功
     */
    int Out2(const char *pszKey);

    /**
     * brief 读取记录
     * param pvKeyBuff 存放key的buff指针
     * param iKeySize key的长度
     * param ppvValBuff 存放数值的buff指针的指针
     * param piValSize 内部开辟空间的大小
     *
     * note ppvValBuff指向的空间不需要外部开辟，由函数内部开辟，开辟的长度为piValSize，需要外部进行释放
     */
    int Get(const void *pvKeyBuff, int iKeySize, void **ppvValBuff, int *piValSize);

    /**
     * brief 读取记录
     * param pszKey 存放key的字符串，必须以\0结尾
     * param ppszVal 存放数值的buff指针的指针，以\0结尾
     *
     * note ppszVal指向的空间不需要外部开辟，由函数内部开辟，未返回开辟长度，需要外部进行释放
     */
    int Get2(const char *pszKey, char **ppszVal);

    /**
     * brief 读取记录
     * param pvKeyBuff 存放key的buff指针
     * param iKeySize key的长度
     * param pvValBuff 外部存放Val的Buff大小，存储空间由外部开辟
     * param piValBuffSize IN/OUT 传入时，作为Buff大小参数，传出时，作为Val的长度
     *
     * note pvValBuff必须由外部开辟，同时将Buff长度作为piValBuffSize参数传入，如果存储空间不够val存储，接口将返回错误
     */
    int Get3(const void *pvKeyBuff, int iKeySize, void *pvValBuff, int *piValBuffSize);

    /**
     * brief 获取某个记录VAL的长度
     * param pvKeyBuff 存放key的buff指针
     * param iKeySize key的长度
     * param piValSize OUT返回该记录的长度
     */
    int GetVSize(const void *pvKeyBuff, int iKeySize, int *piValSize);

    /**
     * brief 获取某个记录VAL的长度
     * param pszKey 存放key的字符串，必须以\0结尾
     * param piValSize OUT返回该记录的长度
     */
    int GetVSize2(const char *pszKey, int *piValSize);

    /**
     * brief 遍历时，将应用指针指向某个记录
     * param pvKeyBuff 需要将iterator指向的记录对应的key
     * param iKeySize 需要将iterator指向的记录对应key的长度
     *
     * note 如果iKeySize=0或者pvKeyBuff=NULL，iterator将指向DB中的第一个记录
     */
    int IterInit(const void *pvKeyBuff = NULL, int iKeySize = 0);

    /**
     * brief 遍历时，将应用指针指向某个记录
     * param pszKey 需要将iterator指向的记录对应的key，以\0结尾
     *
     * note 如果pszKey=NULL或者strlen(pszKey)=0，iterator将指向DB中的第一个记录
     */
    int IterInit2(const char *pszKey = NULL);

    /**
     * brief 遍历时，获取下一个Key值
     * param ppvKeyBuff 存放数值的key指针的指针
     * param piKeySize Key的长度
     *
     * note ppvKeyBuff对应的存储空间由内部开辟，外部释放
     */
    int IterNextKey(void **ppvKeyBuff, int *piKeySize);

    /**
     * brief 遍历时，获取下一个Key值
     * param ppszKeyBuff 存放数值的key指针的指针，以'\0'结尾
     *
     * note ppszKeyBuff对应的存储空间由内部开辟，外部释放
     */
    int IterNextKey2(char **ppszKeyBuff);

    /**
     * brief 遍历时，获取下一个Key值
     * param pvKeyBuff 存放数值的key指针
     * param piKeyBuffSize IN/OUT 传入Buff的大小，同时表示传出Key的长度
     *
     * note pvKeyBuff对应的存储空间由外部开辟
     */
    int IterNextKey3(void *pvKeyBuff, int *piKeyBuffSize);

    /**
     * brief 遍历时，获取下一个Key/Val值
     * param ppvKeyBuff 存放数值的key指针的指针
     * param piKeySize Key的长度
     * param ppvValBuff 存放数值的val指针的指针
     * param piValSize val的长度
     *
     * note ppvKeyBuff ppvValBuff都是由函数内部开辟，需要外部释放
     */
    int IterNext(void **ppvKeyBuff, int *piKeySize, void **ppvValBuff, int *piValSize);

    /**
     * brief 遍历时，获取下一个Key/Val值
     * param ppszKeyBuff 存放数值的key指针的指针
     * param ppszValBuff 存放数值的val指针的指针
     *
     * note ppszKeyBuff ppszValBuff都是由函数内部开辟，需要外部释放
     */
    int IterNext2(char **ppszKeyBuff, char **ppszValBuff);

    /**
     * brief 遍历时，获取下一个Key/Val值
     * param pvKeyBuff 存放数值的key指针
     * param piKeyBuffSize IN/OUT 传入Buff的长度，传出Key的长度
     * param pvValBuff 存放数值的val指针
     * param piValBuffSize IN/OUT 传入Buff的长度，传出Val的长度
     *
     * note pvKeyBuff pvValBuff由外部分配空间
     */
    int IterNext3(void *pvKeyBuff, int *piKeyBuffSize, void *pvValBuff, int *piValBuffSize);

    /**
     * brief 清除整个DB
     *
     * note 将会删除整个DB里面所有的记录
     */
    int VanishDB();

    unsigned long long GetFileSize();

    unsigned long long GetRecordNum();

    void ShowDBInfo();

    const char *GetErrMsg()
    {
        return m_szErrMsg;
    }

private:
    char m_szErrMsg[256];

    TCHDB *m_pstTCHDB;

};
} /* snslib */

#endif

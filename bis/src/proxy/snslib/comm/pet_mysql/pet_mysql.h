/**
 @file	pet_mysql.h
 @brief  对MySQL数据库C接口的一层非常薄的CPP封装
 @author	jamieli
 */

#ifndef	_MYSQL_H_
#define	_MYSQL_H_

#include <string>

namespace snslib
{

#include "mysql.h"
#include <stdlib.h>

typedef int ( ProcessBatchRecord)(MYSQL_ROW lRow);//<, char *lsBuffer, int nBuffSize)

class CPetMySQL
{

public:
    CPetMySQL();
    ~CPetMySQL();

    int Init(const char *szDBHost, const char *szDBUser, const char *szDBPasswd, const char *szDBName, int iPort = 3306, int read_timeout=-1, int write_timeout=-1);

    int Execute(const char *pszSQL, int iSQLLen = 0);

    int CheckConnection();

    int AffectedRows();

    int StoreResult();

    int NumRows();

    int NumFields();

    int FreeResult();

    int UseDB(const char *pszDBName);

    MYSQL_ROW FetchRecord();
    MYSQL_FIELD *FetchField();
    unsigned long * FetchLengths();

    int EscapeString(char *pszDstSQL, const char *pszSrcSQL, int iSQLLen);

    const char *GetErrMsg()
    {
        return m_szErrMsg;
    }

private:
    void SetErrMsg(const char *pszErrMsg = NULL);

private:
    char m_szErrMsg[256];

    MYSQL m_stMYSQL;
    MYSQL_RES *m_pstRES;

    //(GCS要求自己处理连接丢失的问题)
    std::string m_DBHost;
    std::string m_DBUserName;
    std::string m_DBPassWd;
    std::string m_DBName;
    int m_DBPort;
    bool m_IsInited;

    int m_ReadTimeout;
    int m_WriteTimeout;

    int Reconnect();
};

}
#endif

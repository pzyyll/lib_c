#include	<string.h>
#include	<stdio.h>

#include	"comm/pet_mysql/pet_mysql.h"

using namespace snslib;

#define MYSQL_ERR_CONNECT           2003  //Can't connect to MySQL server on
#define MYSQL_ERR_GONE_AWAY         2006  //MySQL server has gone away
#define MYSQL_ERR_LOST_DURING_QUERY 2013  //Lost connection to MySQL server during query
#define MYSQL_ERR_BACKEND_DOWN      1105  //Unknown error或  (proxy) all backends are down
#define MYSQL_RECONNECT_COUNT       3     //连接丢失时重连次数

CPetMySQL::CPetMySQL()
{
    memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
    m_pstRES = NULL;

    m_DBPort = -1;
    m_IsInited = false;

    m_ReadTimeout = -1;
    m_WriteTimeout = -1;
}

CPetMySQL::~CPetMySQL()
{
    if (m_pstRES)
    {
        mysql_free_result(m_pstRES);
        m_pstRES = NULL;
    }
}

int CPetMySQL::Init(const char *pszDBHost, const char *pszDBUser, const char *pszDBPasswd, const char *pszDBName, int iPort, int read_timeout/*=-1*/, int write_timeout/*=-1*/)
{
    if (mysql_init(&m_stMYSQL) == NULL)
    {
        SetErrMsg();
        return -1;
    }

    //char cReconnectFlag = 1;
    char cReconnectFlag = 0;  //自己处理连接的问题
    if (mysql_options(&m_stMYSQL, MYSQL_OPT_RECONNECT, (char *)&cReconnectFlag) != 0)
    {
        SetErrMsg();
        mysql_close(&m_stMYSQL);
        return -2;
    }

    m_ReadTimeout = read_timeout;
    m_WriteTimeout = write_timeout;
    if(m_ReadTimeout > 0)
    {
        if (mysql_options(&m_stMYSQL, MYSQL_OPT_READ_TIMEOUT, (char *)&m_ReadTimeout) != 0)
        {
            SetErrMsg();
            mysql_close(&m_stMYSQL);
            return -2;
        }
    }
    if(m_WriteTimeout > 0)
    {
        if (mysql_options(&m_stMYSQL, MYSQL_OPT_WRITE_TIMEOUT, (char *)&m_WriteTimeout) != 0)
        {
            SetErrMsg();
            mysql_close(&m_stMYSQL);
            return -2;
        }
    }

    int iConnectOption = CLIENT_FOUND_ROWS; //����UPDATE������AffectRows��ʾƥ�������
    if (mysql_real_connect(&m_stMYSQL, pszDBHost, pszDBUser, pszDBPasswd, pszDBName, iPort, NULL, iConnectOption) == NULL)
    {
        SetErrMsg();
        mysql_close(&m_stMYSQL);
        return -3;
    }

    m_DBHost = pszDBHost;
    m_DBUserName = pszDBUser;
    m_DBPassWd = pszDBPasswd;
    m_DBName = pszDBName;
    m_DBPort = iPort;
    m_IsInited = true;
    return 0;
}

void CPetMySQL::SetErrMsg(const char *pszErrMsg)
{
    if (pszErrMsg == NULL)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", mysql_error(&m_stMYSQL));
    }
    else
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", pszErrMsg);
    }
}

int CPetMySQL::Execute(const char *pszSQL, int iSQLLen /*= 0*/)
{
    if (CheckConnection() != 0)
    {
        SetErrMsg("mysql check connection failed");
        return -1;
    }

    if (iSQLLen == 0)
    {
        iSQLLen = strlen(pszSQL);
    }

    if ((pszSQL == NULL)||(iSQLLen <= 0))
    {
        SetErrMsg("sql is not valid");
        return -2;
    }

    if (mysql_real_query(&m_stMYSQL, pszSQL, iSQLLen) != 0)
    {
        SetErrMsg();
        return -3;
    }

    return 0;
}

int CPetMySQL::AffectedRows()
{
    int iRetVal = 0;
    iRetVal = mysql_affected_rows(&m_stMYSQL);
    if (iRetVal < 0)
    {
        SetErrMsg();
    }

    return iRetVal;
}

int CPetMySQL::StoreResult()
{
    m_pstRES = mysql_store_result(&m_stMYSQL);

    if(m_pstRES == NULL)
    {
        SetErrMsg();
        return -1;
    }

    return 0;
}

int CPetMySQL::FreeResult()
{

    mysql_free_result(m_pstRES);
    m_pstRES = NULL;

    return 0;
}

int CPetMySQL::UseDB(const char *pszDBName)
{
    int iRetVal = 0;

    iRetVal = mysql_select_db(&m_stMYSQL, pszDBName);
    if (iRetVal < 0)
    {
        SetErrMsg();
    }

    return iRetVal;
}

int CPetMySQL::NumRows()
{
    int iRetVal = 0;

    if (m_pstRES == NULL)
    {
        SetErrMsg("RES is not exist");
        return -1;
    }

    iRetVal = mysql_num_rows(m_pstRES);
    if (iRetVal < 0)
    {
        SetErrMsg();
    }

    return iRetVal;
}

int CPetMySQL::NumFields()
{
    int iRetVal = 0;

    if (m_pstRES == NULL)
    {
        SetErrMsg("RES is not exist");
        return -1;
    }

    iRetVal = mysql_num_fields(m_pstRES);
    if (iRetVal < 0)
    {
        SetErrMsg();
    }

    return iRetVal;
}

MYSQL_ROW CPetMySQL::FetchRecord()
{
    if (m_pstRES == NULL)
    {
        SetErrMsg("RES is not exist");
        return NULL;
    }

    MYSQL_ROW stROW;

    stROW = mysql_fetch_row(m_pstRES);
    if (stROW == NULL)
    {
        SetErrMsg();
    }

    return stROW;
}
unsigned long * CPetMySQL::FetchLengths()
{
    if (m_pstRES == NULL)
    {
        SetErrMsg("RES is not exist");
        return NULL;
    }

    unsigned long * plLengths = mysql_fetch_lengths(m_pstRES);
    if ( plLengths == NULL)
    {
        SetErrMsg();
    }

    return plLengths;
}

MYSQL_FIELD *CPetMySQL::FetchField()
{
    if (m_pstRES == NULL)
    {
        SetErrMsg("RES is not exist");
        return NULL;
    }

    MYSQL_FIELD *pstField;

    pstField = mysql_fetch_field(m_pstRES);
    if (pstField == NULL)
    {
        SetErrMsg();
    }

    return pstField;
}

//GCS要求业务自己处理连接丢失的问题
/*
int CPetMySQL::CheckConnection()
{
    int iRetVal = 0;
    // retry 2 times
    for (int i = 0; i < 2; i++)
    {
        iRetVal = mysql_ping(&m_stMYSQL);
        if (iRetVal == 0)
        {
            break;
        }
    }
    char szMsg[256] = "";
    snprintf(szMsg, sizeof(szMsg), "mysql_ping:%d", iRetVal);
    SetErrMsg(szMsg);

    return iRetVal;
}
*/

int CPetMySQL::CheckConnection()
{
    int iRetVal = 0;
    if(!m_IsInited)  //下面连接丢失且重连又不成功时会把m_stMYSQL关闭掉,这个时候尝试重连
    {
        iRetVal = Reconnect();
    }
    else  //if(iRetVal == 0)
    {
        iRetVal = mysql_ping(&m_stMYSQL);
        if(iRetVal != 0)
        {
            unsigned int _mysql_errno = mysql_errno(&m_stMYSQL);
            if((_mysql_errno==MYSQL_ERR_BACKEND_DOWN || _mysql_errno==MYSQL_ERR_GONE_AWAY))
                iRetVal = Reconnect();
        }
    }

    char szMsg[256] = "";
    snprintf(szMsg, sizeof(szMsg), "mysql_ping:%d", iRetVal);
    SetErrMsg(szMsg);

    return iRetVal;
}

int CPetMySQL::EscapeString(char *pszDstSQL, const char *pszSrcSQL, int iSQLLen)
{
    int iRetVal = 0;
    iRetVal = mysql_real_escape_string(&m_stMYSQL, pszDstSQL, pszSrcSQL, iSQLLen);

    return iRetVal;
}

int CPetMySQL::Reconnect()
{
    if(m_IsInited)
        mysql_close(&m_stMYSQL);
    m_IsInited = false;

    int retry_times = 0;
    while(retry_times < MYSQL_RECONNECT_COUNT)
    {
        if (mysql_init(&m_stMYSQL) == NULL)
            return -9999;
        char cReconnectFlag = 0;  //自己处理连接的问题
        if (mysql_options(&m_stMYSQL, MYSQL_OPT_RECONNECT, (char *)&cReconnectFlag) != 0)
        {
            mysql_close(&m_stMYSQL);
            return -9998;
        }

        if(m_ReadTimeout > 0)
        {
            if (mysql_options(&m_stMYSQL, MYSQL_OPT_READ_TIMEOUT, (char *)&m_ReadTimeout) != 0)
            {
                SetErrMsg();
                mysql_close(&m_stMYSQL);
                return -2;
            }
        }
        if(m_WriteTimeout > 0)
        {
            if (mysql_options(&m_stMYSQL, MYSQL_OPT_WRITE_TIMEOUT, (char *)&m_WriteTimeout) != 0)
            {
                SetErrMsg();
                mysql_close(&m_stMYSQL);
                return -2;
            }
        }

        int iConnectOption = CLIENT_FOUND_ROWS; //����UPDATE������AffectRows��ʾƥ�������
        if (mysql_real_connect(&m_stMYSQL, m_DBHost.c_str(), m_DBUserName.c_str(), m_DBPassWd.c_str(), m_DBName.c_str(), m_DBPort, NULL, iConnectOption) != NULL)
            break;
        mysql_close(&m_stMYSQL);
        ++retry_times;
    }
    if(retry_times == MYSQL_RECONNECT_COUNT)
        return -9997;

    m_IsInited = true;
    return 0;
}

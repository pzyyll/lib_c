#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "webapi.h"
using namespace snslib;

CWebApi::CWebApi()
{
    m_uiHostIP = 0;
}

CWebApi::~CWebApi()
{
}

int CWebApi::Init(const char* pszConfFile)
{
    int iRetVal = 0;

    iRetVal = m_objWebApiConn.Init(pszConfFile);
    if (iRetVal != 0)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "webapi_conn init failed, ret=%d, errmsg=%s", iRetVal, m_objWebApiConn.GetErrMsg());
        return iRetVal;
    }

    //获取本机IP地址
    //优先获取eth1的IP地址，如果获取不到，再获取eth0的IP地址
    const char *pszHostAddr = NULL;
    pszHostAddr = CSysTool::GetNicAddr("eth1");
    if (pszHostAddr == NULL){
        pszHostAddr = CSysTool::GetNicAddr("eth0");
        if (pszHostAddr == NULL){
            m_uiHostIP = 0;
        }else{
            m_uiHostIP = inet_addr(pszHostAddr);
        }
    }else{
        m_uiHostIP = inet_addr(pszHostAddr);
    }


    //所有的功能基类，都要在这里调用一下BaseInit
    CWebApiFlag::BaseInit(&m_objWebApiConn, m_uiHostIP);
    CWebApiSnsInfo::BaseInit(&m_objWebApiConn, m_uiHostIP);
    CWebApiTest::BaseInit(&m_objWebApiConn, m_uiHostIP);
    CWebApiLock::BaseInit(&m_objWebApiConn, m_uiHostIP);
    CWebApiRandSelect::BaseInit(&m_objWebApiConn, m_uiHostIP);
    CWebApiQQInfo::BaseInit(&m_objWebApiConn, m_uiHostIP);
    CWebApiQQList::BaseInit(&m_objWebApiConn, m_uiHostIP);
    CWebApiKongfu::BaseInit(&m_objWebApiConn, m_uiHostIP);
    //其他一些WEBAPI业务模块初始化可以在这里做

    return 0;
}


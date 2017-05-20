/**
 * @file    app_transaction_api.h
 * @brief   事务处理接口，内部暂存空间64k
 * @author  rayxia@tencent.com
 * @date    2009-04-27
 *
 */

#ifndef _APP_TRANSACTION_API_H_
#define _APP_TRANSACTION_API_H_

#include <map>
#include <vector>
#include <string.h>
#include "api/include/sns_protocol.h"

namespace snslib
{

class CTransactionAPI
{
public:
    const static int SUCCESS = 0;
    const static int ERROR = -1;
    const static int ERROR_PARAM = -2;
    const static int ERROR_BUFF_LESS = -3;

public:
    // ushType: TRANSACTION_TYPE_ADD or TRANSACTION_TYPE_MODIFY
	CTransactionAPI( unsigned int uiUin, unsigned short ushType, unsigned int uiID = 0 );
	// V2简化版，不支持SNSAPP V1
	CTransactionAPI( unsigned int uiUin );
    ~CTransactionAPI();

    // ushType:TRANSACTION_TYPE_ADD, ushLevel:STEP_LEVEL_AUTO
    int AddStep( unsigned int uiUin, unsigned short ushType, unsigned short ushLevel, unsigned short ushAppID, unsigned short ushSvrID,
    		unsigned short ushCmd, char * pszData = NULL, unsigned int uiDataLen = 0 );
    // V2 简化版，不支持SNSAPP V1
    int AddStep( unsigned int uiUin, unsigned short ushSvrID, unsigned short ushCmd, const char * pszData = NULL, unsigned int uiDataLen = 0 );
    char * GetBufAddr();
    unsigned int GetBufLen();

    const char *GetErrMsg() {
		return m_szErrMsg;
	}

private:
    char m_szErrMsg[256];
    char m_szBuf[64*1024];
    unsigned int m_iBufLen;
    unsigned short m_ushStepID;
    unsigned short m_ushLevel;
    snslib::TransactionHeader m_stTransactionHeader;
};

}
#endif

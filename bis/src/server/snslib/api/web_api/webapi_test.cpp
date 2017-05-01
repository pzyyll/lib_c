#include "webapi_test.h"
#include "api/proto/snsapp.pb.h"

using namespace snslib;

const int WEBAPI_SNSINFO_MAX_UIN_NUM_PER_REQ = 100;

CWebApiTest::CWebApiTest()
{}

CWebApiTest::~CWebApiTest()
{}

int CWebApiTest::BenchTest(unsigned int uiUin, int iReadLen, int iWriteLen)
{
    int iRetVal = 0;

    BenchTestReq objBenchTestReq;
    BenchTestRsp objBenchTestRsp;

    objBenchTestReq.set_read_len(iReadLen);
    objBenchTestReq.set_write_len(iWriteLen);

    iRetVal = SendAndRecv(uiUin, CMD_BENCH_TEST, 100, 2, objBenchTestReq, objBenchTestRsp);

    if (iRetVal != 0)
    {
        return iRetVal;
    }

    return 0;
}

int CWebApiTest::EchoTest(unsigned int uiUin, char *pszEchoText)
{
    int iRetVal = 0;

    AppHeader stSendAppHeader, stRecvAppHeader;
    memset(&stSendAppHeader, 0x0, sizeof(stSendAppHeader));
    stSendAppHeader.uiUin = uiUin;
    stSendAppHeader.ushCmdID = CMD_TEST_ECHO;
    stSendAppHeader.ushDestSvrID = 2;

    char *pszRecvBuff = NULL;
    int iRecvLen = 0;
    iRetVal = SendAndRecv(stSendAppHeader, pszEchoText, strlen(pszEchoText), stRecvAppHeader, &pszRecvBuff, &iRecvLen);
    if (iRetVal != 0)
    {
        return -2;
    }

    if (iRecvLen <= 0)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "recv len is not valid, recv_len=%d", iRecvLen);
        return -3;
    }

    if (stRecvAppHeader.iRet != 0)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "ret val in header is %d", stRecvAppHeader.iRet);
        return stRecvAppHeader.iRet;
    }

    if (iRecvLen != (int)strlen(pszEchoText))
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "echo text len is not valid, ret_text_len=%d, send_text_len=%d", iRecvLen, (int)strlen(pszEchoText));
        return -4;
    }

    return 0;
}

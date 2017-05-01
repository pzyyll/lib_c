#include <arpa/inet.h>

#include "comm/log/pet_log.h"
#include "comm/ini_file/ini_file.h"
#include "comm/util/pet_util.h"

#include "api/include/pet_protocol.h"
#include "api/session_api/feedsvr_prot.h"
#include "comm/safe_tcp_client/safe_tcp_client.h"

using namespace petlib;

CSafeTcpClient g_objTcpClient;

int GetConnStatus()
{
    STcpConnStatusHeader stSTcpConnStatusHeader;
    std::vector<STcpConnStatus> vstConnStatus;
    g_objTcpClient.GetAllStatus(stSTcpConnStatusHeader, vstConnStatus);
    char szCreateTime[64];
    char szLastCheckTime[64];

    snprintf(szCreateTime, sizeof(szCreateTime), "%s", CStrTool::TimeString(stSTcpConnStatusHeader.tCreateTime));
    snprintf(szLastCheckTime, sizeof(szLastCheckTime), "%s", CStrTool::TimeString(stSTcpConnStatusHeader.tLastCheckTime));

    printf("CONN STATUS HEADER\n");
    printf(" MAGIC:%x VER:%x CTIME:%s[%zd] LCTIME:%s[%zd] FULLNUM:%d\n",
        stSTcpConnStatusHeader.uiMagicNum,
        stSTcpConnStatusHeader.ushVersion,
        szCreateTime, stSTcpConnStatusHeader.tCreateTime,
        szLastCheckTime, stSTcpConnStatusHeader.tLastCheckTime,
        stSTcpConnStatusHeader.uiStatusFullNum
        );

    printf("CONN STATUS NUM:%zd\n", vstConnStatus.size());
    for (unsigned int i=0; i<vstConnStatus.size(); i++)
    {
        snprintf(szCreateTime, sizeof(szCreateTime), "%s", CStrTool::TimeString(vstConnStatus[i].tLastRetryTime));

        printf("CONN[%d] %s:%d STATUS:%d LRTIME:%s TIMEOUT:%d RETRY:%u CENUM:%u RENUM:%u SENUM:%u LRNUM:%u\n",
            i,
            inet_ntoa(*((struct in_addr*)&vstConnStatus[i].uiIPAddr)),
            vstConnStatus[i].ushPort,
            vstConnStatus[i].ushStatus,
            szCreateTime,
            vstConnStatus[i].uiTimeOut,
            vstConnStatus[i].uiRetryTimes,
            vstConnStatus[i].uiConnErrNum,
            vstConnStatus[i].uiRecvErrNum,
            vstConnStatus[i].uiSendErrNum,
            vstConnStatus[i].uiLastReqNum);
    }

    return 0;
}

int SetConnStatus(const STcpConnStatus &stSTcpConnStatus)
{
    int iRetVal = 0;
    iRetVal = g_objTcpClient.SetConnStatus(stSTcpConnStatus);

    if (iRetVal == 0)
    {
        printf("SET CONN STATUS[%s:%d %d] SUCC\n", inet_ntoa(*((struct in_addr*)&stSTcpConnStatus.uiIPAddr)), stSTcpConnStatus.ushPort, stSTcpConnStatus.ushStatus);
    }
    else
    {
        printf("SET CONN STATUS[%s:%d %d] FAIL\n", inet_ntoa(*((struct in_addr*)&stSTcpConnStatus.uiIPAddr)), stSTcpConnStatus.ushPort, stSTcpConnStatus.ushStatus);
    }

    printf("\nNOW CONN STATUS:\n");

    GetConnStatus();

    return 0;
}

int DelConnStatus(const STcpConnStatus &stSTcpConnStatus)
{
    int iRetVal = 0;
    iRetVal = g_objTcpClient.DelConnStatus(stSTcpConnStatus);

    if (iRetVal == 0)
    {
        printf("DEL CONN STATUS[%s:%d] SUCC\n", inet_ntoa(*((struct in_addr*)&stSTcpConnStatus.uiIPAddr)), stSTcpConnStatus.ushPort);
    }
    else
    {
        printf("SET CONN STATUS[%s:%d] FAIL\n", inet_ntoa(*((struct in_addr*)&stSTcpConnStatus.uiIPAddr)), stSTcpConnStatus.ushPort);
    }

    printf("\nNOW CONN STATUS:\n");

    GetConnStatus();

    return 0;
}

int Usage(const char *pszProgName)
{
    printf("usage: %s <cmd> [...]\n", pszProgName);
    printf(" cmd:get set del\n");
    printf(" get:get all conn status\n");
    printf(" set:set conn status, set <ip> <port> <status>\n");
    printf(" del:del conn status, del <ip> <port>\n");
    printf(" status:1-OK 2-CONN_FAIL 3-RECV_FAIL 4-SEND_FAIL\n");

    return -1;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return Usage(argv[0]);
    }

    OpenPetLog("stcp_tool");

    if(strcmp(argv[1], "get") == 0)
    {
        GetConnStatus();
    }
    else if (strcmp(argv[1], "set") == 0)
    {
        if (argc < 5)
        {
            return Usage(argv[0]);
        }
        else
        {
            STcpConnStatus stSTcpConnStatus;
            stSTcpConnStatus.uiIPAddr = inet_addr(argv[2]);
            stSTcpConnStatus.ushPort = atoi(argv[3]);
            stSTcpConnStatus.ushStatus = atoi(argv[4]);

            if ((stSTcpConnStatus.uiIPAddr == 0) || (stSTcpConnStatus.ushPort == 0))
            {
                printf("host[%s] or port[%s] is not valid\n", argv[2], argv[3]);
                return -1;
            }

            if ((stSTcpConnStatus.ushStatus < 1) || (stSTcpConnStatus.ushStatus > 4))
            {
                printf("status[%s] is not valid\n", argv[4]);
                return -1;
            }

            SetConnStatus(stSTcpConnStatus);
        }
    }
    else if (strcmp(argv[1], "del") == 0)
    {
        if (argc < 4)
        {
            return Usage(argv[0]);
        }
        else
        {
            STcpConnStatus stSTcpConnStatus;
            stSTcpConnStatus.uiIPAddr = inet_addr(argv[2]);
            stSTcpConnStatus.ushPort = atoi(argv[3]);

            if ((stSTcpConnStatus.uiIPAddr == 0) || (stSTcpConnStatus.ushPort == 0))
            {
                printf("host[%s] or port[%s] is not valid\n", argv[2], argv[3]);
                return -1;
            }

            DelConnStatus(stSTcpConnStatus);
        }
    }
    else
    {
        return Usage(argv[0]);
    }

    return 0;
}

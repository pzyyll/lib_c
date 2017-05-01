#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "comm/bus_api/bus_api.h"

using namespace snslib;

const int MAX_DST_BUS_ID = 20;

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("usage: %s <bus_api_conf> <proc_id> <dest_id>\n", argv[0]);
        return -1;
    }

    unsigned int uiProcID = 0;

    uiProcID = inet_addr(argv[2]);
    if (uiProcID == 0)
    {
        printf("proc_id[%s] is not valid\n", argv[2]);
        return -2;
    }

    unsigned int uiDestID = inet_addr(argv[3]);
    if (uiDestID == 0)
    {
        printf("dest_id[%s] is not valid\n", argv[3]);
        return -2;
    }

    int iRetVal = 0;

    CBusApi objBus;
    iRetVal = objBus.Init(argv[1], uiProcID);
    if (iRetVal != 0)
    {
        printf("bus init failed, conf=%s, ret=%d, errmsg=%s\n", argv[1], iRetVal, objBus.GetErrMsg());
        return -3;
    }

    char szSendBuff[1024]={0};
    char iSendLen = 0;

    char szTestStr[]="THIS IS A TEST PKG!!";

    BusHeader stBusHeader;
    memset(&stBusHeader, 0x0, sizeof(stBusHeader));

    stBusHeader.uiSrcID = uiProcID;
    // stBusHeader.uiDestID = inet_addr("2.0.0.1");
    stBusHeader.uiDestID = uiDestID;

    memcpy(szSendBuff, &stBusHeader, sizeof(stBusHeader));
    memcpy(szSendBuff+sizeof(stBusHeader), szTestStr, strlen(szTestStr));

    iSendLen = sizeof(stBusHeader) + strlen(szTestStr);
    iRetVal = objBus.SendToAllRouter(szSendBuff, iSendLen);
    if (iRetVal != 0)
    {
        printf("bus send failed, ret=%d, errmsg=%s\n", iRetVal, objBus.GetErrMsg());
        return -7;
    }

    return 0;
}


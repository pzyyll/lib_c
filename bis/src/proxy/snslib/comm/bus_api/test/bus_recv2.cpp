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
        printf("usage: %s <bus_api_conf> <proc_id> <forward_flag>\n", argv[0]);
        printf("  forward_flag 1-forward 0-not forward\n");
        return -1;
    }

    unsigned int uiProcID = 0;
    int iRetVal = 0;

    if (inet_aton(argv[2], (struct in_addr *)&uiProcID) == 0)
    {
        printf("proc_id[%s] is not valid\n", argv[2]);
        return -1;
    }
    if (uiProcID == 0)
    {
        printf("proc_id[%s] is not valid\n", argv[2]);
        return -2;
    }

    int iForwardFlag = 0;

    iForwardFlag = atoi(argv[3]);

    CBusApi objBus;
    iRetVal = objBus.Init(argv[1], uiProcID);
    if (iRetVal != 0)
    {
        printf("bus init failed, ret=%d, errmsg=%s\n", iRetVal, objBus.GetErrMsg());
        return -3;
    }

    char szRecvBuff[102400]={0};
    unsigned int uiSrcBusID = 0;

    while(true)
    {
        int iRecvLen = sizeof(szRecvBuff);
        memset(szRecvBuff, 0x0, sizeof(szRecvBuff));
        iRetVal = objBus.Recv(&uiSrcBusID, szRecvBuff, &iRecvLen);
        if (iRetVal == objBus.BUS_EMPTY)
        {
            usleep(100000);
            continue;
        }
        else if (iRetVal != 0)
        {
            printf("bus recv failed, ret=%d, errmsg=%s\n", iRetVal, objBus.GetErrMsg());
            return -4;
        }

        char szSrcBusID[20] = {0};
        char szDstBusID[20] = {0};

        snprintf(szSrcBusID, sizeof(szSrcBusID), "%s", inet_ntoa(*((struct in_addr*)&uiSrcBusID)));
        snprintf(szDstBusID, sizeof(szDstBusID), "%s", inet_ntoa(*((struct in_addr*)&uiProcID)));

        //printf("RECV[%s-%s](%d)[%s]\n", szSrcBusID, szDstBusID, iRecvLen, CStrTool::Str2Hex(szRecvBuff, iRecvLen));

        if (iForwardFlag == 1)
        {
            memset(szRecvBuff, 0x0, 2);
            iRetVal = objBus.Send(uiSrcBusID, szRecvBuff, iRecvLen);
            if (iRetVal != 0)
            {
                printf("bus send failed, ret=%d, errmsg=%s\n", iRetVal, objBus.GetErrMsg());
                return -5;
            }

            snprintf(szSrcBusID, sizeof(szSrcBusID), "%s", inet_ntoa(*((struct in_addr*)&uiProcID)));
            snprintf(szDstBusID, sizeof(szDstBusID), "%s", inet_ntoa(*((struct in_addr*)&uiSrcBusID)));

            //printf("FRWD[%s-%s](%d)[%s]\n", szSrcBusID, szDstBusID, iRecvLen, CStrTool::Str2Hex(szRecvBuff, iRecvLen));
        }
    }

    return 0;
}



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
    if (argc < 5)
    {
        printf("usage: %s <bus_api_conf> <proc_id> <speed> <dest_id1> [dest_id2] ...\n", argv[0]);
        printf("  speed: pkg/s\n");
        return -1;
    }

    int iDestBusIDNum = argc - 4;
    unsigned int uiProcID = 0;
    unsigned int auiDestBusID[MAX_DST_BUS_ID] = {0};
    int iRetVal = 0;

    if (iDestBusIDNum > MAX_DST_BUS_ID)
    {
        iDestBusIDNum = MAX_DST_BUS_ID;
    }

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

    int iSpeed = atoi(argv[3]);
    if (iSpeed == 0)
    {
        printf("speed[%s] is not valid\n", argv[3]);
        return -3;
    }

    for (int i=0; i<iDestBusIDNum; i++)
    {
        if (inet_aton(argv[i+4], (struct in_addr *)&auiDestBusID[i]) == 0)
        {
            printf("dest_id%d[%s] is not valid\n", i+1, argv[i+4]);
            return -4;
        }
        if (auiDestBusID[i] == 0)
        {
            printf("dest_id%d[%s] is not valid\n", i+1, argv[i+4]);
            return -5;
        }
    }

    CBusApi objBus;
    iRetVal = objBus.Init(argv[1], uiProcID);
    if (iRetVal != 0)
    {
        printf("bus init failed, conf=%s, ret=%d, errmsg=%s\n", argv[1], iRetVal, objBus.GetErrMsg());
        return -6;
    }

    char szSendBuff[102400]={0};
    char iSendLen = 0;

    snprintf(szSendBuff, sizeof(szSendBuff), "THIS IS A TEST PKG!!");

    while(true)
    {
        for (int i=0; i<iDestBusIDNum; i++)
        {
            iSendLen = strlen(szSendBuff);
            iRetVal = objBus.Send(auiDestBusID[i], szSendBuff, iSendLen);
            if (iRetVal != 0)
            {
                printf("bus send failed, ret=%d, errmsg=%s\n", iRetVal, objBus.GetErrMsg());
                return -7;
            }
            char szSrcBusID[20] = {0};
            char szDstBusID[20] = {0};

            snprintf(szSrcBusID, sizeof(szSrcBusID), "%s", inet_ntoa(*((struct in_addr*)&uiProcID)));
            snprintf(szDstBusID, sizeof(szDstBusID), "%s", inet_ntoa(*((struct in_addr*)&auiDestBusID[i])));
            printf("SEND[%s-%s](%d)[%s]\n", szSrcBusID, szDstBusID, iSendLen, CStrTool::Str2Hex(szSendBuff, iSendLen));
        }
        usleep(500000);
    }

    return 0;
}


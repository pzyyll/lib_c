#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "comm/mem_pool/fixedsize_mem_pool.h"

using namespace snslib;

typedef struct tagMyNode
{
    int iID;
    char szName[256];
}MyNode;

int main()
{
    int iRetVal = 0;
    CFixedsizeMemPool<MyNode> objFMP;
    int iMemSize = 10240000;

    srandom(getpid());

    void *pvMem = malloc(iMemSize);

    if(pvMem == NULL)
    {
        printf("malloc mem failed!!\n");
        return -1;
    }

    iRetVal = objFMP.Init(pvMem, iMemSize, 1000, 1);
    if(iRetVal != 0)
    {
        printf("FMP init failed, ret=%d!!\n", iRetVal);
    }

    printf("FMP init success!!\n");

    MyNode *apstMyNode[1000] = {NULL};

    for(int i=0; i<500; i++)
    {
        apstMyNode[i] = objFMP.AllocateNode();
        apstMyNode[i]->iID = i;
        snprintf(apstMyNode[i]->szName, sizeof(apstMyNode[i]->szName), "jamieli_%d",  i);
    }

    for(int i=0; i<10000000; i++)
    {
        int iRandom=random()%1000;
        if (apstMyNode[iRandom] != NULL)
        {
            if (apstMyNode[iRandom]->iID != iRandom)
            {
                printf("verify failed, random=%d, id=%d\n", iRandom, apstMyNode[iRandom]->iID);
                return -1;
            }

            iRetVal = objFMP.ReleaseNode(apstMyNode[iRandom]);
            if (iRetVal != 0)
            {
                printf("release node failed, random=%d\n", iRandom);
                return -1;
            }

            //printf("release node succ, random=%d\n", iRandom);

            apstMyNode[iRandom] = NULL;
        }
        else
        {
            apstMyNode[iRandom] = objFMP.AllocateNode();
            if (apstMyNode[iRandom] == NULL)
            {
                printf("allocate node failed, random=%d\n", iRandom);
            }

            //printf("allocate node succ, random=%d\n", iRandom);

            apstMyNode[iRandom]->iID = iRandom;
            snprintf(apstMyNode[iRandom]->szName, sizeof(apstMyNode[iRandom]->szName), "jamieli_%d", iRandom);
        }

    }

    iRetVal = objFMP.Verify();
    if(iRetVal != 0)
    {
        printf("FMP verify failed after allocate, ret=%d!!\n", iRetVal);
    }

    printf("FMP verify succ after allocate!!\n");

    return 0;
}

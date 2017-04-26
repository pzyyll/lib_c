#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int iMemSize = 1024000;

    void *pvMem = malloc(iMemSize);

    if(pvMem == NULL)
    {
        printf("malloc mem failed!!\n");
        return -1;
    }

    iRetVal = objFMP.Init(pvMem, iMemSize, 100, 1);
    if(iRetVal != 0)
    {
        printf("FMP init failed, ret=%d!!\n", iRetVal);
    }

    printf("FMP init success!!\n");

    MyNode *apstMyNode[10];

    for(int i=0; i<10; i++)
    {
        apstMyNode[i] = objFMP.AllocateNode();
        apstMyNode[i]->iID = i+100;
        snprintf(apstMyNode[i]->szName, sizeof(apstMyNode[i]->szName), "jamieli_%d",  i+200);
    }

    iRetVal = objFMP.Verify();
    if(iRetVal != 0)
    {
        printf("FMP verify failed after allocate, ret=%d!!\n", iRetVal);
    }

    objFMP.Show();

    for(int i=0; i<10; i++)
    {
        printf("NODE[%d]|%d|%s\n", i, apstMyNode[i]->iID, apstMyNode[i]->szName);
    }

    objFMP.Show();

    for(int i=0; i<10; i++)
    {
        objFMP.ReleaseNode(apstMyNode[i]);
    }

    objFMP.Show();

    iRetVal = objFMP.Verify();
    if(iRetVal != 0)
    {
        printf("FMP verify failed after release, ret=%d!!\n", iRetVal);
    }


    return 0;
}

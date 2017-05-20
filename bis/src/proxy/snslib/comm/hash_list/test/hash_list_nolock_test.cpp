#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comm/hash_list/hash_list_nolock.h"

using namespace snslib;

typedef struct tagMyNode
{
    int iID;
    char szName[256];
}MyNode;

int main()
{
    int iRetVal = 0;
    CHashListNoLock<int, MyNode> objHashList;
    int iMemSize = 1024000;

    void *pvMem = malloc(iMemSize);

    if(pvMem == NULL)
    {
        printf("malloc mem failed!!\n");
        return -1;
    }

    iRetVal = objHashList.Init(pvMem, iMemSize, 2, 20);
    if(iRetVal != 0)
    {
        printf("FMP init failed, ret=%d!!\n", iRetVal);
    }

    printf("FMP init success!!\n");

    objHashList.Clear();

    MyNode *pstMyNode;
    MyNode stMyNode;

    for(int i=0; i<10; i++)
    {
        stMyNode.iID = i+100;
        snprintf(stMyNode.szName, sizeof(stMyNode.szName), "jamieli_%d",  i+200);

        iRetVal = objHashList.Insert(i, stMyNode);
        if (iRetVal != 0)
        {
            printf("HashList Insert failed, id=%d, mynode_id=%d, mynode_name=%s!!\n", i, stMyNode.iID, stMyNode.szName);
            return -1;
        }
    }

    for(int i=0; i<10; i++)
    {
        pstMyNode = objHashList.Get(i);
        printf("NODE[%d]|%d|%s\n", i, pstMyNode->iID, pstMyNode->szName);
    }

    for(int i=0; i<10; i++)
    {
        objHashList.Remove(i);
    }

    return 0;
}

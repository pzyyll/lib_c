#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comm/mem_pool/pet_map.h"

using namespace snslib;

typedef struct tagMyNode
{
    int iID;
    char szName[28];
}MyNode;

int main()
{
    int iRetVal = 0;
    int iMemSize = 10240000;

    CPetMap<unsigned long long, MyNode> objPetMap;

    void *pvMem = malloc(iMemSize);

    if(pvMem == NULL)
    {
        printf("malloc mem failed!!\n");
        return -1;
    }

    iRetVal = objPetMap.InitMemPool(pvMem, iMemSize, 800, 1);
    if(iRetVal != 0)
    {
        printf("PetMap init failed, ret=%d!!\n", iRetVal);
        return -1;
    }
    printf("PetMap init success!!\n");

    int aiHandle[100];

    for(int i=0; i<100; i++)
    {
        aiHandle[i] = objPetMap.NewMap();
        if (aiHandle[i] < 0)
        {
            printf("PetMap new_map failed, i=%d, ret=%d\n", i, aiHandle[i]);
            return -1;
        }

        printf("PetMap new_map succ, i=%d, handle=%d\n", i, aiHandle[i]);
    }

    MyNode stMyNode;

    for(int i=0; i<100; i++)
    {
        iRetVal = objPetMap.InitWithHandle(aiHandle[i]);
        if (iRetVal != objPetMap.SUCCESS)
        {
            printf("PetMap init_with_handle failed, handle=%d, ret=%d\n", aiHandle[i], iRetVal);
            return -1;
        }

        for(int j=0;j<500;j++)
        {
            stMyNode.iID = i*1000+j;
            snprintf(stMyNode.szName, sizeof(stMyNode.szName), "jamieli%d", stMyNode.iID);
            iRetVal = objPetMap.InsertNode(stMyNode.iID, stMyNode);
            if(iRetVal != objPetMap.SUCCESS)
            {
                printf("PetMap insert_node failed, handle=%d, key=%d, msg=%s, ret=%d\n", aiHandle[i], stMyNode.iID, stMyNode.szName, iRetVal);
                return -1;
            }
            printf("PetMap insert_node succ, handle=%d, key=%d, msg=%s\n", aiHandle[i], stMyNode.iID, stMyNode.szName);

        }
    }

    iRetVal = objPetMap.VerifyMemPool();
    if(iRetVal != 0)
    {
        printf("PetMap verify1 failed after release, ret=%d!!\n", iRetVal);
    }

    printf("PetMap verify1 succ\n");

    MyNode *pstMyNode = NULL;
    for(int i=0; i<100; i++)
    {
        iRetVal = objPetMap.InitWithHandle(aiHandle[i]);
        if (iRetVal != objPetMap.SUCCESS)
        {
            printf("PetMap init_with_handle failed, handle=%d, ret=%d\n", aiHandle[i], iRetVal);
            return -1;
        }

        for(int j=0;j<500;j++)
        {
            int iID = i*1000+j;
            iRetVal = objPetMap.GetNode(iID, &pstMyNode);
            if(iRetVal != objPetMap.SUCCESS)
            {
                printf("PetMap get_node failed, handle=%d, key=%d, ret=%d\n", aiHandle[i], iID, iRetVal);
                return -1;
            }

            printf("PetMap get_node succ, handle=%d, key=%d, msg=%s\n", aiHandle[i], iID, pstMyNode->szName);

        }
    }

    iRetVal = objPetMap.VerifyMemPool();
    if(iRetVal != 0)
    {
        printf("PetMap verify2 failed after release, ret=%d!!\n", iRetVal);
    }

    printf("PetMap verify2 succ\n");

    for(int i=0; i<100; i++)
    {
        iRetVal = objPetMap.InitWithHandle(aiHandle[i]);
        if (iRetVal != objPetMap.SUCCESS)
        {
            printf("PetMap init_with_handle failed, handle=%d, ret=%d\n", aiHandle[i], iRetVal);
            return -1;
        }
        iRetVal = objPetMap.ReleaseMap();
        if (iRetVal != objPetMap.SUCCESS)
        {
            printf("PetMap release failed, handle=%d, ret=%d\n", aiHandle[i], iRetVal);
            return -1;
        }

        printf("PetMap release succ, handle=%d\n", aiHandle[i]);

    }

    iRetVal = objPetMap.VerifyMemPool();
    if(iRetVal != 0)
    {
        printf("PetMap verify3 failed after release, ret=%d!!\n", iRetVal);
    }

    printf("PetMap verify3 succ\n");


    return 0;
}

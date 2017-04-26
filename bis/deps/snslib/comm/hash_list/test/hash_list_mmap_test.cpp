#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "comm/hash_list/hash_list_mmap.h"

using namespace snslib;

typedef struct tagMyDataNode
{
    unsigned long long ullPetID;
    char szPetMsg[20];
}MyDataNode;

class CMyMMapHashList: public CHashListMMap<unsigned long long, MyDataNode>
{
public:
    /* 使用CHashListMMap只需要重载Hash算法函数 */
	int Hash(const unsigned long long &Key)
	{
	    return (*((unsigned int *)(&Key))) % m_iIndexNodeNum;
	}

	/* 以下部分可以不重载，重载只是用于调试输出 */
	const char *FormatKey(const unsigned long long &Key, char *pBuff, int iBuffSize)
	{
		snprintf(pBuff, iBuffSize, "%llu", Key);
		return pBuff;
	}

	const char *FormatData(const MyDataNode &Data, char *pBuff, int iBuffSize)
	{
		snprintf(pBuff, iBuffSize, "%llu|%s", Data.ullPetID, Data.szPetMsg);
		return pBuff;
	}

};

void ProcessOneNode(MyDataNode &stData)
{
    printf("ProcessOneNode:%llu:%s\n", stData.ullPetID, stData.szPetMsg);
    return;
}

void ProcessAllNode(MyDataNode &stData)
{
    printf("ProcessAllNode:%llu:%s\n", stData.ullPetID, stData.szPetMsg);
    return;
}

int main(int argc, char* argv[])
{
	int iRetVal = 0;
	int iLoopNum = 100;

	if (argc > 1)
	{
	    iLoopNum = atoi(argv[1]);
	}

	CMyMMapHashList theMMapHashList;

	//创建MMapHashList
	iRetVal = theMMapHashList.Init("MMapHashList.mapfile", 10240000, 10000, 20000);
    if (iRetVal != theMMapHashList.SUCCESS)
    {
        printf("MMapHashList Init failed, ret=%d\n", iRetVal);
        return iRetVal;
    }

    //清空MMapHashList
	iRetVal = theMMapHashList.Clear();
	if (iRetVal != theMMapHashList.SUCCESS)
	{
		printf("MMapHashList Clear failed, ret=%d\n", iRetVal);
		return iRetVal;
	}


	//向MMapHashList插入两个数据节点
	MyDataNode theDataNode;
    memset(&theDataNode, 0x0, sizeof(theDataNode));
    strncpy(theDataNode.szPetMsg, "ABCDEFG_TEST", sizeof(theDataNode.szPetMsg));

    struct timeval stBeginTime, stEndTime, stTimeDiff;

    gettimeofday(&stBeginTime, NULL);

    int iINum = 0, iUNum = 0, iDNum = 0;

    for(int mm=0; mm<1; mm++)
    {
        for(int i=0; i<1; i++)
        {
            for(int j=0; j<5000; j++)
            {
                theDataNode.ullPetID = j;
                iRetVal = theMMapHashList.Insert(theDataNode.ullPetID, theDataNode);
                if (iRetVal != theMMapHashList.SUCCESS)
                {
                    printf("MMapHashList Insert failed, ret=%d\n", iRetVal);
                    return iRetVal;
                }
                iINum++;
            }
        }

        for(int i=0; i<100; i++)
        {
            for(int j=0; j<5000; j++)
            {
                 theDataNode.ullPetID = j;
                 iRetVal = theMMapHashList.Update(theDataNode.ullPetID, theDataNode);
                 if (iRetVal != theMMapHashList.SUCCESS)
                 {
                     printf("MMapHashList Update failed, ret=%d\n", iRetVal);
                     return iRetVal;
                 }
                 iUNum++;
            }
        }

        for(int i=0; i<1; i++)
        {
            for(int j=0; j<5000; j++)
            {
                 theDataNode.ullPetID = j;
                 iRetVal = theMMapHashList.Remove(theDataNode.ullPetID);
                 if (iRetVal != theMMapHashList.SUCCESS)
                 {
                     printf("MMapHashList Remove failed, ret=%d\n", iRetVal);
                     return iRetVal;
                 }
                 iDNum++;
            }
        }
    }
    gettimeofday(&stEndTime, NULL);
    timersub(&stEndTime, &stBeginTime, &stTimeDiff);

    printf("LOOP: i:%d u:%d d:%d USE_TIME: %luS:%luUS\n",
            iINum, iUNum, iDNum,
            stTimeDiff.tv_sec, stTimeDiff.tv_usec);

/*

	memset(&theDataNode, 0x0, sizeof(theDataNode));
	theDataNode.ullPetID = strtoull("281913513274626", NULL, 10);    //449899778
	strncpy(theDataNode.szPetMsg, "test pet2, uin=449899778", sizeof(theDataNode.szPetMsg));

	iRetVal = theMMapHashList.Insert(theDataNode.ullPetID, theDataNode);
	if (iRetVal != theMMapHashList.SUCCESS)
	{
		printf("MMapHashList Insert failed, ret=%d\n", iRetVal);
		return iRetVal;
	}

    memset(&theDataNode, 0x0, sizeof(theDataNode));
    unsigned long long ullPetID = 0;
    iRetVal = theMMapHashList.GetRandomNode(ullPetID, theDataNode);
    if (iRetVal != theMMapHashList.SUCCESS)
    {
        printf("MMapHashList GetRandomNode failed, ret=%d\n", iRetVal);
        return iRetVal;
    }

    printf("get_random_node[%llu][%s]\n", ullPetID, theDataNode.szPetMsg);


	//打印HashList的信息
	printf("==============HeadInfo==============\n");
	theMMapHashList.ShowHeadInfo();
	printf("==============HeadInfoEnd===========\n\n");
	//theMMapHashList.ShowIndexInfo();
	//printf("\n");
    printf("==============DataInfo==============\n");
	theMMapHashList.ShowDataInfo();
    printf("==============DataInfoEnd===========\n\n");


	//回调函数处理单个节点
	iRetVal = theMMapHashList.Process(strtoull("72339511450412525", NULL, 10), ProcessOneNode);
    if (iRetVal != theMMapHashList.SUCCESS)
    {
        printf("MMapHashList Process failed, ret=%d\n", iRetVal);
        return iRetVal;
    }

    //回调函数处理所有节点
    iRetVal = theMMapHashList.ProcessAll(ProcessAllNode);
    if (iRetVal != theMMapHashList.SUCCESS)
    {
        printf("MMapHashList ProcessAll failed, ret=%d\n", iRetVal);
        return iRetVal;
    }
*/
    return 0;
}


/*
 * 例子  addky
 * 			./tt_feeds_test  tt_feeds_api.ini addky  9899  1 1  hao_key user
 * getallkey
 *  		./tt_feeds_test  tt_feeds_api.ini getallky  9899
 *
 *  		具体字段含有看下面代码
 *
 * */

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "api/tt_feeds_api/tt_feeds_api.h"
#include "comm/log/pet_log.h"

using namespace snslib;

int main(int argc, char *argv[])
{
    int iRetVal = 0;

    if (argc < 4)
    {
        printf("usage: %s <conf_file> <cmd> <uin> ...\n", argv[0]);
        printf(" cmd: add/get/getall/del/delall/getnc/cleannc/addky/getallky\n");
        return -1;
    }

    OpenPetLog("tt_feeds_test");

    CTTFeedsApi objTTFeedsApi;

    const char *pszConfFile = argv[1];
    const char *pszCmd = argv[2];
    unsigned int uiUin = atoi(argv[3]);

    iRetVal = objTTFeedsApi.Init(pszConfFile);
    if (iRetVal != 0)
    {
        printf("init failed, ret=%d\n", iRetVal);
        return -1;
    }

    if (strncmp(pszCmd, "add", 6)==0)
    {
        if (argc < 5)
        {
            printf("usage: %s <conf_file> add <uin> <msg>\n", argv[0]);
            return -1;
        }

        std::string sFeedsMsg;
        sFeedsMsg.assign(argv[4]);

        //while (getchar() != EOF)
        {
        iRetVal = objTTFeedsApi.AddFeeds(uiUin, sFeedsMsg);
        if (iRetVal != 0)
        {
            printf("add failed, ret=%d\n", iRetVal);
            //return -1;
        }
        else
        {
            printf("add succ\n");
            //return 0;
        }
        }
    }

    else if (strncmp(pszCmd, "addky", 6)==0)
    {
    	keyval FeedsKeyVal;
    	int nAppID = atoi(argv[4]);
    	int nMsgID = atoi(argv[5]);

    	for (int i = 6; i < argc; i++)
    	{
    		char szkey[64] = {0};
    		char szval[64] = {0};
    		snprintf(szkey, sizeof(szkey), "key%s", argv[i]);
    		snprintf(szval, sizeof(szval), "val%s", argv[i]);
    		std::string key =  szkey;
    		std::string val =  szval;
        	FeedsKeyVal[key] = val;
    	}


        iRetVal = objTTFeedsApi.AddFeedsKeyVal(uiUin, nAppID, nMsgID, FeedsKeyVal);
        if (iRetVal != 0)
        {
            printf("add keyval failed, ret=%d\n", iRetVal);
            //return -1;
        }
        else
        {
            printf("add keyval succ\n");
            //return 0;
        }

    }
    else if (strncmp(pszCmd, "get", 6)==0)
    {
        if (argc < 5)
        {
            printf("usage: %s <conf_file> get <uin> <feeds_id>\n", argv[0]);
            return -1;
        }

        unsigned long long ullFeedsID = strtoull(argv[4], NULL, 10);

        //while (getchar() != EOF)
        {
        std::string sFeedsMsg;

        iRetVal = objTTFeedsApi.GetFeeds(uiUin, ullFeedsID, sFeedsMsg);
        if (iRetVal != 0)
        {
            printf("getall failed, ret=%d\n", iRetVal);
            //return -1;
        }
        else
        {
            printf("get succ\n");
            printf("FEEDS|%llu|%s\n", ullFeedsID, sFeedsMsg.c_str());
            //return 0;
        }
        }
    }

    else if (strncmp(pszCmd, "getall", 10)==0)
    {
        std::vector<std::string> vsFeedsMsg;
        std::vector<unsigned long long> vullFeedsID;
        int iCleanFlag = 0;
        if (argc > 4)
        {
            iCleanFlag = atoi(argv[4]);
        }

        iRetVal = objTTFeedsApi.GetAllFeeds(uiUin, vullFeedsID, vsFeedsMsg, iCleanFlag);
        if (iRetVal != 0)
        {
            printf("getall failed, ret=%d\n", iRetVal);
            return -1;
        }
        else
        {
            printf("getall succ\n");
            for(unsigned int i=0; i<vullFeedsID.size(); i++)
            {
                time_t tFeedsTime = 0;
                char szFeedsTime[64] = {0};
                memcpy(&tFeedsTime, &vullFeedsID[i], 4);
                strftime(szFeedsTime, sizeof(szFeedsTime), "%Y-%m-%d %H:%M:%S", localtime((time_t*)&tFeedsTime));
                printf("FEEDS[%d]|%llu|%s|%s\n", i, vullFeedsID[i], szFeedsTime, vsFeedsMsg[i].c_str());
            }
            return 0;
        }
    }

    else if (strncmp(pszCmd, "getallky", 10)==0)
    {
    	std::vector<keyval> vecFeedsKeyVal;
    	std::vector<std::string> vecFeeds;
        std::vector<unsigned long long> vullFeedsID;
        int iCleanFlag = 0;
        if (argc > 4)
        {
            iCleanFlag = atoi(argv[4]);
        }

        iRetVal = objTTFeedsApi.GetAllFeedsKeyVal(uiUin, vullFeedsID, vecFeedsKeyVal, vecFeeds, iCleanFlag);
        if (iRetVal != 0)
        {
            printf("getall keyval failed, ret=%d\n", iRetVal);
            return -1;
        }
        else
        {
            printf("getall succ\n");
            for(unsigned int i=0; i<vullFeedsID.size(); i++)
            {
                time_t tFeedsTime = 0;
                char szFeedsTime[64] = {0};
                memcpy(&tFeedsTime, &vullFeedsID[i], 4);
                strftime(szFeedsTime, sizeof(szFeedsTime), "%Y-%m-%d %H:%M:%S", localtime((time_t*)&tFeedsTime));
                printf("FEEDS[%d]|%llu|%s|  ", i, vullFeedsID[i], szFeedsTime);
                keyval feedsky = vecFeedsKeyVal[i];
                for (keyval::iterator it = feedsky.begin(); it != feedsky.end(); ++it)
                {
                	printf("<%s, %s>    ", it->first.c_str(), it->second.c_str());
                }
                if (!vecFeeds.empty() && i < vecFeeds.size())
                {
                	printf("======%s\n", vecFeeds[i].c_str());
                }
                printf("\n");
            }


            return 0;
        }
    }
    else if (strncmp(pszCmd, "del", 6)==0)
    {
        if (argc < 5)
        {
            printf("usage: %s <conf_file> del <uin> <feeds_id>\n", argv[0]);
            return -1;
        }

        unsigned long long ullFeedsID = strtoull(argv[4], NULL, 10);

        iRetVal = objTTFeedsApi.DelFeeds(uiUin, ullFeedsID);
        if (iRetVal != 0)
        {
            printf("del failed, ret=%d\n", iRetVal);
            return -1;
        }
        else
        {
            printf("del succ\n");
            return 0;
        }
    }

    else if (strncmp(pszCmd, "delall", 6)==0)
    {
        iRetVal = objTTFeedsApi.DelAllFeeds(uiUin);
        if (iRetVal != 0)
        {
            printf("delall failed, ret=%d\n", iRetVal);
            return -1;
        }
        else
        {
            printf("delall succ\n");
            return 0;
        }
    }

    else if (strncmp(pszCmd, "getnc", 6)==0)
    {
        unsigned int iNewFeedsCount = 0;
        iRetVal = objTTFeedsApi.GetNewFeedsCount(uiUin, (unsigned int *)&iNewFeedsCount);
        if (iRetVal != 0)
        {
            printf("getnc failed, ret=%d\n", iRetVal);
            return -1;
        }
        else
        {
            printf("getnc succ\n");
            printf("new_feeds_count=%d\n", iNewFeedsCount);
            return 0;
        }
    }

    else if (strncmp(pszCmd, "cleannc", 6)==0)
    {
        iRetVal = objTTFeedsApi.CleanNewFeedsCount(uiUin);
        if (iRetVal != 0)
        {
            printf("cleannc failed, ret=%d\n", iRetVal);
            return -1;
        }
        else
        {
            printf("cleannc succ\n");
            return 0;
        }
    }

    else
    {
        printf("usage: %s <conf_file> <cmd> <uin> ...\n", argv[0]);
        printf(" cmd: add/get/getall/del/delall\n");
        return -1;
    }

    return 0;
}

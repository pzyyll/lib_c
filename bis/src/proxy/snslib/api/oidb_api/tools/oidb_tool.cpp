#include <stdio.h>
#include <arpa/inet.h>
#include "api/oidb_api/oidb_api.h"
#include "comm/util/pet_util.h"
#include "comm/log/pet_log.h"

using namespace snslib;

typedef void(*TestFunc)(int argc, char *argv[]);

COIDBProxyAPI g_objOIDB;

void TestGetFriendList(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);
    std::vector<unsigned int> vecFriends;
    int iRetCode = g_objOIDB.GetFriendList(uiUin,  vecFriends);

    printf("ret: %d, %s\n", iRetCode, g_objOIDB.GetErrMsg());

    if (iRetCode == 0)
    {
        printf("uin: %u, friend count: %u\n", uiUin, static_cast<unsigned int>(vecFriends.size()));
        for (unsigned int i = 0; i < vecFriends.size(); ++i)
        {
            printf("%u\n", vecFriends[i]);
        }
    }
}

void TestGetFriendListSession(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);
    SessionContext stContext;
    stContext.uiAppID = atoi(argv[3]);
    stContext.cKeyType = atoi(argv[4]);
    stContext.ushSessionKeyLen = snprintf(stContext.szSessionKey, sizeof(stContext.szSessionKey), "%s", argv[5]);
    stContext.uiClientIP = inet_addr(argv[6]);
    stContext.uiConnSvrIP = inet_addr(argv[7]);

    std::vector<unsigned int> vecFriends;
    int iRetCode = g_objOIDB.GetFriendList(uiUin, vecFriends, stContext);

    if (iRetCode == 0)
    {
        printf("uin: %u, friend count: %u\n", uiUin, static_cast<unsigned int>(vecFriends.size()));
        for (unsigned int i = 0; i < vecFriends.size(); ++i)
        {
            printf("%u\n", vecFriends[i]);
        }
    }
    else
    {
        printf("ret:%d, %s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}

void TestGetRichFlag(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);
    unsigned char ucServiceID = atoi(argv[3]);
    unsigned char byFlag;
    int iRetCode = g_objOIDB.GetRichFlag(uiUin, byFlag, ucServiceID);

    printf("ret: %d, %s\n", iRetCode, g_objOIDB.GetErrMsg());

    if (iRetCode == 0)
    {
        printf("uin: %u, flag: %u\n", uiUin, byFlag);
    }
}

void TestGetRichFlagSession(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);

    SessionContext stContext;
    stContext.uiAppID = atoi(argv[3]);
    stContext.cKeyType = atoi(argv[4]);
    stContext.ushSessionKeyLen = snprintf(stContext.szSessionKey, sizeof(stContext.szSessionKey), "%s", argv[5]);
    stContext.uiClientIP = inet_addr(argv[6]);
    stContext.uiConnSvrIP = inet_addr(argv[7]);

    unsigned char ucServiceID = atoi(argv[8]);
    unsigned char byFlag;
    int iRetCode = g_objOIDB.GetRichFlag(uiUin, byFlag, ucServiceID, stContext);

    if (iRetCode == 0)
    {
        printf("uin: %u, flag: %u\n", uiUin, byFlag);
    }
    else
    {
        printf("ret:%d, %s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}

void TestSetRichFlag(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);
    unsigned char ucServiceID = atoi(argv[3]);
    unsigned char byFlag = atoi(argv[4]);;
    int iRetCode = g_objOIDB.SetRichFlag(uiUin, byFlag, ucServiceID);

    if (iRetCode == 0)
    {
        printf("uin: %u, flag: %u\n", uiUin, byFlag);
    }
    else
    {
        printf("ret: %d, %s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}

void TestGetSimpleInfo(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);
    SQQSimpleInfo stInfo;
    int iRetCode = g_objOIDB.GetSimpleInfo(uiUin, stInfo);

    printf("ret: %d, %s\n", iRetCode, g_objOIDB.GetErrMsg());

    if (iRetCode == 0)
    {
        printf("uin:    %u\n", uiUin);
        printf("age:    %u\n", stInfo.byAge);
        printf("gender: %u\n", stInfo.byGender);
        printf("nick:   %s\n", stInfo.szNick);
    }
}

void TestGetSimpleInfoSession(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);

    SessionContext stContext;
    stContext.uiAppID = atoi(argv[3]);
    stContext.cKeyType = atoi(argv[4]);
    stContext.ushSessionKeyLen = snprintf(stContext.szSessionKey, sizeof(stContext.szSessionKey), "%s", argv[5]);
    stContext.uiClientIP = inet_addr(argv[6]);
    stContext.uiConnSvrIP = inet_addr(argv[7]);

    SQQSimpleInfo stInfo;
    int iRetCode = g_objOIDB.GetSimpleInfo(uiUin, stInfo, stContext);

    if (iRetCode == 0)
    {
        printf("uin:    %u\n", uiUin);
        printf("face:   %u\n", stInfo.ushFace);
        printf("age:    %u\n", stInfo.byAge);
        printf("gender: %u\n", stInfo.byGender);
        printf("nick:   %s\n", stInfo.szNick);
    }
    else
    {
        printf("ret: %d, %s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}

void TestGetFriendsRichFlag(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);
    unsigned char ucServiceID = atoi(argv[3]);
    std::vector<unsigned int> vecFriendList;
    std::map<unsigned int, unsigned char> mapFlag;

    unsigned int uiFriendUin = 0;
    for (int i = 4; i < argc; ++i)
    {
        uiFriendUin = strtoul(argv[i], NULL, 10);
        vecFriendList.push_back(uiFriendUin);
    }

    int iRetCode = g_objOIDB.GetFriendsRichFlag(uiUin, vecFriendList, mapFlag, ucServiceID);

    printf("ret: %d\n", iRetCode);

    if (iRetCode == 0)
    {
        printf("uin: %u\n", uiUin);
        std::map<unsigned int, unsigned char>::const_iterator cit;
        for (cit = mapFlag.begin(); cit != mapFlag.end(); ++cit)
        {
            printf("%u %u\n", cit->first, cit->second);
        }
    }
    else
    {
        printf("errmsg:%s\n", g_objOIDB.GetErrMsg());
    }
}

void TestBatchGetRichFlagSession(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);

    SessionContext stContext;
    stContext.uiAppID = atoi(argv[3]);
    stContext.cKeyType = atoi(argv[4]);
    stContext.ushSessionKeyLen = snprintf(stContext.szSessionKey, sizeof(stContext.szSessionKey), "%s", argv[5]);
    stContext.uiClientIP = inet_addr(argv[6]);
    stContext.uiConnSvrIP = inet_addr(argv[7]);

    unsigned char ucServiceID = atoi(argv[8]);

    std::vector<unsigned int> vecFriendList;
    std::map<unsigned int, unsigned char> mapFlag;

    unsigned int uiFriendUin = 0;
    for (int i = 9; i < argc; ++i)
    {
        uiFriendUin = strtoul(argv[i], NULL, 10);
        vecFriendList.push_back(uiFriendUin);
    }

    int iRetCode = g_objOIDB.BatchGetRichFlag(uiUin, vecFriendList, mapFlag, ucServiceID, stContext);

    if (iRetCode == 0)
    {
        printf("uin: %u\n", uiUin);
        std::map<unsigned int, unsigned char>::const_iterator cit;
        for (cit = mapFlag.begin(); cit != mapFlag.end(); ++cit)
        {
            printf("%u %u\n", cit->first, cit->second);
        }
    }
    else
    {
        printf("ret: %d:%s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}

void TestBatchGetSimpleInfo(int argc, char *argv[])
{
    std::vector<unsigned int> vecUinList;
    unsigned int uiUin = 0;
    for (int i = 2; i < argc; ++i)
    {
        uiUin = strtoul(argv[i], NULL, 10);
        vecUinList.push_back(uiUin);
    }

    std::vector<SQQSimpleInfo> vecInfo;
    int iRetCode = g_objOIDB.BatchGetSimpleInfo(vecUinList, vecInfo);

    printf("ret: %d\n", iRetCode);

    if (iRetCode == 0)
    {
        std::vector<SQQSimpleInfo>::const_iterator cit;
        for (cit = vecInfo.begin(); cit != vecInfo.end(); ++cit)
        {
            printf("uin:    %u\n", cit->uiUin);
            printf("  face:   %u\n", cit->ushFace);
            printf("  age:    %u\n", cit->byAge);
            printf("  gender: %u\n", cit->byGender);
            printf("  nick:   %s\n", cit->szNick);
            printf("--------------------\n");
        }
    }
    else
    {
        printf("errmsg:%s\n", g_objOIDB.GetErrMsg());
    }
}

void TestBatchGetSimpleInfoSession(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);

    SessionContext stContext;
    stContext.uiAppID = atoi(argv[3]);
    stContext.cKeyType = atoi(argv[4]);
    stContext.ushSessionKeyLen = snprintf(stContext.szSessionKey, sizeof(stContext.szSessionKey), "%s", argv[5]);
    stContext.uiClientIP = inet_addr(argv[6]);
    stContext.uiConnSvrIP = inet_addr(argv[7]);

    std::vector<unsigned int> vecUinList;
    unsigned int uiCurUin = 0;
    for (int i = 8; i < argc; ++i)
    {
        uiCurUin = strtoul(argv[i], NULL, 10);
        vecUinList.push_back(uiCurUin);
    }

    std::map<unsigned int, SQQSimpleInfo> mstInfo;
    int iRetCode = g_objOIDB.BatchGetSimpleInfo(uiUin, vecUinList, mstInfo, stContext);

    if (iRetCode == 0)
    {
        std::map<unsigned int, SQQSimpleInfo>::const_iterator cit;
        for (cit = mstInfo.begin(); cit != mstInfo.end(); ++cit)
        {
            printf("uin:    %u\n", cit->first);
            printf("  face:   %u\n", cit->second.ushFace);
            printf("  age:    %u\n", cit->second.byAge);
            printf("  gender: %u\n", cit->second.byGender);
            printf("  nick:   %s\n", cit->second.szNick);
            printf("--------------------\n");
        }
    }
    else
    {
        printf("ret:%d:%s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}

void TestCheckFriend(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);
    unsigned int uiFriendUin = strtoul(argv[3], NULL, 10);
    int iRetCode = g_objOIDB.CheckFriend(uiUin, uiFriendUin);

    if (iRetCode == g_objOIDB.RET_IS_FRIEND || iRetCode == g_objOIDB.RET_NOT_FRIEND)
    {
        printf("%u %u %d\n", uiUin, uiFriendUin, iRetCode);
    }
    else
    {
        printf("%u %u %d %s\n", uiUin, uiFriendUin, iRetCode, g_objOIDB.GetErrMsg());
    }
}

void TestCheckFriendSession(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);

    SessionContext stContext;
    stContext.uiAppID = atoi(argv[3]);
    stContext.cKeyType = atoi(argv[4]);
    stContext.ushSessionKeyLen = snprintf(stContext.szSessionKey, sizeof(stContext.szSessionKey), "%s", argv[5]);
    stContext.uiClientIP = inet_addr(argv[6]);
    stContext.uiConnSvrIP = inet_addr(argv[7]);

    unsigned int uiFriendUin = strtoul(argv[8], NULL, 10);
    int iRetCode = g_objOIDB.CheckFriend(uiUin, uiFriendUin, stContext);

    if (iRetCode == g_objOIDB.RET_IS_FRIEND || iRetCode == g_objOIDB.RET_NOT_FRIEND)
    {
        printf("%u %u %d\n", uiUin, uiFriendUin, iRetCode);
    }
    else
    {
        printf("%u %u %d %s\n", uiUin, uiFriendUin, iRetCode, g_objOIDB.GetErrMsg());
    }
}

void TestGetMssFlag(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);
    unsigned short ushMssType = strtoul(argv[3], NULL, 10);
    char cMssValue = 0;

    int iRetCode = g_objOIDB.GetMssFlag(uiUin, ushMssType, cMssValue);
    if (iRetCode == 0)
    {
        printf("%u %d\n", uiUin, cMssValue);
    }
    else
    {
        printf("%u %d %s\n", uiUin, iRetCode, g_objOIDB.GetErrMsg());
    }
}

void TestSetMssFlag(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);
    unsigned short ushMssType = strtoul(argv[3], NULL, 10);
	char cMssValue = strtol(argv[4], NULL, 10);

    int iRetCode = g_objOIDB.SetMssFlag(uiUin, ushMssType, cMssValue);
    if (iRetCode == 0)
    {
        printf("SUCCESS\n");
    }
    else
    {
        printf("%u %d %s\n", uiUin, iRetCode, g_objOIDB.GetErrMsg());
    }
}

void TestBatchGetMssFlag(int argc, char *argv[])
{
    unsigned short ushMssType = strtoul(argv[2], NULL, 10);

    std::vector<unsigned int> vuiUinList;
    std::vector<QQMssInfo> vstQQMssInfo;

    for(int i=3; i<argc; i++)
    {
        unsigned int uiUin = strtoul(argv[i], NULL, 10);
        vuiUinList.push_back(uiUin);
    }

    int iRetCode = g_objOIDB.BatchGetMssFlag(vuiUinList, ushMssType, vstQQMssInfo);

    if (iRetCode == 0)
    {
        printf("RET_NUM=%u\n", static_cast<unsigned int>(vstQQMssInfo.size()));
        std::vector<QQMssInfo>::iterator pstQQMssInfo;

        for(pstQQMssInfo = vstQQMssInfo.begin(); pstQQMssInfo!=vstQQMssInfo.end(); pstQQMssInfo++)
        {
            printf(" %d %d %d\n", pstQQMssInfo->uiUin, pstQQMssInfo->ushMssType, pstQQMssInfo->cMssValue);
        }
    }
    else
    {
        printf("%d %s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}


void TestGetMssFlagSession(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);

    SessionContext stContext;
    stContext.uiAppID = atoi(argv[3]);
    stContext.cKeyType = atoi(argv[4]);
    stContext.ushSessionKeyLen = snprintf(stContext.szSessionKey, sizeof(stContext.szSessionKey), "%s", argv[5]);
    stContext.uiClientIP = inet_addr(argv[6]);
    stContext.uiConnSvrIP = inet_addr(argv[7]);

    unsigned short ushMssType = strtoul(argv[8], NULL, 10);

    char cMssVal = 0;
    int iRetCode = g_objOIDB.GetMssFlag(uiUin, ushMssType, cMssVal, stContext);

    if (iRetCode == 0)
    {
        printf("%u %d %d\n", uiUin, ushMssType, cMssVal);
    }
    else
    {
        printf("%d %s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}

void TestBatchGetMssFlagSession(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);

    SessionContext stContext;
    stContext.uiAppID = atoi(argv[3]);
    stContext.cKeyType = atoi(argv[4]);
    stContext.ushSessionKeyLen = snprintf(stContext.szSessionKey, sizeof(stContext.szSessionKey), "%s", argv[5]);
    stContext.uiClientIP = inet_addr(argv[6]);
    stContext.uiConnSvrIP = inet_addr(argv[7]);

    unsigned short ushMssType = strtoul(argv[8], NULL, 10);

    std::vector<unsigned int> vuiUinList;
    std::map<unsigned int, QQMssInfo> mstQQMssInfo;

    for(int i=9; i<argc; i++)
    {
        unsigned int uiUin2 = strtoul(argv[i], NULL, 10);
        vuiUinList.push_back(uiUin2);
    }

    int iRetCode = g_objOIDB.BatchGetMssFlag(uiUin, vuiUinList, ushMssType, mstQQMssInfo, stContext);

    if (iRetCode == 0)
    {
        printf("RET_NUM=%d\n", static_cast<unsigned int>(mstQQMssInfo.size()));
        std::map<unsigned int, QQMssInfo>::iterator pstQQMssInfo;

        for(pstQQMssInfo = mstQQMssInfo.begin(); pstQQMssInfo!=mstQQMssInfo.end(); pstQQMssInfo++)
        {
            printf(" %d %d %d\n", pstQQMssInfo->first, pstQQMssInfo->second.ushMssType, pstQQMssInfo->second.cMssValue);
        }
    }
    else
    {
        printf("%d %s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}

/*
void TestGetMssFlagMulti(int argc, char *argv[])
{
    QQAllMssInfo stQQAllMssInfo;
    unsigned int uiUin = strtoul(argv[2], NULL, 10);

    int iRetCode = g_objOIDB.GetMssFlagMulti(uiUin, stQQAllMssInfo);

    if (iRetCode == 0)
    {
        printf(" %d %s\n", stQQAllMssInfo.uiUin, CStrTool::Str2Hex(stQQAllMssInfo.szMssValue, sizeof(stQQAllMssInfo.szMssValue)));
    }
    else
    {
        printf("%d %s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}
*/

void TestGetRichFlagLevel(int argc, char *argv[]){
	unsigned int uiUin = strtoul(argv[2], NULL, 10);
	unsigned char byServiceType = strtoul(argv[3], NULL, 10);
	unsigned char byLevel = 0;

	int iRetCode = g_objOIDB.GetRichFlagLevel(uiUin, byServiceType, byLevel);
	if(iRetCode == 0)
	{
		printf("%u %hhu %hhu\n", uiUin, byServiceType, byLevel);
	}
	else
	{
		printf("ERROR %d %s\n", iRetCode, g_objOIDB.GetErrMsg());
	}
}

void TestGetRichFlagLevelSession(int argc, char *argv[]){
	unsigned int uiUin = strtoul(argv[2], NULL, 10);

    SessionContext stContext;
    stContext.uiAppID = atoi(argv[3]);
    stContext.cKeyType = atoi(argv[4]);
    stContext.ushSessionKeyLen = snprintf(stContext.szSessionKey, sizeof(stContext.szSessionKey), "%s", argv[5]);
    stContext.uiClientIP = inet_addr(argv[6]);
    stContext.uiConnSvrIP = inet_addr(argv[7]);

	unsigned char byServiceType = strtoul(argv[8], NULL, 10);
	unsigned char byLevel = 0;

	int iRetCode = g_objOIDB.GetRichFlagLevel(uiUin, byServiceType, byLevel, stContext);
	if(iRetCode == 0)
	{
		printf("%u %hhu %hhu\n", uiUin, byServiceType, byLevel);
	}
	else
	{
		printf("ERROR %d %s\n", iRetCode, g_objOIDB.GetErrMsg());
	}
}

void TestBatchGetRichFlagLevelSession(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);

    SessionContext stContext;
    stContext.uiAppID = atoi(argv[3]);
    stContext.cKeyType = atoi(argv[4]);
    stContext.ushSessionKeyLen = snprintf(stContext.szSessionKey, sizeof(stContext.szSessionKey), "%s", argv[5]);
    stContext.uiClientIP = inet_addr(argv[6]);
    stContext.uiConnSvrIP = inet_addr(argv[7]);

    unsigned char byServiceType = strtoul(argv[8], NULL, 10);

    std::vector<unsigned int> vuiUinList;
    std::map<unsigned int, unsigned char> mRichFlagLevel;

    for(int i=9; i<argc; i++)
    {
        unsigned int uiUin2 = strtoul(argv[i], NULL, 10);
        vuiUinList.push_back(uiUin2);
    }

    int iRetCode = g_objOIDB.BatchGetRichFlagLevel(uiUin, vuiUinList, byServiceType, mRichFlagLevel, stContext);

    if (iRetCode == 0)
    {
        printf("RET_NUM=%d\n", static_cast<unsigned int>(mRichFlagLevel.size()));
        std::map<unsigned int, unsigned char>::iterator pCurRichLevel;

        for(pCurRichLevel = mRichFlagLevel.begin(); pCurRichLevel!=mRichFlagLevel.end(); pCurRichLevel++)
        {
            printf(" %d %d\n", pCurRichLevel->first, pCurRichLevel->second);
        }
    }
    else
    {
        printf("%d %s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}


void TestSetRichFlagLevel(int argc, char *argv[]){
	unsigned int uiUin = strtoul(argv[2], NULL, 10);
	unsigned char byServiceType = strtoul(argv[3], NULL, 10);
	unsigned char byLevel = strtoul(argv[4], NULL, 10);

	int iRetCode = g_objOIDB.SetRichFlagLevel(uiUin, byServiceType, byLevel);
	if(iRetCode == 0)
	{
		printf("SUCCESS\n");
	}
	else
	{
		printf("ERROR %d %s\n", iRetCode, g_objOIDB.GetErrMsg());
	}
}

void TestGetRichFlag2(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);
    unsigned char ucServiceID = atoi(argv[3]);
    unsigned char byFlag;
    int iRetCode = g_objOIDB.GetRichFlag2(uiUin, byFlag, ucServiceID);

    printf("ret: %d, %s\n", iRetCode, g_objOIDB.GetErrMsg());

    if (iRetCode == 0)
    {
        printf("uin: %u, flag: %u\n", uiUin, byFlag);
    }
}

void TestSetRichFlag2(int argc, char *argv[])
{
    unsigned int uiUin = strtoul(argv[2], NULL, 10);
    unsigned char ucServiceID = atoi(argv[3]);
    unsigned char byFlag = atoi(argv[4]);;
    int iRetCode = g_objOIDB.SetRichFlag2(uiUin, byFlag, ucServiceID);

    if (iRetCode == 0)
    {
        printf("uin: %u, flag: %u\n", uiUin, byFlag);
    }
    else
    {
        printf("ret: %d, %s\n", iRetCode, g_objOIDB.GetErrMsg());
    }
}

////////////////////////////////////////////////////////////

typedef struct tagCmdDef
{
	char szName[64];
	TestFunc pFunc;
	int iParamCount;
	char szParamsDesc[128];

} CMD_DEF;

const CMD_DEF astCmdDef[] =
{
	{"查好友", TestGetFriendList, 1, "uin"},
	{"查RichFlag", TestGetRichFlag, 2, "uin rich_flag"},
	{"查简单资料", TestGetSimpleInfo, 1, "uin"},
	{"批量查RichFlag", TestGetFriendsRichFlag, 3, "uin rich_flag uin1 uin2 ..."},
    {"批量查简单资料", TestBatchGetSimpleInfo, 1, "uin1 uin2 ..."},
    {"验证好友", TestCheckFriend, 2, "uin friend_uin"},
    {"查询单个增值位", TestGetMssFlag, 2, "uin mss_type"},
    {"批量查询单个增值位", TestBatchGetMssFlag, 2, "mss_type uin1 uin2 ..."},
//  {"查询所有增值位", TestGetMssFlagMulti, 1, "uin"},
    {"设置RichFlag", TestSetRichFlag, 3, "uin rich_flag rich_flag_val"},
    {"查询RichFlagLevel", TestGetRichFlagLevel, 2, "uin rich_flag"},
    {"设置RichFlagLevel", TestSetRichFlagLevel, 3, "uin rich_flag rich_flag_val"},
    {"设置单个增值位", TestSetMssFlag, 3, "uin mss_type mss_value"},
	{"查RichFlag2", TestGetRichFlag2, 2, "uin rich_flag"},
    {"设置RichFlag2", TestSetRichFlag2, 3, "uin rich_flag rich_flag_val"},
    {"带SESSION查好友", TestGetFriendListSession, 6, "uin appid skey_type skey clientip svrip"},
    {"带SESSION查简单资料", TestGetSimpleInfoSession, 6, "uin appid skey_type skey clientip svrip"},
    {"带SESSION查RichFlag", TestGetRichFlagSession, 7, "uin appid skey_type skey clientip svrip rich_flag"},
    {"带SESSION批量查RichFlag", TestBatchGetRichFlagSession, 8, "uin appid skey_type skey clientip svrip rich_flag uin1 uin2 ..."},
    {"带SESSION验证好友", TestCheckFriendSession, 7, "uin appid skey_type skey clientip svrip rich_flag friend_uin"},
    {"带SESSION查询单个增值位", TestGetMssFlagSession, 7, "uin appid skey_type skey clientip svrip mss_type"},
    {"带SESSION批量查询单个增值位", TestBatchGetMssFlagSession, 8, "uin appid skey_type skey clientip svrip mss_type uin1 uin2 ..."},
    {"带SESSION批量查简单资料", TestBatchGetSimpleInfoSession, 7, "uin appid skey_type skey clientip svrip uin1 uin2 ..."},
    {"带SESSION查询RichFlagLevel", TestGetRichFlagLevelSession, 7, "uin appid skey_type skey clientip svrip rich_flag"},
    {"带SESSION批量查询RichFlagLevel", TestBatchGetRichFlagLevelSession, 8, "uin appid skey_type skey clientip svrip rich_flag uin1 uin2 ..."},
};

void Usage(const char *pszAppName)
{
	printf("Usage:\n");
	unsigned int uiCmdCount = sizeof(astCmdDef)/sizeof(astCmdDef[0]);

	const CMD_DEF *p = astCmdDef;
	for (unsigned int i = 0; i < uiCmdCount; ++i, ++p)
	{
		printf("  %-12s: %s %d %s\n", pszAppName, p->szName, i + 1, p->szParamsDesc);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		Usage(argv[0]);
		return 0;
	}

	unsigned int uiCmdIndex = atoi(argv[1]);
	unsigned int uiCmdCount = sizeof(astCmdDef)/sizeof(astCmdDef[0]);

	if (uiCmdIndex < 1 || uiCmdIndex > uiCmdCount)
	{
		Usage(argv[0]);
		return 0;
	}

	const CMD_DEF *p = astCmdDef;
	p += uiCmdIndex - 1;

	if (p->iParamCount > (argc - 2))
	{
		Usage(argv[0]);
		return 0;
	}

	OpenPetLog("oidb_tool");

	int iRetCode = g_objOIDB.Init("./oidb_api.ini", "oidb_tool");
    if (iRetCode != 0)
    {
        printf("init failed: %s\n",g_objOIDB.GetErrMsg());
        return -1;
    }

	p->pFunc(argc, argv);

    return 0;
}


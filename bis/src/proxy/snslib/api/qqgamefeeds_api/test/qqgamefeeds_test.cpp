#include <stdio.h>
#include <stdlib.h>
#include "api/qqgamefeeds_api/qqgamefeeds_api.h"

using namespace snslib;

int main(int argc, char *argv[])
{
    if (argc < 6)
    {
        printf("usage: %s <uin> <appid> <access_token> <api_key> <content>\n", argv[0]);
        return -1;
    }

    int Ret = 0;
    CQQGameFeedsAPI objAPI;

    unsigned int uiUin = atoll(argv[1]);
    unsigned int uiAppID = atoll(argv[2]);
    const char *pszAccessToken = argv[3];
    const char *pszApiKey = argv[4];
    const char *pszContent = argv[5];

    Ret = objAPI.AddFeeds(uiUin, uiAppID, pszAccessToken, pszApiKey, pszContent);
    if (Ret != 0)
    {
        printf("add feeds failed, ret=%d, errmsg=%s\n", Ret, objAPI.GetErrMsg());
        return -1;
    }
    else
    {
        printf("add feeds succ\n");
    }

    return 0;
}

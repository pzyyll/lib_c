#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api/adopt_api/adopt_api.h"

using namespace petlib;

const int MAX_LINE                  = 1024;

typedef void(*TestFunc)(int argc, char *argv[]);

CAdoptAPI client;

void TestGetAll(int argc, char *argv[])
{
    unsigned int uiUin = 0;
    char szLine[MAX_LINE] = {0};
    SAdoptGetResp stResponse;
    int iRet = 0;

    while (fgets(szLine, MAX_LINE, stdin) != NULL)
    {
        if (sscanf(szLine, "%u", &uiUin) != 1)
            continue;
		iRet = client.GetAll(uiUin, stResponse);
		if(iRet == 0)
        {
			if (stResponse.byPetNum> 0)
			{
				for (unsigned int i = 0; i < stResponse.byPetNum; ++i)
				{
				    printf("%u %llu %u %s\n", uiUin, stResponse.aullPetID[i],
                        *(unsigned short *)((char *)&stResponse.aullPetID[i] + 4), stResponse.aszPetName[i]);
				}
			}
            else
			{
				printf("%u 0\n", uiUin);
			}
		}
		else
		{
			printf("%u -1 %s\n", uiUin, client.GetErrMsg());
		}
    }
}

void TestGetQQ(int argc, char *argv[])
{
    unsigned int uiUin = 0;
    char szLine[MAX_LINE] = {0};
    SAdoptGetResp stResponse;
    int iRet = 0;

    while (fgets(szLine, MAX_LINE, stdin) != NULL)
    {
        if (sscanf(szLine, "%u", &uiUin) != 1)
            continue;
		iRet = client.Get(uiUin, stResponse);
		if(iRet == 0)
        {
			if (stResponse.byPetNum> 0)
			{
				for (unsigned int i = 0; i < stResponse.byPetNum; ++i)
				{
				    printf("%u %llu %u %s\n", uiUin, stResponse.aullPetID[i],
                        *(unsigned short *)((char *)&stResponse.aullPetID[i] + 4), stResponse.aszPetName[i]);
				}
			}
            else
			{
				printf("%u 0\n", uiUin);
			}
		}
		else
		{
			printf("%u -1 %s\n", uiUin, client.GetErrMsg());
		}
    }
}

void TestGetPig(int argc, char *argv[])
{
    unsigned int uiUin = 0;
    char szLine[MAX_LINE] = {0};
    SAdoptGetResp stResponse;
    int iRet = 0;

    while (fgets(szLine, MAX_LINE, stdin) != NULL)
    {
        if (sscanf(szLine, "%u", &uiUin) != 1)
            continue;
        iRet = client.GetPig(uiUin, stResponse);
        if(iRet == 0)
        {
            if (stResponse.byPetNum> 0)
            {
                for (unsigned int i = 0; i < stResponse.byPetNum; ++i)
                {
                    printf("%u %llu %u %s\n", uiUin, stResponse.aullPetID[i],
                        *(unsigned short *)((char *)&stResponse.aullPetID[i] + 4), stResponse.aszPetName[i]);
                }
            }
            else
            {
                printf("%u 0\n", uiUin);
            }
        }
        else
        {
            printf("%u -1 %s\n", uiUin, client.GetErrMsg());
        }
    }
}

void TestDel(int argc, char *argv[])
{
    unsigned long long ullPetID = 0;
    unsigned int uiUin = 0;
    char szLine[MAX_LINE] = {0};
    int iRet = 0;

    int i = 0;
    while (fgets(szLine, MAX_LINE, stdin) != NULL)
    {
        if (sscanf(szLine, "%llu", &ullPetID) != 1)
            continue;

		uiUin = (unsigned int)ullPetID;
		iRet = client.Del(ullPetID);
		printf("%u %llu %d\n", uiUin, ullPetID, iRet);

        // 由于领养删除操作没有应答包，所以这里要控制一下速率
        i++;
        if (i >= 100)
        {
            usleep(300000);
            i = 0;
        }
    }
}

void TestSet(int argc, char *argv[])
{
    unsigned long long ullPetID = 0;
    char szPetName[32 + 1] = {0};
    unsigned int uiUin = 0;
    char szLine[MAX_LINE] = {0};
    int iRet = 0;

    while (fgets(szLine, MAX_LINE, stdin) != NULL)
    {
        if (sscanf(szLine, "%llu %s", &ullPetID, szPetName) != 2)
            continue;

		uiUin = (unsigned int)ullPetID;
		iRet = client.Set(ullPetID, szPetName);
		printf("%u %llu %s %d\n", uiUin, ullPetID, szPetName, iRet);
    }
}

typedef struct tagCmdDef
{
	char szName[64];
	TestFunc pFunc;
	int iParamCount;
	char szParamsDesc[128];
} CMD_DEF;

const CMD_DEF astCmdDef[] =
{
	{"查询所有", TestGetAll, 0, ""},
	{"查询企鹅", TestGetQQ, 0, ""},
 	{"删除领养", TestDel, 0, ""},
    {"设置领养", TestSet, 0, ""},
    {"查询猪猪", TestGetPig, 0, ""},
};

void Usage(const char *pszAppName)
{
	printf("Usage:\n\n");
    printf("从标准输入中读取数据，批量执行命令，每行数据格式如下:\n");
    printf("  查询: uin\n");
    printf("  删除: pet_id\n");
    printf("  设置: pet_id pet_name\n\n");
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

	if (p->iParamCount != (argc - 2))
	{
		Usage(argv[0]);
		return 0;
	}

	int ret = client.Init("./adopt_api.ini");
	if (ret < 0)
    {
		printf("接口初始化错误: %d, %s\n", ret, client.GetErrMsg());
		return 0;
	}

	p->pFunc(argc, argv);

	return 0;
}


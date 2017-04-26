#include <stdio.h>
#include <string.h>
#include "comm/hash_db/hash_db.h"
#include "comm/log/pet_log.h"
#include "comm/util/pet_util.h"

using namespace snslib;

void usage(const char *pszProgName)
{
    printf("usage: %s <conf_file> <operation> [other_args]...\n", pszProgName);
    printf(" operation: put/put_cat/put_keep/get/out/getall/outall\n");
    printf(" put/put_cat/put_keep <key> <value>\n");
    printf(" get/out <key>\n");
    printf(" getall/outall\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        usage(argv[0]);
        return -1;
    }

    OpenPetLog("hash_db_test");

    int iRetVal = 0;
    CHashDB objDB;

    const char *pszConfFile = argv[1];

    iRetVal = objDB.Init(pszConfFile);
    if (iRetVal != 0)
    {
        printf("db init failed, ret=%d, errmsg=%s\n", iRetVal, objDB.GetErrMsg());
        return -1;
    }

    if (strcmp(argv[2], "put") == 0)
    {
        if (argc < 5)
        {
            usage(argv[0]);
            return -1;
        }

        iRetVal = objDB.Put2(argv[3], argv[4]);
        if (iRetVal != 0)
        {
            printf("put failed, ret=%d, errmsg=%s\n", iRetVal, objDB.GetErrMsg());
            return -1;
        }
    }
    else if (strcmp(argv[2], "put_cat") == 0)
    {
        if (argc < 5)
        {
            usage(argv[0]);
            return -1;
        }

        iRetVal = objDB.PutCat2(argv[3], argv[4]);
        if (iRetVal != 0)
        {
            printf("put_cat failed, ret=%d, errmsg=%s\n", iRetVal, objDB.GetErrMsg());
            return -1;
        }
    }
    else if (strcmp(argv[2], "put_keep") == 0)
    {
        if (argc < 5)
        {
            usage(argv[0]);
            return -1;
        }

        iRetVal = objDB.PutKeep2(argv[3], argv[4]);
        if (iRetVal != 0)
        {
            printf("put_keep failed, ret=%d, errmsg=%s\n", iRetVal, objDB.GetErrMsg());
            return -1;
        }
    }
    else if (strcmp(argv[2], "get") == 0)
    {
        if (argc < 4)
        {
            usage(argv[0]);
            return -1;
        }

        char *pszVal = NULL;
        iRetVal = objDB.Get2(argv[3], &pszVal);
        if (iRetVal != 0)
        {
            printf("get failed, ret=%d, errmsg=%s\n", iRetVal, objDB.GetErrMsg());
            return -1;
        }

        printf("%s - %s\n", argv[3], pszVal);
        free(pszVal);
    }
    else if (strcmp(argv[2], "out") == 0)
    {
        if (argc < 4)
        {
            usage(argv[0]);
            return -1;
        }

        iRetVal = objDB.Out2(argv[3]);
        if (iRetVal != 0)
        {
            printf("out failed, ret=%d, errmsg=%s\n", iRetVal, objDB.GetErrMsg());
            return -1;
        }
    }
    else if (strcmp(argv[2], "getall") == 0)
    {
        iRetVal = objDB.IterInit();
        if (iRetVal != 0)
        {
            printf("iter init failed, ret=%d, errmsg=%s\n", iRetVal, objDB.GetErrMsg());
            return -1;
        }

        void *pszKey;
        int iKeyLen;
        void *pszVal;
        int iValLen;
        while((iRetVal = objDB.IterNext(&pszKey, &iKeyLen, &pszVal, &iValLen)) == 0)
        {
            ((char *)pszKey)[iKeyLen] = '\0';
            printf("%s - %s\n", (char *)pszKey, CStrTool::Str2Hex(pszVal, iValLen));
            free(pszKey);
            free(pszVal);
        }

        if (iRetVal != CHashDB::HDBE_NORECORD)
        {
            printf("iternext failed, ret=%d, errmsg=%s\n", iRetVal, objDB.GetErrMsg());
            return -1;
        }
    }
    else if (strcmp(argv[2], "outall") == 0)
    {
        iRetVal = objDB.VanishDB();

        if (iRetVal != 0)
        {
            printf("outall failed, ret=%d, errmsg=%s\n", iRetVal, objDB.GetErrMsg());
            return -1;
        }
    }
    else
    {
        usage(argv[0]);
        return -1;
    }

    printf("succ\n");

    return 0;
}

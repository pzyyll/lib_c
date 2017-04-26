#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comm/util/pet_util.h"

using namespace snslib;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: %s <nic_name>\n", argv[0]);
        return -1;
    }

    const char *pszNicAddr = NULL;

    pszNicAddr = CSysTool::GetNicAddr(argv[1]);

    if (pszNicAddr == NULL)
    {
        printf("get nic[%s] addr failed\n", argv[1]);
        return -1;
    }

    printf("ADDR[%s]:%s\n", argv[1], pszNicAddr);

    return 0;
}

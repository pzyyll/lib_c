#include <stdio.h>
#include "comm/log/pet_log.h"

int main(int argc, char *argv[])
{
    if (argc < 7)
    {
        printf("usage: %s <module_name> <log_level> <log_id> <pet_id> <log_content> <log_len> [loopop_count]\n", argv[0]);
        return -1;
    }
    int iLogLen = atoi(argv[6]);
    int iStrLen = strlen(argv[5]);
    if (iLogLen == 0)
    {
        iLogLen = iStrLen;
    }

    int iLoopCount = 1;
    if (argc > 7)
    {
        iLoopCount = atoi(argv[7]);
    }

    OpenPetLog(argv[1], 3, "/usr/local/pet50/conf");

    char szSendBuff[102400+1] = {0};

    if (iLogLen >= 102400)
    {
        iLogLen = 102400;
    }

    for(int i=0; i<iLogLen; i++)
    {
        szSendBuff[i] = argv[5][i%iStrLen];
    }


    for(int i=0; i<iLoopCount; i++)
    {
        PetLog(atoi(argv[3]), strtoull(argv[4], NULL, 10), atoi(argv[2]), "%d|%s", i, szSendBuff);
    }

    return 0;
}

#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

#include "api/tt_feeds_api/tt_feeds_api.h"
#include "comm/log/pet_log.h"
#include "comm/util/pet_util.h"

using namespace snslib;

bool bStop = false;

static void SigHandler(int iSigNo)
{
    bStop = true;
}

int FeedsTest(const char *pszConfFile, int iUserIDMin, int iUserIDMax, int iOpNum, int iMinLen, int iMaxLen, int iSpeedLimit)
{
    int iRetVal = 0;
    printf("PROC|%d|%d|%d|%d|%d|%d\n", getpid(), iUserIDMin, iUserIDMax, iOpNum, iMinLen, iMaxLen);

    CTTFeedsApi objFeedsAPI;
    iRetVal = objFeedsAPI.Init(pszConfFile);
    if (iRetVal != 0)
    {
        printf("init feeds_api failed\n");
        return -1;
    }

    char szFeedsBuff[8192];
    memset(szFeedsBuff, 'T', sizeof(szFeedsBuff));
    std::string sFeedsInfo;
    std::vector<std::string> vsAllFeedsInfo;
    std::vector<unsigned long long> vullAllFeedsID;

    int iReadNum = 0;
    int iWriteNum = 0;

    struct timeval stTimeBegin, stTimeEnd, stTimeDiff;

    gettimeofday(&stTimeBegin, NULL);

    struct timeval Time1, Time2, Time3, TimeDiff;
    gettimeofday(&Time1, NULL);

    struct timeval SpeedCtlStart = Time1, SpeedCtlNow, SpeedCtlDiff;
    Time3 = Time1;

    int FailNum = 0;

    int i=0;
    for(; i<iOpNum; i++)
    {
        if (bStop)
        {
            break;
        }

        while (true)
        {
            gettimeofday(&SpeedCtlNow, NULL);
            timersub(&SpeedCtlNow, &SpeedCtlStart, &SpeedCtlDiff);
            double Speed = (double)i/((double)SpeedCtlDiff.tv_sec + ((double)SpeedCtlDiff.tv_usec/1000000));

            if (Speed > (double)iSpeedLimit)
            {
                usleep(1000);
                continue;
            }

            break;
        }

        int iCurUserID = CRandomTool::Instance()->Get(iUserIDMin, iUserIDMax);
        int iOpCode = CRandomTool::Instance()->Get(0, 10);
        int iFeedsLen = CRandomTool::Instance()->Get(iMinLen, iMaxLen);
        if (iOpCode >= 7)
        {
            //模拟70%的读取，30%的写入
            sFeedsInfo.assign(szFeedsBuff, iFeedsLen);
            iRetVal = objFeedsAPI.AddFeeds(iCurUserID, sFeedsInfo);
            if (iRetVal != 0)
            {
                FailNum++;
            }
            sFeedsInfo.clear();
            iWriteNum++;
        }
        else
        {
            iRetVal = objFeedsAPI.GetAllFeeds(iCurUserID, vullAllFeedsID, vsAllFeedsInfo);
            if ((iRetVal != 0)&&(iRetVal != CTTFeedsApi::TT_FEEDS_ERR_NORECORED))
            {
                FailNum++;
            }
            vullAllFeedsID.clear();
            vsAllFeedsInfo.clear();
            iReadNum++;
        }

        gettimeofday(&Time2, NULL);
        timersub(&Time2, &Time3, &TimeDiff);
        if (TimeDiff.tv_sec >= 1)
        {
            timersub(&Time2, &Time1, &TimeDiff);
            double Speed = (double)i/((double)TimeDiff.tv_sec + ((double)TimeDiff.tv_usec/1000000));
            PetLog(0, 0, PETLOG_INFO, "%s|PID|%d|LOOP|%d|READ|%d|WRITE|%d|ERR|%d|ERR_PCT|%f|SPEED|%f", __FUNCTION__, getpid(), i, iReadNum, iWriteNum, FailNum, (double)FailNum/i, Speed);

            Time3 = Time2;
        }

    }

    gettimeofday(&stTimeEnd, NULL);

    timersub(&stTimeEnd, &stTimeBegin, &stTimeDiff);

    double Speed = (double)i/((double)TimeDiff.tv_sec + ((double)TimeDiff.tv_usec/1000000));
    PetLog(0, 0, PETLOG_INFO, "%s|PID|%d|LOOP|%d|READ|%d|WRITE|%d|ERR|%d|ERR_PCT|%f|SPEED|%f|END", __FUNCTION__, getpid(), i, iReadNum, iWriteNum, FailNum, (double)FailNum/i, Speed);

    return 0;
}


int main(int argc, char *argv[])
{
//    int iRetVal = 0;

    if (argc < 8)
    {
        printf("usage: %s <conf_file> <usr_num> <op_num> <proc_num> <min_len> <max_len> <speed_limit>\n", argv[0]);
        return -1;
    }

    OpenPetLog("tt_feeds_batch_test");

    const char *pszConfFile = argv[1];
    int iUserNum = atoi(argv[2]);
    int iOpNum = atoi(argv[3]);
    int iProcNum = atoi(argv[4]);
    int iMinLen = atoi(argv[5]);
    int iMaxLen = atoi(argv[6]);
    int iSpeedLimit = atoi(argv[7]);

    if ((iUserNum <=0)||(iOpNum <= 0)||(iProcNum <= 0)||(iMinLen<=0)||(iMaxLen<iMinLen)||(iProcNum > iUserNum)||(iProcNum > 1000)||(iMaxLen > 8000)||(iMinLen > 8000))
    {
        printf("param invalid, USERNUM:%d OPNUM:%d PROCNUM:%d MINLEN:%d MAXLEN:%d\n",iUserNum, iOpNum, iProcNum, iMinLen, iMaxLen);
        return -1;
    }

    printf("USERNUM:%d OPNUM:%d PROCNUM:%d MINLEN:%d MAXLEN:%d SPEEDLIMIT:%d\n",iUserNum, iOpNum, iProcNum, iMinLen, iMaxLen, iSpeedLimit);

    bStop = false;
    struct sigaction stSiga;
    memset(&stSiga, 0, sizeof(stSiga));
    stSiga.sa_handler = SigHandler;
    sigaction(SIGUSR1, &stSiga, NULL);
    sigaction(SIGTERM, &stSiga, NULL);
    sigaction(SIGINT, &stSiga, NULL);

    pid_t pidChild[1000];
    for(int i=0; i<iProcNum; i++)
    {
        pidChild[i] = fork();
        if (pidChild[i] == 0)
        {
            int iUserIDMin = ((iUserNum/iProcNum)*i);
            int iUserIDMax = ((iUserNum/iProcNum)*(i+1));

            FeedsTest(pszConfFile, iUserIDMin, iUserIDMax, iOpNum, iMinLen, iMaxLen, iSpeedLimit);

            return 0;
        }
    }

    for(int i=0; i<iProcNum; i++)
    {
        wait(NULL);
    }

    return 0;
}

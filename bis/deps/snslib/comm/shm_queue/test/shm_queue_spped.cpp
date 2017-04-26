/**
 * @file:   shm_queue_spped.cpp
 * @brief:  用来测试一下ShmQueue的性能，使用方法 ./shm_queue_spped  1 1 100000
 * 结论，1个发，1个收，不会出现队列满，或者队列空的情况，每秒吞吐量为30W/s
 * 在5个发，5个收的时候，就会出现队列满或者队列空的现象，在满的时候，发送进程休息200 microsecond，每秒的吞吐量为2W多，不到2W5
 * @author  shimmeryang@tencent.com
 * @date:   2013-09-26 23:39:22
 */


#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
// #include "log_def.h"
#include "comm/log/pet_log.h"
#include "comm/shm_queue/shm_queue.h"

void usage(const char *pszApp)
{
    printf("%s send_process_num recv_process_num Num\n", pszApp);
}

void Sender(snslib::CShmQueue &oShmQueue, int iNum)
{
    int iCount = 0;
    char szData[1024];
    memset(szData, 's', sizeof(szData));
    struct timeval tpstart, tpend;
    float timeuse;
    gettimeofday(&tpstart, NULL);
    for (int i = 0; i < iNum; i++)
    {
        int iRet = oShmQueue.InQueue(szData, sizeof(szData));
        if (iRet == snslib::CShmQueue::E_SHM_QUEUE_FULL)
        {
            PetLog(0, 0, PETLOG_WARN, "Queue full");
            usleep(100);
        }
        else if (iRet == 0)
        {
            // PetLog(0, 0, PETLOG_INFO, "InQueue succ");
            iCount++;
        }
        else
        {
            PetLog(0, 0, PETLOG_WARN, "InQueue failed, iRet = %d, ErrMsg = %s", iRet, oShmQueue.GetErrMsg());
        }
    }
    gettimeofday(&tpend, NULL);
    timeuse = (double)((tpend.tv_sec - tpstart.tv_sec) * 1000000 + (tpend.tv_usec - tpstart.tv_usec)) / 1000000;
    printf("Sender(%d) iCount = %d, use %f sec\n", getpid(), iCount, timeuse);
}

void Recver(snslib::CShmQueue &oShmQueue)
{
    int iLen;
    char szData[1024];
    int iNum = 0;
    int iRetryTimes = 0;
    struct timeval tpstart, tpend;
    float timeuse;
    gettimeofday(&tpstart, NULL);
    while (1)
    {
        int iRet = oShmQueue.OutQueue(szData, &iLen);
        if (iRet == snslib::CShmQueue::E_SHM_QUEUE_EMPTY)
        {
            PetLog(0, 0, PETLOG_WARN, "Queue empty");
            iRetryTimes++;
            if (iRetryTimes >= 10)
            {
                break;
            }
            usleep(100);
        }
        else if (iRet == 0)
        {
            // PetLog(0, 0, PETLOG_INFO, "OutQueue succ");
            iNum++;
            iRetryTimes = 0;
        }
        else
        {
            PetLog(0, 0, PETLOG_WARN, "OutQueue failed, iRet = %d, ErrMsg = %s", iRet, oShmQueue.GetErrMsg());
        }
    }

    gettimeofday(&tpend, NULL);
    timeuse = (double)((tpend.tv_sec - tpstart.tv_sec) * 1000000 + (tpend.tv_usec - tpstart.tv_usec)) / 1000000;
    printf("Recver(%d) iNum = %d, use %f sec\n", getpid(), iNum, timeuse);

}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    OpenPetLog(argv[0]);

    snslib::CShmQueue oShmQueue;
    int iSendProcNum = atoi(argv[1]);
    int iRecvProcNum = atoi(argv[2]);
    int iNum = atoi(argv[3]);

    int iRet = oShmQueue.Init(20130926, 102400000);
    if (iRet != 0)
    {
        printf("oShmQueue.Init failed, iRet = %d, ErrMsg = %s\n", iRet, oShmQueue.GetErrMsg());
        exit(EXIT_FAILURE);
    }

    // fork Sender Proc
    for (int i = 0; i < iSendProcNum; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            printf("Sender fork failed\n");
            continue;
        }
        else if (pid == 0)
        {
            printf("Sender create succ\n");
            Sender(oShmQueue, iNum);
            exit(EXIT_SUCCESS);
        }
    }

    // fork Recver Proc
    for (int i = 0; i < iRecvProcNum; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            printf("Recver fork failed\n");
            continue;
        }
        else if (pid == 0)
        {
            printf("Recver create succ\n");
            Recver(oShmQueue);
            exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < iSendProcNum + iRecvProcNum; i++)
    {
        int status;
        pid_t pid = wait(&status);
        printf("process(%d) end with %d\n", pid, status);
    }


    exit(EXIT_SUCCESS);
}

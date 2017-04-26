#ifndef _SHM_QUEUE_H_
#define _SHM_QUEUE_H_

#include    "comm/sem_lock/sem_lock.h"

namespace snslib
{

//在链表尾部加入识别码，当出现数据损坏时靠这个识别下一条记录
const char TAIL_FLAG = 0xAA;

class CShmQueue
{
public:
    const static int SUCCESS = 0;
    const static int ERROR = -1;

    const static int E_SHM_QUEUE_SEMLOCK = -601;
    const static int E_SHM_QUEUE_AT_SHM = -602;
    const static int E_SHM_QUEUE_GET_SHM = -603;
    const static int E_SHM_QUEUE_FULL = -604;
    const static int E_SHM_QUEUE_EMPTY = -605;
    const static int E_SHM_QUEUE_BUFFLIMIT = -606;

    typedef struct tagQueueHead
    {
        int iLen;  //Queue的总长度
        int iHead; //数据头指针
        int iTail; //数据尾指针
        int iNum;  //数据块个数
    } QueueHead;

    typedef struct tagBlockHead
    {
        int iIndex;
        // 2013年12月12日 21:44:26
        // shimmeryang: short最大只有32K，不够用啊，调节为int型吧
        int iLen;
    } BlockHead;

public:
    CShmQueue();
    virtual ~CShmQueue();

    int Init(const int aiShmKey, const int aiSize);

    // 插入数据
    int InQueue(const char * pData, int iDataLen);

    // 获取数据，数据长度会保存在iBufLen
    int OutQueue(char * pBuf, int * iBufLen);

    // 头部插入数据
    int HeaderInQueue(const char * pData, int iDataLen);

    // 获取Queue中记录条数
    int GetNum();

    const QueueHead * GetHead();

    const char *GetErrMsg()
    {
        return m_szErrMsg;
    }

protected:
    char *m_pMem;
    QueueHead * m_pHead;
    CSemLock * m_pobjSemLock;
    unsigned int m_iQueueSize;

    char m_szErrMsg[256];
};

}
#endif


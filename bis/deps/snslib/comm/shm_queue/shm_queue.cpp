#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

#include "shm_queue.h"
#include "comm/log/pet_log.h"

using namespace snslib;

CShmQueue::CShmQueue()
{
    m_pMem = NULL;
    m_pHead = NULL;
    m_pobjSemLock = NULL;
    m_iQueueSize = 0;

    memset(m_szErrMsg, 0x0, sizeof(m_szErrMsg));
}

CShmQueue::~CShmQueue()
{
}

int CShmQueue::Init(const int aiKey, const int aiSize)
{
    m_iQueueSize = aiSize;

    /// create sem.
    m_pobjSemLock = new CSemLock();
    int iRetVal = m_pobjSemLock->Init(aiKey);
    if (iRetVal != SUCCESS){
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "sem_lock init failed, key=%d, ret=%d", aiKey, iRetVal);
        return E_SHM_QUEUE_SEMLOCK;
    }

    /// create shm.
    int iHandle = 0, iExist = 0;
    if ((iHandle = shmget(aiKey, aiSize + sizeof(QueueHead), 0666 | IPC_CREAT | IPC_EXCL )) < 0){
        if (errno== EEXIST){
            iExist = 1;

            if ((iHandle = shmget(aiKey, aiSize, 0666)) != -1) {
                m_pMem = (char *) shmat(iHandle, NULL, 0);
                if (m_pMem == NULL){
                    snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shmget exist, but attach failed, key=%d, size=%d", aiKey, aiSize);
                    return E_SHM_QUEUE_AT_SHM;
                }
            }else{
                snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shmget create exist, but shmget failed, key=%d, size=%d, ret=%d", aiKey, aiSize, iHandle);
                return E_SHM_QUEUE_GET_SHM;
            }
        }else{
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shmget create failed, key=%d, size=%d, ret=%d", aiKey, aiSize, iHandle);
            return E_SHM_QUEUE_GET_SHM;
        }
    }else{
        iExist = 0;
        m_pMem = (char *) shmat(iHandle, NULL, 0);
        if (m_pMem == NULL){
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm attach failed, handle=%d, key=%d, size=%d", iHandle, aiKey, aiSize);
            return E_SHM_QUEUE_AT_SHM;
        }
    }

    //printf ("liExist = %d \n", liExist);
    if (iExist == 0){
        /// first time create it.
        memset(m_pMem, 0x0, aiSize + sizeof(QueueHead));
        m_pHead = (QueueHead *) (m_pMem);
        m_pHead->iLen = aiSize;
        m_pHead->iHead = 0;
        m_pHead->iTail = 0;
        m_pHead->iNum = 0;
    } else if (iExist == 1){
        /// shm exist .
        m_pHead = (QueueHead *) (m_pMem);
    }

    m_pMem += sizeof(QueueHead);

    return SUCCESS;
}

int CShmQueue::InQueue(const char * pData, int iDataLen)
{
    int liRet = 0;
    char szTail[6];
    memset(szTail, 0xAA, sizeof(szTail));

    // 2013��12��12�� 21:03:34
    // shimmeryang�� 32k̫С�ˣ���������ȡqqface��ʹ�ð�
    // ���ڰ�block�ĳ��ȵ���Ϊint������Ҫ�ж�����ĳ����ˡ�
    // if (iDataLen >= 131072)
    // {
    //     snprintf(m_szErrMsg, sizeof(m_szErrMsg), "in quuee buffer too large %d", iDataLen);
    //     return -1;
    // }

    m_pobjSemLock->Lock();

    unsigned int liFreeSpace = 0;

    if (m_pHead->iHead < m_pHead->iTail || ((m_pHead->iHead == m_pHead->iTail) && (m_pHead->iNum == 0))){
        //ʣ��ռ�
        liFreeSpace = m_pHead->iLen - (m_pHead->iTail - m_pHead->iHead);
    }else if (m_pHead->iHead > m_pHead->iTail){
        liFreeSpace = m_pHead->iHead - m_pHead->iTail;
    }else{
        liFreeSpace = 0;
    }

    if (liFreeSpace < iDataLen + sizeof(BlockHead) + sizeof(szTail)){
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm queue full|%u|%d|%zu|%zu|||%d", liFreeSpace, iDataLen, sizeof(BlockHead), sizeof(szTail),
        		m_pHead->iHead, m_pHead->iTail, m_pHead->iNum, m_pHead->iLen);
        liRet = E_SHM_QUEUE_FULL;
    }else{
        unsigned int liIndex = m_pHead->iTail;
        unsigned int liLeft = m_pHead->iLen - m_pHead->iTail;

        if ((m_pHead->iHead > m_pHead->iTail) || (liLeft >= (iDataLen + sizeof(BlockHead) + sizeof(szTail)))){
            //// block.
            liIndex = m_pHead->iTail;
            BlockHead stHead;
            stHead.iIndex = liIndex;
            stHead.iLen = iDataLen;
            memcpy(&(m_pMem[liIndex]), (char *) &stHead, sizeof(stHead));
            liIndex += sizeof(stHead);
            memcpy(&(m_pMem[liIndex]), pData, iDataLen);
            liIndex += iDataLen;
            memcpy(&(m_pMem[liIndex]), szTail, sizeof(szTail));

            m_pHead->iTail = (m_pHead->iTail + iDataLen + sizeof(BlockHead) + sizeof(szTail)) % m_pHead->iLen;
            m_pHead->iNum++;
        }else{
            /// split or block.
            BlockHead stHead;
            stHead.iIndex = m_pHead->iTail;
            stHead.iLen = iDataLen;

            ///< set block_head.
            if (liLeft >= sizeof(BlockHead)){
                memcpy(&(m_pMem[liIndex]), (char *) &stHead, sizeof(stHead));
                liIndex = (liIndex + sizeof(stHead)) % m_pHead->iLen;

                liLeft = liLeft - sizeof(BlockHead);
            }else{
                memcpy(&(m_pMem[liIndex]), (char *) &stHead, liLeft);
                liIndex = 0;
                memcpy(&(m_pMem[liIndex]), ((char *) &stHead) + liLeft, sizeof(BlockHead) - liLeft);
                liIndex = (liIndex + sizeof(BlockHead) - liLeft) % m_pHead->iLen;

                liLeft = m_pHead->iHead - liIndex;
            }

            /// set data.
            if ((int)liLeft >= iDataLen){
                memcpy(&(m_pMem[liIndex]), pData, iDataLen);

                liIndex = (liIndex + iDataLen) % m_pHead->iLen;
                liLeft = liLeft - iDataLen;
            }else{
                memcpy(&(m_pMem[liIndex]), pData, liLeft);
                liIndex = 0;
                memcpy(&(m_pMem[liIndex]), pData + liLeft, iDataLen - liLeft);

                liIndex = (liIndex + iDataLen - liLeft) % m_pHead->iLen;
                liLeft = m_pHead->iHead - liIndex;
            }

            // set tail flag
            if (liLeft >= sizeof(szTail)){
                memcpy(&(m_pMem[liIndex]), szTail, sizeof(szTail));
            }else{
                memcpy(&(m_pMem[liIndex]), szTail, liLeft);
                liIndex = 0;
                memcpy(&(m_pMem[liIndex]), szTail + liLeft, sizeof(szTail) - liLeft);
            }

            m_pHead->iTail = (m_pHead->iTail + iDataLen + sizeof(BlockHead) + sizeof(szTail)) % m_pHead->iLen;
            m_pHead->iNum++;
        }
    }

    m_pobjSemLock->Release();

    return liRet;
}


int CShmQueue::HeaderInQueue(const char * pData, int iDataLen)
{
    int liRet = 0;
    char szTail[6];
    memset(szTail, 0xAA, sizeof(szTail));

    m_pobjSemLock->Lock();

    unsigned int liFreeSpace = 0;

    if (m_pHead->iHead < m_pHead->iTail || ((m_pHead->iHead == m_pHead->iTail) && (m_pHead->iNum == 0))){
        //ʣ��ռ�
        liFreeSpace = m_pHead->iLen - (m_pHead->iTail - m_pHead->iHead);
    }else if (m_pHead->iHead > m_pHead->iTail){
        liFreeSpace = m_pHead->iHead - m_pHead->iTail;
    }else{
        liFreeSpace = 0;
    }

    if (liFreeSpace < iDataLen + sizeof(BlockHead) + sizeof(szTail)){
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm queue full");
        liRet = E_SHM_QUEUE_FULL;
    } else {
        int liIndex = 0, liHeadIndex = 0;
        int liSplitFlag = 0;
        if (m_pHead->iTail >= m_pHead->iHead){
            if (m_pHead->iHead >= (int)(iDataLen + sizeof(BlockHead) + sizeof(szTail))) {
                liIndex = m_pHead->iHead - (iDataLen + sizeof(BlockHead) + sizeof(szTail));
                liSplitFlag = 0;
            } else {
                liIndex = m_pHead->iLen - (iDataLen + sizeof(BlockHead) + sizeof(szTail) - m_pHead->iHead);
                liSplitFlag = 1;
            }
        } else {
            liIndex = m_pHead->iHead - (iDataLen + sizeof(BlockHead) + sizeof(szTail));
            liSplitFlag = 0;
        }

        liHeadIndex = liIndex;

        if (liSplitFlag == 0){
            //// block.
            BlockHead stHead;
            stHead.iIndex = liIndex;
            stHead.iLen = iDataLen;
            memcpy(&(m_pMem[liIndex]), (char *) &stHead, sizeof(stHead));
            liIndex += sizeof(stHead);
            memcpy(&(m_pMem[liIndex]), pData, iDataLen);
            liIndex += iDataLen;
            memcpy(&(m_pMem[liIndex]), szTail, sizeof(szTail));
        } else {
            /// split or block.
            BlockHead stHead;
            stHead.iIndex = liIndex;
            stHead.iLen = iDataLen;

            ///< set block_head.
            if (((liIndex > m_pHead->iHead) && (m_pHead->iLen - liIndex) >= (int)sizeof(BlockHead))||(liIndex < m_pHead->iHead)) {
                memcpy(&(m_pMem[liIndex]), (char *) &stHead, sizeof(stHead));
                liIndex = liIndex + sizeof(stHead);
            }else{
                int iSplitLen = m_pHead->iLen - liIndex;
                memcpy(&(m_pMem[liIndex]), (char *) &stHead, iSplitLen);
                liIndex = 0;
                memcpy(&(m_pMem[liIndex]), ((char *) &stHead) + iSplitLen, sizeof(BlockHead) - iSplitLen);
                liIndex = liIndex + sizeof(BlockHead) - iSplitLen;
            }

            /// set data.
            if (((liIndex > m_pHead->iHead) && (m_pHead->iLen - liIndex) >= iDataLen)||(liIndex < m_pHead->iHead)) {
                memcpy(&(m_pMem[liIndex]), pData, iDataLen);

                liIndex = liIndex + iDataLen;
            } else {
                int iSplitLen = m_pHead->iLen - liIndex;
                memcpy(&(m_pMem[liIndex]), pData, iSplitLen);
                liIndex = 0;
                memcpy(&(m_pMem[liIndex]), pData + iSplitLen, iDataLen - iSplitLen);

                liIndex = liIndex + iDataLen - iSplitLen;
            }

            // set tail flag
            if (((liIndex > m_pHead->iHead) && (m_pHead->iLen - liIndex) >= (int)sizeof(szTail))||(liIndex < m_pHead->iHead)) {
                memcpy(&(m_pMem[liIndex]), szTail, sizeof(szTail));
            } else {
                int iSplitLen = m_pHead->iLen - liIndex;
                memcpy(&(m_pMem[liIndex]), szTail, iSplitLen);
                liIndex = 0;
                memcpy(&(m_pMem[liIndex]), szTail + iSplitLen, sizeof(szTail) - iSplitLen);
            }

        }
        m_pHead->iNum++;
        m_pHead->iHead = liHeadIndex;
    }

    m_pobjSemLock->Release();

    return liRet;
}

int CShmQueue::OutQueue(char * pBuf, int * iBufLen)
{
    int liRet = 0;

    m_pobjSemLock->Lock();

    if (m_pHead->iNum > 0 || m_pHead->iHead != m_pHead->iTail){
        /// process.
        int liIndex = m_pHead->iHead;
        int liLeft = m_pHead->iLen - m_pHead->iHead;
        BlockHead stHead;
        memset(&stHead, 0x0, sizeof(stHead));

        if (liLeft < (int)sizeof(BlockHead)){
            /// block head split.
            memcpy((char *) &stHead, &(m_pMem[liIndex]), liLeft);
            liIndex = 0;
            memcpy(((char *) &stHead) + liLeft, &(m_pMem[liIndex]), sizeof(BlockHead) - liLeft);
            liIndex = (liIndex + sizeof(BlockHead) - liLeft) % m_pHead->iLen;
        }else{
            memcpy(&stHead, &(m_pMem[liIndex]), sizeof(stHead));
            liIndex = (liIndex + sizeof(BlockHead)) % m_pHead->iLen;
        }

        if (*iBufLen < stHead.iLen)
        {
        	*iBufLen = stHead.iLen;
			m_pobjSemLock->Release();
        	return E_SHM_QUEUE_BUFFLIMIT;
        }

        // check
        if ((*iBufLen < stHead.iLen) || (stHead.iIndex != m_pHead->iHead) || (pBuf == NULL))
        {
            PetLog(0, 0, PETLOG_ERR, "ShmQueue|buflen|%d|headlen|%d|headidx|%d|head|%d|left|%d|headsize|%ld|num|%d|searching next head ...", *iBufLen, stHead.iLen, stHead.iIndex, m_pHead->iHead, liLeft, sizeof(BlockHead), m_pHead->iNum);

            if (m_pHead->iNum > 0)
            {
                //searching flag data
                int iFlagDataNum = 0;
                while (1)
                {
                    if (m_pMem[liIndex] == TAIL_FLAG)
                    {
                        iFlagDataNum++;
                    }
                    else
                    {
                        iFlagDataNum = 0;
                    }

                    liIndex++;
                    liIndex = liIndex % m_pHead->iLen;

                    if (iFlagDataNum == 6)
                    {
                        m_pHead->iHead = liIndex;
                        m_pHead->iNum--;
                        PetLog(0, 0, PETLOG_ERR, "ShmQueue|Relocate Head = %d, NUM = %d", liIndex, m_pHead->iNum);
                        break;
                    }
                }
            }
            else
            {
                //��ʼ��
                m_pHead->iLen = m_iQueueSize;
                m_pHead->iHead = 0;
                m_pHead->iTail = 0;
                m_pHead->iNum = 0;

                PetLog(0, 0, PETLOG_ERR, "ShmQueue|ALL Data has been Initialize to ZERO ...");
            }

            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm queue empty");
            liRet = E_SHM_QUEUE_EMPTY;
        } else {
        	if (stHead.iLen < 0)
        	{
                    //��ʼ��
                    m_pHead->iLen = m_iQueueSize;
                    m_pHead->iHead = 0;
                    m_pHead->iTail = 0;
                    m_pHead->iNum = 0;

                    PetLog(0, 0, PETLOG_ERR, "ShmQueue|ALL Data has been Initialize to ZERO ...");

                    snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm queue empty");
                    liRet = E_SHM_QUEUE_EMPTY;
        	}
        	else
        	{
				*iBufLen = stHead.iLen;

				///set data.
				liLeft = m_pHead->iLen - liIndex;
				if (liLeft < stHead.iLen)
				{
					memcpy(pBuf, &(m_pMem[liIndex]), liLeft);
					liIndex = 0;
					memcpy(pBuf + liLeft, &(m_pMem[liIndex]), stHead.iLen - liLeft);

					liIndex = (liIndex + stHead.iLen - liLeft) % m_pHead->iLen;
					liLeft = m_pHead->iLen - liIndex;
				}
				else
				{
					memcpy(pBuf, &(m_pMem[liIndex]), stHead.iLen);

					liIndex = (liIndex + stHead.iLen) % m_pHead->iLen;
					liLeft = liLeft - stHead.iLen;
				}

				// verify tail
				char szTail[6];
				memset(szTail, 0xAA, sizeof(szTail));
				char szGetData[6];
				memset(szGetData, 0x0, sizeof(szGetData));

				if (liLeft >= (int)sizeof(szGetData))
				{
					memcpy(szGetData, &(m_pMem[liIndex]), sizeof(szGetData));
				}
				else
				{
					memcpy(szGetData, &(m_pMem[liIndex]), liLeft);
					liIndex = 0;
					memcpy(szGetData + liLeft, &(m_pMem[liIndex]), sizeof(szGetData) - liLeft);
				}

				m_pHead->iHead = (m_pHead->iHead + stHead.iLen + sizeof(BlockHead) + sizeof(szTail)) % m_pHead->iLen;
				m_pHead->iNum--;

				if (memcmp(szGetData, szTail, sizeof(szTail)) != 0)
				{
					PetLog(0, 0, PETLOG_ERR, "flag not match, FLAG:%d|%d|%d|%d|%d|%d, tail:%d|%d|%d|%d|%d|%d", szGetData[0], szGetData[1], szGetData[2], szGetData[3], szGetData[4], szGetData[5], szTail[0], szTail[1], szTail[2], szTail[3], szTail[4], szTail[5]);
					liRet = ERROR;
				}
        	}
        }
    }
    else
    {
        liRet = E_SHM_QUEUE_EMPTY;
    }

    m_pobjSemLock->Release();

    return liRet;
}

int CShmQueue::GetNum()
{
    int liNum = 0;

    m_pobjSemLock->Lock();

    liNum = m_pHead->iNum;

    m_pobjSemLock->Release();

    return liNum;
}

const CShmQueue::QueueHead * CShmQueue::GetHead()
{
    return m_pHead;
}


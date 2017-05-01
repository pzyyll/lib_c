#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "share_mem.h"

using namespace snslib;

CShareMem::CShareMem()
{
}

CShareMem::~CShareMem()
{

}

int CShareMem::Create(key_t tKey, int iSize, int iMode)
{
    m_tShmKey = tKey;
    m_iShmSize = iSize;

    //create share mem
    if ((m_iShmId = shmget(m_tShmKey, m_iShmSize, IPC_CREAT| IPC_EXCL | iMode)) < 0) //try to create
    {
        if (errno!= EEXIST)
        {
            return ERROR;
        }
        //exist,get
        if ((m_iShmId = shmget(m_tShmKey, m_iShmSize, iMode)) < 0)
        {
            return ERROR;
        }

        return SHM_EXIST;
    }
    else
    {
        return SUCCESS;
    }
}

int CShareMem::Get(key_t tKey, int iSize, int iMode)
{
    m_tShmKey = tKey;
    m_iShmSize = iSize;

    //exist,get
    if ((m_iShmId = shmget(tKey, iSize, iMode)) < 0)
    {
        return ERROR;
    }
    else
    {
        return SUCCESS;
    }
}

int CShareMem::Attach()
{
    if ((m_pvMem = shmat(m_iShmId, NULL, 0)) == (void *)-1)
        return ERROR;
    else
        return SUCCESS;
}

int CShareMem::Detach()
{
    if (shmdt(m_pvMem) != 0)
        return ERROR;
    else
        return SUCCESS;
}

int CShareMem::Remove()
{
    if (shmctl(m_iShmId, IPC_RMID, NULL) < 0)
        return ERROR;
    else
        return 0;
}


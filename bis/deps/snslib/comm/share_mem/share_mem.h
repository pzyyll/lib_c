/**
 * @file    share_mem.h
 * @brief   共享内存管理类
 * @author  jamieli@tencent.com
 * @date    2008-08-12
 */
#ifndef _SHARE_MEM_H_
#define _SHARE_MEM_H_

#include <sys/ipc.h>
#include <sys/shm.h>

namespace snslib
{

class CShareMem
{
public:
    const static int SHM_EXIST = 1;

    const static int SUCCESS = 0;
    const static int ERROR = -1;


public:
	CShareMem();
	virtual ~CShareMem();

	int Get(key_t tKey,int iSize, int iMode = 0644);
	int Create(key_t tKey,int iSize,int iMode = 0644);
	int Attach();
	int Detach();
	int Remove();

    void *GetMem()
    {
        return m_pvMem;
    }

protected:
	key_t m_tShmKey;	//share memory key
	int m_iShmSize;		//share memory size
	int m_iShmId;		//share memory id
	void* m_pvMem;		//point to share memory
};

}
#endif



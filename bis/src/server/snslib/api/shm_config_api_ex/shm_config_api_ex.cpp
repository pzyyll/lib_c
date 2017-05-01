/*
 * shm_config_api_ex.cpp
 *
 *  Created on: 2010-8-20
 *      Author: peterfang
 */

#include "shm_config_api_ex.h"

using namespace snslib;
using namespace std;

CShmConfigApiEx::CShmConfigApiEx()
{
    // TODO Auto-generated constructor stub

}

CShmConfigApiEx::~CShmConfigApiEx()
{
    // TODO Auto-generated destructor stub
}

int CShmConfigApiEx::initShmMem(const char* pszShmConf)
{
    bool bCreateShmFlag = true;

    //共享内存操作相关
    int iShmKey = 30000;
    int iShmSize = 120 * 1024 * 1024;
    int iRetVal = 0;

    //读取共享内存配置文件
    CIniFile objShmIniFile( pszShmConf );

    if (objShmIniFile.IsValid())
    {
        objShmIniFile.GetInt("SHM", "ShmKey", 0, &iShmKey);
        //objShmIniFile.GetInt("SHM", "ShmSize", 0, &iShmSize);
    }

    CShareMem objShareMem;
    iRetVal = objShareMem.Create(iShmKey, iShmSize);
    if (iRetVal == objShareMem.SHM_EXIST)
    {
        //共享内存已经存在，暂时不做处理
        bCreateShmFlag = false;
        printf("shm is exist!\n");
    }
    else if (iRetVal != 0)
    {
        //创建共享内存失败
        printf("create shm failed, key=%d, size=%d, ret=%d", iShmKey, iShmSize, iRetVal);
        return -1;
    }
    else
    {}

    iRetVal = objShareMem.Attach();
    if (iRetVal != 0)
    {
        printf("attach shm failed, key=%d, size=%d, ret=%d", iShmKey, iShmSize, iRetVal);
        return -1;
    }

    g_pvMem = objShareMem.GetMem();

    return 0;
}

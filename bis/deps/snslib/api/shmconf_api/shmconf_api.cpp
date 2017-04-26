/**
 * @file    load_tool_peter.cpp
 * @brief   数据库信息加载工具,将数据库信息加载到共享内存当中去
 * @author  peterfang@tencent.com
 * @date    2010-08-19
 */
#include "shmconf_api.h"
#include <sys/shm.h>
#include <vector>
#include <string>
#include "comm/util/pet_util.h"

using namespace snslib;
using namespace std;

CShmConfApi::CShmConfApi()
{
}

CShmConfApi::~CShmConfApi()
{
}

int CShmConfApi::Init()
{
   //共享内存操作相关
	int iShmKey = SHMCONF_KEY;
	int iShmSize = SHMCONF_TOTAL_LENGTH;
	int iRetVal = 0;

	CShareMem objShareMem;
	iRetVal = objShareMem.Create(iShmKey, iShmSize, 0666&(~IPC_CREAT));
	if (iRetVal != objShareMem.SHM_EXIST){
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "shm not existed, errmsg=%s", strerror(errno) );
		return -1;
	}

	iRetVal = objShareMem.Attach();
	if (iRetVal != 0){
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "attach shm failed, key=%d, size=%d, ret=%d", iShmKey, iShmSize, iRetVal );
		return -2;
	}

	m_pszShmConf = (char *)objShareMem.GetMem();

	return 0;
}

int CShmConfApi::GetRocord(unsigned int uiID, char ** pszData, int * piLength )
{
	char * pszReadPos = *(int *)m_pszShmConf != 0 ? (m_pszShmConf + SHMCONF_DATA1_OFFSET ) : (m_pszShmConf + SHMCONF_DATA0_OFFSET);

	// 从table查找id
	char * pszTablePos = pszReadPos;
	SHMCONF_TABLE_INFO * pstTable;
	int iOff = sizeof(int);
	int iFoundFlag = 0;
	pstTable = (SHMCONF_TABLE_INFO *)(pszTablePos+iOff);
	while(pstTable->ID){
		if(pstTable->ID == uiID){
			iFoundFlag = 1;
			break;
		}
		iOff += sizeof(SHMCONF_TABLE_INFO);
		pstTable = (SHMCONF_TABLE_INFO *)(pszTablePos+iOff);
	}

	if(!iFoundFlag){
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "can't found id=%u", uiID );
		return -1;
	}

	// 读取数据
	*pszData = m_pszShmConf + pstTable->offset;
	*piLength = pstTable->data_size;
	
	//printf("%s|data=[%d]%s!", __func__, *piLength, snslib::CStrTool::Str2Hex(*pszData, *piLength));

	return 0;
}


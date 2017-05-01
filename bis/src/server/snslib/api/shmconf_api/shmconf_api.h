/**
 * @file    load_tool_peter.h
 * @brief   数据库信息加载工具,将数据库信息加载到共享内存当中去
 * @author  peterfang@tencent.com
 * @date    2010-08-19
 */
#ifndef __SHMCONF_API_H__
#define __SHMCONF_API_H__

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "comm/share_mem/share_mem.h"

namespace snslib
{

const static unsigned int SHMCONF_KEY = 0x550001;
const static unsigned int SHMCONF_HEAD_LENGTH = 64;
const static unsigned int SHMCONF_TABLE_LENGTH = 4*1024*1024;
const static unsigned int SHMCONF_DATA_LENGTH = 64*1024*1024;
const static unsigned int SHMCONF_BLOCK_SIZE =  SHMCONF_TABLE_LENGTH + SHMCONF_DATA_LENGTH;
const static unsigned int SHMCONF_DATA0_OFFSET = SHMCONF_HEAD_LENGTH ;
const static unsigned int SHMCONF_DATA1_OFFSET = SHMCONF_HEAD_LENGTH + SHMCONF_BLOCK_SIZE;
const static unsigned int SHMCONF_TOTAL_LENGTH = SHMCONF_HEAD_LENGTH + 2*(SHMCONF_BLOCK_SIZE);

//该结构标明了每条记录数据所存放的位置等信息，以便更好的定位和访问
typedef struct{
    unsigned int ID;          //某个数据段所对应的内存Id号
    unsigned int offset;      //该数据段的偏移量
    unsigned int data_size;   //该数据段的大小
}SHMCONF_TABLE_INFO;

//定义HEAD格式
typedef struct{
    int iUseFlag;             //使用标志，0表示data0可以使用，1表示data1可以使用
}SHMCONF_MEM_HEAD;

//数据库基本配置信息
typedef struct
{
    int  DB_Port;
    char DB_Host[64];
    char DB_User[64];
    char DB_Pass[64];
    char DB_Name[64];
    char DB_Table_Name[128];
}DB_INFO;

class CShmConfApi
{
public:
	CShmConfApi();
	~CShmConfApi();

public:
	int Init();
	int GetRocord(unsigned int uiID, char ** pszData, int * piLength );
	char * GetErrMsg(){
		return m_szErrMsg;
	}
private:
	char m_szErrMsg[255];
	char * m_pszShmConf;
};

}

#endif //__SHMCONF_API_H__

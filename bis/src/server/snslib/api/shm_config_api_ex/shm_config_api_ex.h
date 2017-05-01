/*
 * @   Filename：  shm_config_api_ex.h
 * @        dsc：  用户可以使用getBuffById(),来得到共享内存中相应的数据，用户只需输入Mem首地址， 且输入ID号，即可得到ID所对应的共享内存数据
 * @ Created on: 2010-8-20
 * @     Author: peterfang@tencent.com
 */

#ifndef SHM_CONFIG_API_EX_H_
#define SHM_CONFIG_API_EX_H_

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "comm/util/pet_util.h"
#include "comm/ini_file/ini_file.h"
#include "comm/share_mem/share_mem.h"
//#include "../parse_speed_test.pb.h"
//#include "../hf_db_user_info.pb.h"
#include <google/protobuf/message_lite.h>

using namespace std;

namespace snslib
{
#define O2P(offset) ((char*)g_pvMem+offset)  //计算得到对应偏移量位置的指针

//以下情况是针对共享内存大小为120*1024*1024的情况
const static unsigned int HEAD_LENGTH =  100;
const static unsigned int TABLE_LENGTH = 2*1024*1024;
const static unsigned int DATA1_OFFSET = 60*1024*1024;

//该结构标明了每条记录数据所存放的位置等信息，以便更好的定位和访问
typedef struct
{
    unsigned int ID;          //某个数据段所对应的内存Id号
    unsigned int offset;      //该数据段的偏移量
    unsigned int data_size;   //该数据段的大小
    char Message_name[50];    //该数据段的名字，对应一种消息类型
}TABLE_INFO;

//数据库基本配置信息
typedef struct
{
    int  DB_Id;
    int  DB_Port;
    char DB_Host[50];
    char DB_User[50];
    char DB_Pass[50];
    char DB_Name[50];
    char DB_Table_Name[50];
}DB_INFO;

class CShmConfigApiEx 
{
public:
    CShmConfigApiEx();

    int initShmMem(const char* pszConf);               //初始化共享内存

    template <typename T>
    int GetRecordList(uint32_t uiMessageID, T& objRecords)
    {
        g_pHeadMem = (TABLE_INFO*) Head();

        uint32_t uiIdNumber = 0;
        uint32_t uiFlag = *((uint32_t*) Head());

        TABLE_INFO* g_pTOfDataMem;

        if (uiFlag == 0)
        {
            printf("uiFlag == 0\n");
            g_pTOfDataMem = (TABLE_INFO*) TOfData0();
            uiIdNumber = *(int*) Data0();
        }
        else
        {
            printf("uiFlag == 1\n");
            g_pTOfDataMem = (TABLE_INFO*) TOfData1();
            uiIdNumber = *(int*) Data1();
        }

        for (unsigned int i = 0; i < uiIdNumber; i++)
        {
            if ((g_pTOfDataMem + i)->ID == uiMessageID)
            {
                //printf("ID = %d\n", (g_pTOfDataMem + i)->ID);
                //printf("Name = %s\n", (g_pTOfDataMem + i)->Message_name);
                //printf("offset = %d\n", (g_pTOfDataMem + i)->offset);
                //printf("size = %d\n", (g_pTOfDataMem + i)->data_size);
                //printf("<<<buff=%s>>>\n\n", CStrTool::Str2Hex( (void*)O2P((g_pTOfDataMem + i)->offset), (g_pTOfDataMem + i)->data_size));
                objRecords.ParseFromArray((void*)O2P((g_pTOfDataMem + i)->offset), (g_pTOfDataMem + i)->data_size);
                return 0;
            }
        }

        //到这来了，说明没找到
        printf("ERROR:Your Message ID is:%d,cannot find this Message.\n", uiMessageID);
        return -1;
    }

    void getBuffById(unsigned int inputID, char *buff, int *buff_size); //对外提供的接口，用户只需输入Mem首地址， 且输入ID号，即可得到ID所对应的共享内存信息

    virtual ~CShmConfigApiEx();

private:
    void *g_pvMem;                                     //保存共享内存首地址信息

    TABLE_INFO* g_pHeadMem;       //指向头指针首地址
    char *g_pDataMem;                  //指向数据区所对应的首地址

    inline char* Head() {return O2P(0);}

    inline char* Data0()  {return O2P(HEAD_LENGTH);}

    inline char* TOfData0()  {return O2P(HEAD_LENGTH+8);}        //注意table中的最开始4个字节，保存table中ID的数量

    inline char* BuffOfData0()  {return O2P(HEAD_LENGTH+8+TABLE_LENGTH);}  //buff的起始位置从2M的地方开始

    inline char* Data1()  {return O2P(DATA1_OFFSET);}

    inline char* TOfData1()  {return O2P(DATA1_OFFSET+8);}       //注意table中的最开始4个字节，保存table中ID的数量

    inline char* BuffOfData1()  {return O2P(DATA1_OFFSET+8+TABLE_LENGTH);}
};
}
#endif /* SHM_CONFIG_API_EX_H_ */

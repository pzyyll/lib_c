#ifndef ADOPTMAPPROTOCOL_H
#define ADOPTMAPPROTOCOL_H

#include "adopt_api.h"

namespace snslib
{

// 命令字定义
const unsigned short ADOPT_MODIFY_PROTO_CMD_SET     = 0x0200;
const unsigned short ADOPT_MODIFY_PROTO_CMD_DEL     = 0x0201;
const unsigned short ADOPT_MODIFY_PROTO_CMD_GET     = 0x0202;
const unsigned short ADOPT_MODIFY_PROTO_CMD_GETALL  = 0x0302;

struct stSvrMsgHeader
{
    unsigned short iPkgLen;
    unsigned short iPetVersion;
    unsigned int iUin;
    unsigned short iPetCmdID;
    unsigned char cPetMsgType;
};

typedef struct stSvrMsgHeader SSvrMsgHeader;

const int SVR_MSG_HEADER_LEN = 11;

class CAdoptProtocol
{
public:
	// 包头协议解析
	static int PackHeader(unsigned char *pMemory, const SSvrMsgHeader &stHeader);
	static int UnPackHeader(const unsigned char *pMemory, SSvrMsgHeader &stHeader);

	// 领养查询协议解析
	static int PackGetResponse(unsigned char *pMemory, const SAdoptGetResp &stResponse);
	static int UnPackGetResponse(const unsigned char *pMemory, SAdoptGetResp &stResponse);

	// 领养设置协议解析
	static int PackSetRequest(unsigned char *pMemory, unsigned long long ullPetID,
    char szPetName[PET_NAME_LEN + 1]);
	static int UnPackSetRequest(unsigned char *pMemory, unsigned long long &ullPetID,
    char szPetName[PET_NAME_LEN + 1]);
	static int PackSetResponse(unsigned char *pMemory, short shResult);
	static int UnPackSetResponse(const unsigned char *pMemory, short &shResult);

	// 领养设置协议解析
	static int PackDelRequest(unsigned char *pMemory, unsigned long long ullPetID);
	static int UnPackDelRequest(const unsigned char *pMemory, unsigned long long &ullPetID);
};

}

#endif


#include "adopt_protocol.h"
#include "comm/util/pet_util.h"

using namespace snslib;

int CAdoptProtocol::PackHeader(unsigned char *pMemory, const SSvrMsgHeader &stHeader)
{
	int iOffset = 0;

	iOffset += CBuffTool::WriteShort(pMemory + iOffset, stHeader.iPkgLen);
	iOffset += CBuffTool::WriteShort(pMemory + iOffset, stHeader.iPetVersion, false);
	iOffset += CBuffTool::WriteInt(pMemory + iOffset, stHeader.iUin);
	iOffset += CBuffTool::WriteShort(pMemory + iOffset, stHeader.iPetCmdID);
	iOffset += CBuffTool::WriteByte(pMemory + iOffset, stHeader.cPetMsgType);

	return iOffset;
}

int CAdoptProtocol::UnPackHeader(const unsigned char *pMemory, SSvrMsgHeader &stHeader)
{
	int iOffset = 0;

	iOffset += CBuffTool::ReadShort(pMemory + iOffset, stHeader.iPkgLen);
	iOffset += CBuffTool::ReadShort(pMemory + iOffset, stHeader.iPetVersion, false);
	iOffset += CBuffTool::ReadInt(pMemory + iOffset, stHeader.iUin);
	iOffset += CBuffTool::ReadShort(pMemory + iOffset, stHeader.iPetCmdID);
	iOffset += CBuffTool::ReadByte(pMemory + iOffset, stHeader.cPetMsgType);

	return iOffset;
}

int CAdoptProtocol::PackGetResponse(unsigned char *pMemory, const SAdoptGetResp &stResponse)
{
	int iOffset = 0;
	return iOffset;
}

int CAdoptProtocol::UnPackGetResponse(const unsigned char *pMemory, SAdoptGetResp &stResponse)
{
	int iOffset = 0;

    stResponse.byPetNum= 0;
	iOffset += CBuffTool::ReadByte(pMemory + iOffset, stResponse.byPetNum);
    if (stResponse.byPetNum> MAX_ADOPT_PET_COUNT)
    {
        stResponse.byPetNum= MAX_ADOPT_PET_COUNT;
    }
    short shLevel = 0;
    for (int i = 0; i < stResponse.byPetNum; ++i)
    {
        iOffset += CBuffTool::ReadLongLong(pMemory + iOffset, stResponse.aullPetID[i]);
        CBuffTool::ReadString(pMemory + iOffset, stResponse.aszPetName[i], PET_NAME_LEN);
        // 注意:协议里面的宠物昵称是32个字节，而宠物昵称实际上最多16个字节
        iOffset += 32;
        iOffset += CBuffTool::ReadShort(pMemory + iOffset, shLevel);
        stResponse.aszPetName[i][PET_NAME_LEN] = 0;
    }
    
	return iOffset;
}

int CAdoptProtocol::PackSetRequest(unsigned char *pMemory, unsigned long long ullPetID,
    char szPetName[PET_NAME_LEN + 1])
{
	int iOffset = 0;

    iOffset += CBuffTool::WriteLongLong(pMemory + iOffset, ullPetID);
    iOffset += CBuffTool::WriteString(pMemory + iOffset, szPetName, PET_NAME_LEN);
    iOffset += CBuffTool::WriteShort(pMemory + iOffset, (short)0);
        
	return iOffset;
}

int CAdoptProtocol::UnPackSetRequest(unsigned char *pMemory, unsigned long long &ullPetID,
    char szPetName[PET_NAME_LEN + 1])
{
	int iOffset = 0;
	return iOffset;
}

int CAdoptProtocol::PackSetResponse(unsigned char *pMemory, short shResult)
{
	int iOffset = 0;
	return iOffset;
}

int CAdoptProtocol::UnPackSetResponse(const unsigned char *pMemory, short &shResult)
{
	int iOffset = 0;

    iOffset += CBuffTool::ReadShort(pMemory + iOffset, shResult);
    
	return iOffset;
}


int CAdoptProtocol::PackDelRequest(unsigned char *pMemory, unsigned long long ullPetID)
{
	int iOffset = 0;

    iOffset += CBuffTool::WriteLongLong(pMemory + iOffset, ullPetID);
        
	return iOffset;
}

int CAdoptProtocol::UnPackDelRequest(const unsigned char *pMemory, unsigned long long &ullPetID)
{
	int iOffset = 0;
	return iOffset;
}



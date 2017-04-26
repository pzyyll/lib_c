#include <arpa/inet.h>

#include "comm/log/pet_log.h"
#include "comm/ini_file/ini_file.h"
#include "comm/util/pet_util.h"

#include "api/include/pet_protocol.h"
#include "api/session_api/feedsvr_prot.h"
#include "comm/safe_tcp_client/safe_tcp_client.h"

using namespace petlib;

CSafeTcpClient g_objTcpClient;
PetHeader g_stSendPetHeader, g_stRecvPetHeader;
char g_szSendBuff[102400], g_szRecvBuff[102400];
int g_iSendLen, g_iRecvLen;

int Init(const char *pszConfFile)
{
    int iRetVal = 0;
    if (pszConfFile == NULL)
    {
        printf("conf file is NULL\n");
        return -1;
    }

    iRetVal = g_objTcpClient.Init(pszConfFile, "WEBAPI");
    if (iRetVal != 0)
    {
        printf("init safe_tcp_client failed, ret=%d, errmsg=%s\n", iRetVal, g_objTcpClient.GetErrMsg());
        return -1;
    }

    return 0;
}

int WritePetHeader(char* pszBuffer, PetHeader &stPetHeader)
{
    int iOffSet = 0;

    iOffSet += petlib::CBuffTool::WriteShort(pszBuffer + iOffSet, stPetHeader.ushLength);
    iOffSet += petlib::CBuffTool::WriteLongLong(pszBuffer + iOffSet, stPetHeader.ullPetID);
    iOffSet += petlib::CBuffTool::WriteShort(pszBuffer + iOffSet, stPetHeader.ushVersion);
    iOffSet += petlib::CBuffTool::WriteByte(pszBuffer + iOffSet, stPetHeader.byLangVer);
    iOffSet += petlib::CBuffTool::WriteShort(pszBuffer + iOffSet, stPetHeader.ushCmdID);
    iOffSet += petlib::CBuffTool::WriteShort(pszBuffer + iOffSet, stPetHeader.ushCheckSum);

    return iOffSet;
}

int ReadPetHeader(const char* pszBuffer, PetHeader &stPetHeader)
{
    int iOffSet = 0;

    iOffSet += petlib::CBuffTool::ReadShort(pszBuffer + iOffSet, stPetHeader.ushLength);
    iOffSet += petlib::CBuffTool::ReadLongLong(pszBuffer + iOffSet, stPetHeader.ullPetID);
    iOffSet += petlib::CBuffTool::ReadShort(pszBuffer + iOffSet, stPetHeader.ushVersion);
    iOffSet += petlib::CBuffTool::ReadByte(pszBuffer + iOffSet, stPetHeader.byLangVer);
    iOffSet += petlib::CBuffTool::ReadShort(pszBuffer + iOffSet, stPetHeader.ushCmdID);
    iOffSet += petlib::CBuffTool::ReadShort(pszBuffer + iOffSet, stPetHeader.ushCheckSum);

    return iOffSet;
}

int CheckRspPkg()
{
    int iOffSet = 0;

    //校验接收到数据的长度
    if((unsigned int)g_iRecvLen < (PET_HEADER_LEN + sizeof(unsigned short)))
    {
        //数据包最小长度为 PetHeader + ushRetVal
        printf("recv_pkg_len is not valid, pkg_len=%d\n", g_iRecvLen);
        return -1;
    }

    //读取包头
    iOffSet += ReadPetHeader(g_szRecvBuff+iOffSet, g_stRecvPetHeader);

    //校验包长
    if (g_stRecvPetHeader.ushLength != g_iRecvLen)
    {
        printf("pkg_len is not valid, pkg_len=%d, recv_len=%d\n", g_stRecvPetHeader.ushLength, g_iRecvLen);
        return -1;
    }

    //校验CheckSum
    unsigned short ushCheckSum = g_stRecvPetHeader.ushCheckSum;
    unsigned short ushCheckSumCalc = 0;
    g_stRecvPetHeader.ushCheckSum = 0;
    WritePetHeader(g_szRecvBuff, g_stRecvPetHeader);
    ushCheckSumCalc = CStrTool::CheckSum(g_szRecvBuff, g_iRecvLen);
    if (ushCheckSum != ushCheckSumCalc)
    {
        printf("cksum is not valid, rsp_cksum=%d, rsp_cksug_calc=%d\n", ushCheckSum, ushCheckSumCalc);
        return -1;
    }

    //校验PetID
    if (g_stSendPetHeader.ullPetID != g_stRecvPetHeader.ullPetID)
    {
        printf("pet_id is not valid, req_pet_id=%llu, rsp_pet_id=%llu\n", g_stSendPetHeader.ullPetID, g_stRecvPetHeader.ullPetID);
        return -1;
    }

    //校验CmdID
    if (g_stSendPetHeader.ushCmdID != g_stRecvPetHeader.ushCmdID)
    {
        printf("cmd_id is not valid, req_cmd_id=%d, rsp_cmd_id=%d\n", g_stSendPetHeader.ushCmdID, g_stRecvPetHeader.ushCmdID);
        return -1;
    }

    //读取返回值
    unsigned short ushRetVal = 0;
    iOffSet += CFeedSvrProt::ReadShort(g_szRecvBuff+iOffSet, ushRetVal);

    if(ushRetVal != 0)
    {
        //解析错误返回包
        WebRetMsg stWebRetMsg;

        iOffSet += CFeedSvrProt::Read(g_szRecvBuff+iOffSet, stWebRetMsg);
        printf("ret err msg:%s", stWebRetMsg.szInfoMsg);
    }

    return 0;
}

int SendHello(unsigned long long ullPetID)
{
    int iRetVal = 0;

    g_stSendPetHeader.ushLength = 0;
    g_stSendPetHeader.ullPetID = ullPetID;
    g_stSendPetHeader.ushVersion = 0;
    g_stSendPetHeader.byLangVer = 0;
    g_stSendPetHeader.ushCmdID = CMD_FEED_HELLO;
    g_stSendPetHeader.ushCheckSum = 0;

    HelloReq stHelloReq;
    HelloRsp stHelloRsp;
    stHelloReq.ucInterTypeNum = 0;
    stHelloReq.ushTimes = 0;

    g_iSendLen = 0;

    g_iSendLen += WritePetHeader(g_szSendBuff+g_iSendLen, g_stSendPetHeader);
    g_iSendLen += CFeedSvrProt::Write(g_szSendBuff+g_iSendLen, stHelloReq);

    g_stSendPetHeader.ushLength = g_iSendLen;
    WritePetHeader(g_szSendBuff, g_stSendPetHeader);

    g_stSendPetHeader.ushCheckSum = CStrTool::CheckSum(g_szSendBuff, g_iSendLen);
    WritePetHeader(g_szSendBuff, g_stSendPetHeader);

    g_iRecvLen = sizeof(g_szRecvBuff);
    iRetVal = g_objTcpClient.SendAndRecv(g_szSendBuff, g_iSendLen, g_szRecvBuff, (unsigned int *)&g_iRecvLen);
    if(iRetVal != 0)
    {
        printf("sendrecv failed, ret=%d, errmsg=%s\n", iRetVal, g_objTcpClient.GetErrMsg());
        return -1;
    }

    iRetVal = CheckRspPkg();
    if(iRetVal != 0)
    {
        return -1;
    }

    int iOffSet = PET_HEADER_LEN + sizeof(unsigned short);
    iOffSet += CFeedSvrProt::Read(g_szRecvBuff+iOffSet, stHelloRsp);

    printf("Hello succ\n");

    return 0;
}

int main(int argc, char *argv[])
{
    int iRetVal = 0;

    if (argc < 3)
    {
        printf("usage: %s <conf_file> <pet_id>\n", argv[0]);
        return -1;
    }

    OpenPetLog("stcptest");

    const char *pszConfFile = argv[1];
    unsigned long long ullPetID = strtoull(argv[2], NULL, 10);

    iRetVal = Init(pszConfFile);
    if (iRetVal != 0)
    {
        return iRetVal;
    }

    iRetVal = SendHello(ullPetID);
    if (iRetVal != 0)
    {
        return iRetVal;
    }

    STcpConnStatusHeader stSTcpConnStatusHeader;
    std::vector<STcpConnStatus> vstConnStatus;
    g_objTcpClient.GetAllStatus(stSTcpConnStatusHeader, vstConnStatus);
    printf("status_num=%d\n", vstConnStatus.size());
    char szStatusBuff[102400];
    int iStatusBuffLen = 0;

    for (unsigned int i=0; i<vstConnStatus.size(); i++)
    {
        iStatusBuffLen += snprintf(szStatusBuff+iStatusBuffLen, ((int)sizeof(szStatusBuff)) > iStatusBuffLen? sizeof(szStatusBuff)-iStatusBuffLen:0,
            "%s:%d:%d ",
            inet_ntoa(*((struct in_addr*)&vstConnStatus[i].uiIPAddr)),
            vstConnStatus[i].ushPort,
            vstConnStatus[i].ushStatus);

        if (sizeof(szStatusBuff)-iStatusBuffLen == 0)
        {
            break;
        }
    }

    printf("STATUS:%s\n", szStatusBuff);

    return 0;
}

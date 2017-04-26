#include <string.h>

#include "webapi_flag.h"
#include "api/proto/flag.pb.h"

using namespace snslib;
using namespace std;

CWebApiFlag::CWebApiFlag()
{}

CWebApiFlag::~CWebApiFlag()
{}

int CWebApiFlag::Flag(unsigned short ushSvrID, unsigned int uiUin, unsigned short ushAppID, unsigned short ushType )
{
    string str;
    unsigned short ushCmd;

    if(!ushType){
        //读取标志位
        ushCmd = CMD_GET_FLAG;
        GetFlag oGet;
        FlagObj * poFlag = oGet.add_flag_obj();
        poFlag->set_uin(uiUin);
        poFlag->set_appid(ushAppID);
        poFlag->set_ret(0);
        oGet.SerializeToString(&str);
    }else if(ushType == 1){
        // 设置标志位
        ushCmd = CMD_SET_FLAG;
        SetFlag oSet;
        FlagObj * poFlag = oSet.add_flag_obj();
        poFlag->set_uin(uiUin);
        poFlag->set_appid(ushAppID);
        poFlag->set_ret(0);
        oSet.SerializeToString(&str);
    }else if(ushType == 2){
        // 删除标志位
        ushCmd = CMD_DEL_FLAG;
        DelFlag oDel;
        FlagObj * poFlag = oDel.add_flag_obj();
        poFlag->set_uin(uiUin);
        poFlag->set_appid(ushAppID);
        poFlag->set_ret(0);
        oDel.SerializeToString(&str);
    }else{
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "type error, type=%d", ushType );
        return -1;
    }

    AppHeader stAppHeader;
    memset(&stAppHeader, 0x0, sizeof(stAppHeader));

    stAppHeader.uiUin = uiUin;
    stAppHeader.ushCmdID = ushCmd;
    stAppHeader.ushDestSvrID = ushSvrID;

    AppHeader stRecvAppHeader;
    char *pszRecvBuff;
    int iRecvLen;

    int rv = SendAndRecv(stAppHeader, (char *)str.c_str(), str.length(), stRecvAppHeader, &pszRecvBuff, &iRecvLen);
    if(rv){
        return -2;
    }

    if (!ushType) {
        // 读取回包
        GetFlag oGet;
        oGet.ParseFromArray( pszRecvBuff, iRecvLen );
        for(int i=0; i < oGet.flag_obj_size(); i++ ){
            if((oGet.flag_obj(i).appid() == ushAppID)&&(oGet.flag_obj(i).ret()==1))
                return 0;
        }
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "appid not found, appid=%d", ushAppID );
        return -1;
    }else if(ushType == 1){
        // 设置标志位
        SetFlag oSet;
        oSet.ParseFromArray( pszRecvBuff, iRecvLen );
        for(int i=0; i < oSet.flag_obj_size(); i++ ){
            if( (oSet.flag_obj(i).appid() == ushAppID) && (oSet.flag_obj(i).ret()==1) )
                return 0;
        }
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "appid not found, appid=%d", ushAppID );
        return -1;
    }else if(ushType == 2){
        // 删除标志位
        DelFlag oDel;
        oDel.ParseFromArray( pszRecvBuff, iRecvLen );
        for(int i=0; i < oDel.flag_obj_size(); i++ ){
            if((oDel.flag_obj(i).appid() == ushAppID)&&(oDel.flag_obj(i).ret() == 1) )
                return 0;
        }
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "appid not found, appid=%d", ushAppID );
        return -1;
    }

    return 0;
}

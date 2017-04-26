#include <string.h>

#include "webapi_lock.h"
#include "api/proto/lock.pb.h"

using namespace snslib;
using namespace std;

CWebApiLock::CWebApiLock()
{}

CWebApiLock::~CWebApiLock()
{}

void CWebApiLock::SetSvrID(unsigned short ushSvrID)
{
	m_ushSvrID = ushSvrID;
}

int CWebApiLock::Action(unsigned int uiUin, unsigned short ushType, const std::vector<unsigned int> & objUinVect  )
{
    string str;
    unsigned short ushCmd = ushType;

    if(ushCmd == CMD_LOCK_GET){
    	snslib::GetLock oGetLock;
    	for(unsigned int k=0; k<objUinVect.size(); k++){
    		snslib::LockObj* pObj = oGetLock.add_lock_obj();
    		pObj->set_uin(objUinVect[k]);
    	}
    	oGetLock.SerializeToString(&str);
    }else if(ushCmd == CMD_LOCK_SET){
        snslib::SetLock oSetLock;
        for(unsigned int k=0; k<objUinVect.size(); k++){
			snslib::LockObj* pObj = oSetLock.add_lock_obj();
			pObj->set_uin(objUinVect[k]);
			pObj->set_id(9000);
		}
		oSetLock.SerializeToString(&str);
    }else if(ushCmd == CMD_LOCK_DEL){
    	snslib::DelLock oDelLock;
		for(unsigned int k=0; k<objUinVect.size(); k++){
			snslib::LockObj* pObj = oDelLock.add_lock_obj();
			pObj->set_uin(objUinVect[k]);
		}
		oDelLock.SerializeToString(&str);
    }else{
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "cmd error, cmd=%d", ushCmd );
        return -1;
    }

    AppHeader stAppHeader;
    memset(&stAppHeader, 0x0, sizeof(stAppHeader));

    stAppHeader.uiUin = uiUin;
    stAppHeader.ushCmdID = ushCmd;
    stAppHeader.ushDestSvrID = m_ushSvrID;

    AppHeader stRecvAppHeader;
    char *pszRecvBuff;
    int iRecvLen;

    int rv = SendAndRecv(stAppHeader, (char *)str.c_str(), str.length(), stRecvAppHeader, &pszRecvBuff, &iRecvLen);
    if(rv){
        return -2;
    }

    int iRet = 0;
    if (ushCmd == CMD_LOCK_GET) {
        // ¶ÁÈ¡»Ø°ü
        snslib::GetLock oGetLock;
        oGetLock.ParseFromArray( pszRecvBuff, iRecvLen );
        for(int k=0; k < oGetLock.lock_obj_size(); k++ ){
            iRet += oGetLock.lock_obj(k).ret();
        }
    }else if(ushCmd == CMD_LOCK_SET){
    	snslib::SetLock oSetLock;
		oSetLock.ParseFromArray( pszRecvBuff, iRecvLen );
		for(int k=0; k < oSetLock.lock_obj_size(); k++ ){
			iRet += oSetLock.lock_obj(k).ret();
		}
    }else if(ushCmd == CMD_LOCK_DEL){
    	snslib::DelLock oDelLock;
		oDelLock.ParseFromArray( pszRecvBuff, iRecvLen );
		for(int k=0; k < oDelLock.lock_obj_size(); k++ ){
			iRet += oDelLock.lock_obj(k).ret();
		}
    }

    return iRet;
}

int CWebApiLock::SetLock(unsigned int uiUin)
{
	vector<unsigned int> oSetVect;
	oSetVect.push_back(uiUin);
	return Action(uiUin, CMD_LOCK_SET, oSetVect);
}

int CWebApiLock::DelLock(unsigned int uiUin)
{
	vector<unsigned int> oDelVect;
	oDelVect.push_back(uiUin);
	return Action(uiUin, CMD_LOCK_DEL, oDelVect);
}

int CWebApiLock::GetLock(unsigned int uiUin)
{
	vector<unsigned int> oGetVect;
	oGetVect.push_back(uiUin);
	return Action(uiUin, CMD_LOCK_GET, oGetVect);
}


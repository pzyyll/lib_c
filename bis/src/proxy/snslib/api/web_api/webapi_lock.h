#ifndef _WEBAPI_LOCK_H_
#define _WEBAPI_LOCK_H_

#include "webapi_base.h"
#include <vector>

namespace snslib
{

class CWebApiLock:public CWebApiBase
{
public:
	CWebApiLock();
	~CWebApiLock();

	void SetSvrID(unsigned short ushSvrID);

	int GetLock(unsigned int uiUin);
	int SetLock(unsigned int uiUin);
	int DelLock(unsigned int uiUin);

private:
	int Action(  unsigned int uiUin, unsigned short ushType, const std::vector<unsigned int> & objUinVect  );

private:
	unsigned short m_ushSvrID;
};
}
#endif //_WEBAPI_LOCK_H_

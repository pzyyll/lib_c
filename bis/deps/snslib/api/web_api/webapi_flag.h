#ifndef _WEBAPI_FLAG_H_
#define _WEBAPI_FLAG_H_

#include "webapi_base.h"

namespace snslib
{

class CWebApiFlag:public CWebApiBase
{
public:
	CWebApiFlag();
	~CWebApiFlag();

	int Flag(unsigned short ushSvrID, unsigned int uiUin, unsigned short ushAppID, unsigned short ushType);

};
}
#endif //_WEBAPI_FLAG_H_

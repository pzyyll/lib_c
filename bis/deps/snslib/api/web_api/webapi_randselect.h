/***********************************************************
 * FileName:    webapi_randselect.h
 * Author:      smiyang
 * Date:		2010-9-10
 * Description:	为玩家推荐乐斗对象集合
 * Version:		1.0
 * Function List:
 * 				1.	GetRecommendList()  //透过WEBPCL为当前玩家获取乐斗列表
 * 				2.	UpdatePetLev()		//透过WEBPCL更新玩家等级
 ***********************************************************/
#ifndef _WEBAPI_RANDSELECT_H_
#define _WEBAPI_RANDSELECT_H_

#include <vector>
#include <map>

#include "webapi_base.h"

namespace snslib
{

class CWebApiRandSelect:public CWebApiBase
{
public:
	//构造函数
	CWebApiRandSelect();

	//析构函数
	~CWebApiRandSelect();

	/**
	 * @brief 	为当前玩家获取乐斗对象。
	 *
	 * @param 	uiUin：用户标识
	 * @param	usLev：用户等级
	 * @param 	vBlackList：黑名单-当日用户已经乐斗过的用户列表
	 * @param 	vRecommendList：为当前用户推荐的乐斗用户列表
	 * @return 	0：OK,其它：ERROR
	 */
	int GetRecommendList(unsigned int uiUin, unsigned short usLev, const std::vector<unsigned int> &vBlackList, std::vector<unsigned int> &vRecommendList);

	/**
	 * @brief 	为当前玩家获取乐斗对象。
	 *
	 * @param 	uiUin：用户标识
	 * @param	usLev：用户等级
	 * @return 	0：OK,其它：ERROR
	 */
	int UpdatePetLev(unsigned int uiUin, unsigned short usLev);

private:
    char m_szSendBuff[MAX_WEBAPI_BUFF_LEN];
    int m_iSendLen;
};
}
#endif //_WEBAPI_RANDSELECT_H_

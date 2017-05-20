/**
 * @file    route_info_api.h
 * @brief   获取路由信息的接口
 * @author  jamieli@tencent.com
 * @date    2009-03-19
 *
 * @note    该接口实现通过ProcID和DstBusID获取NextHopBusID
 *          该接口的Reload，外部模块需要实现捕获USER1信号，调用该接口
 */

#ifndef _ROUTE_INFO_API_H_
#define _ROUTE_INFO_API_H_

#include <map>
#include <vector>

namespace snslib
{

class CRouteInfo
{
public:
    typedef struct tagRouteInfo
    {
        unsigned int uiProcID;
        unsigned int uiDstID;
        unsigned int uiNextHop;
    }RouteInfo;

    const static int SUCCESS = 0;
    const static int ERROR = -1;

    const static int ERR_BASE = -200;
    const static int CONF_FILE_INVALID = ERR_BASE - 1;
    const static int ERR_NO_ROUTE = ERR_BASE - 2;

public:
    CRouteInfo();

    ~CRouteInfo();

    /**
     * @brief 初始化Route信息
     */
    int Init(const char *pszConfFile);

    /**
     * @brief 重新加载Route信息
     */
    int Reload(const char *pszConfFile);

    /**
     * @brief 获取下一跳节点的BUSID
     */
    int GetNextHop(unsigned int uiProcID, unsigned int uiDstBusID, unsigned int *puiNextHop);

    /**
     * @brief 获取所有下一跳信息
     */
    int GetAllNextHop(unsigned int uiProcID, std::vector<unsigned int> &vuiAllNextHop);

    /**
     * @brief 获取所有的路由信息
     */
    int GetAll(std::vector<RouteInfo> &vstRouteInfo);

    const char *GetErrMsg()
    {
        return m_szErrMsg;
    }

private:
    std::map<unsigned long long, unsigned int> m_mRouteInfo;
    char m_szErrMsg[256];
};

}
#endif

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "api/route_info_api/route_info_api.h"

using namespace snslib;

CRouteInfo::CRouteInfo()
{
    m_mRouteInfo.clear();
    memset(m_szErrMsg, 0x0, sizeof(m_szErrMsg));;
}

CRouteInfo::~CRouteInfo()
{

}

int CRouteInfo::Init(const char *pszConfFile)
{
    FILE *fpConfFile = NULL;

    fpConfFile = fopen(pszConfFile, "r");
    if (fpConfFile == NULL)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "conf file[%s] is not valid", pszConfFile);
        return CONF_FILE_INVALID;
    }

    m_mRouteInfo.clear();
    char szConfLine[256] = {0};

    unsigned int uiProcID, uiDstID, uiNextHopID;
    while (fgets(szConfLine, sizeof(szConfLine), fpConfFile)!= NULL)
    {
        if ((szConfLine[0] >= '0')||(szConfLine[0] <= '9'))
        {
            char *pcConfBusID = strtok(szConfLine, " \t");
            if (pcConfBusID == NULL)
            {
                continue;
            }
            uiProcID = inet_addr(pcConfBusID);
            if (uiProcID == 0)
            {
                continue;
            }

            pcConfBusID = strtok(NULL, " \t");
            if (pcConfBusID == NULL)
            {
                continue;
            }
            uiDstID = inet_addr(pcConfBusID);

            pcConfBusID = strtok(NULL, " \t");
            if (pcConfBusID == NULL)
            {
                continue;
            }
            uiNextHopID = inet_addr(pcConfBusID);
            if (uiNextHopID == 0)
            {
                continue;
            }

            unsigned long long ullKey = 0;
            memcpy(&ullKey, &uiProcID, 4);
            memcpy(((char *)&ullKey) + 4, &uiDstID, 4);

            m_mRouteInfo.insert(std::pair<unsigned long long, unsigned int>(ullKey, uiNextHopID));
        }
    }

    fclose(fpConfFile);

    return SUCCESS;
}

int CRouteInfo::Reload(const char *pszConfFile)
{
    return Init(pszConfFile);
}

int CRouteInfo::GetNextHop(unsigned int uiProcID, unsigned int uiDstBusID, unsigned int *puiNextHop)
{
    unsigned long long ullKey = 0;
    memcpy(&ullKey, &uiProcID, 4);
    memcpy(((char *)&ullKey) + 4, &uiDstBusID, 4);

    if (puiNextHop == NULL)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "puiNextHop is NULL");
        return ERROR;
    }

    std::map<unsigned long long, unsigned int>::iterator pRouteInfo;
    pRouteInfo = m_mRouteInfo.find(ullKey);
    if (pRouteInfo == m_mRouteInfo.end())
    {
        //查找默认路由
        memset(((char *)&ullKey) + 4, 0x0, 4);
        pRouteInfo = m_mRouteInfo.find(ullKey);
        if (pRouteInfo == m_mRouteInfo.end())
        {
            //没有路由信息
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "no route msg for 0x%08x|0x%08x", uiProcID, uiDstBusID);
            return ERR_NO_ROUTE;
        }
    }
    //查到正确的路由信息
    *puiNextHop = pRouteInfo->second;

    return SUCCESS;
}

int CRouteInfo::GetAllNextHop(unsigned int uiProcID, std::vector<unsigned int> &vuiAllNextHop)
{
    vuiAllNextHop.clear();

    unsigned int uiThisProcID = 0;
    std::map<unsigned long long, unsigned int>::iterator pRouteInfo;
    pRouteInfo = m_mRouteInfo.begin();
    while(pRouteInfo != m_mRouteInfo.end())
    {
        memcpy(&uiThisProcID, &(pRouteInfo->first), 4);

        if (uiThisProcID == uiProcID)
        {
            //扫描过滤掉重复的项
            unsigned int i=0;
            for(i=0; i<vuiAllNextHop.size(); i++)
            {
                if (pRouteInfo->second == vuiAllNextHop[i])
                {
                    break;
                }
            }
            if (i == vuiAllNextHop.size())
            {
                vuiAllNextHop.push_back(pRouteInfo->second);
            }
        }

        pRouteInfo++;
    }

    return SUCCESS;
}


int CRouteInfo::GetAll(std::vector<RouteInfo> &vstRouteInfo)
{
    vstRouteInfo.clear();
    std::map<unsigned long long, unsigned int>::iterator pRouteInfo = m_mRouteInfo.begin();
    RouteInfo stRouteInfo;
    while(pRouteInfo != m_mRouteInfo.end())
    {
        memcpy(&stRouteInfo.uiProcID, &pRouteInfo->first, 4);
        memcpy(&stRouteInfo.uiDstID, ((char *)&pRouteInfo->first)+4, 4);
        stRouteInfo.uiNextHop = pRouteInfo->second;

        vstRouteInfo.push_back(stRouteInfo);
        pRouteInfo++;
    }

    return SUCCESS;
}

/*
功能: 将一个map的内容持久化到share memory中。

方式: 
1.因为了便于查找，使用std::map
2.持久化的过程，每次的插入删除操作，都在除操作map本身外，还要修改shm中的内容
3.提供从shm中载入数据到map的方法

*/

#ifndef  MAP_SERIALIZE_H_
#define MAP_SERIALIZE_H_

#include "comm/log/pet_log.h"
#include "fixedsize_mem_pool.h"
#include <tr1/unordered_map>
#include <iostream>
//#include <map>

using namespace std;

namespace snslib
{

template<class DATA_TYPE> 
class CMapSerialize
{

    typedef struct
    {
        unsigned int uiUin;
        DATA_TYPE stData;
    } DATASAVE;


public:

    //typedef std::map<unsigned int,long> hash_map; //uin到共享内存偏移的映射
    typedef std::tr1::unordered_map<unsigned int,long> hash_map; //uin到共享内存偏移的映射

    int init(char *pvMem, int iMemSize, int iClearFlag = 0)
    {
        PetLog(0, 0, PETLOG_TRACE, "%s|pvMem=%p,iMemSize=%d,iClearFlag=%d ", __func__, pvMem, iMemSize, iClearFlag);

        m_pvMem = pvMem;
        m_pMemPool = new CFixedsizeMemPool<DATASAVE>();
        int iRet = m_pMemPool->Init(pvMem, iMemSize, 0, iClearFlag);
        if (0 != iRet)
            return iRet;

        //如果m_pMemPool中有数据，从中载入
        DATASAVE* pData = m_pMemPool->GetNextNode(1);
        for (int i = 0; i < m_pMemPool->GetUsedNodeNum(); i++)
        {
            if (NULL != pData)
            {
                m_oUinMap.insert(make_pair(pData->uiUin, (char*)(pData) - m_pvMem));
              //  PetLog(0, 0, PETLOG_DEBUG, "%s|uin=%u,offset=%l", __func__, pData->uiUin, (long)((char*)(pData) - m_pvMem));

            }
            pData = m_pMemPool->GetNextNode();
        }

        PetLog(0, 0, PETLOG_DEBUG, "%s|Load from MemPool succ. total records=%lu", __func__, m_oUinMap.size());

        return 0;
    }

    int get_need_memsize(int iNum)
    {
        return m_pMemPool->GetNeedMemSize(iNum);
    }

    //return:0成功    <0失败
    int insert(unsigned int uiKey, DATA_TYPE& data)
    {
        DATASAVE* pData = NULL;
        //如果已经存在于map中。更新之. 如果不存在，则创建结点
        typename hash_map::iterator it = m_oUinMap.find(uiKey);
        if (it != m_oUinMap.end())
        {
            PetLog(0, 0, PETLOG_DEBUG, "%s|find [uiKey=%u] in map, update it", __func__, uiKey);
            pData = (DATASAVE*)(m_pvMem + m_oUinMap[uiKey]);
        }
        else
        {
            PetLog(0, 0, PETLOG_DEBUG, "%s|cannot find [uiKey=%u] in map, create in shm and add it into map", __func__,
                   uiKey);
            pData = m_pMemPool->AllocateNode();
            if (NULL == pData) //估计不够节点分配了
            {
                PetLog(0, 0, PETLOG_WARN, "%s|AllocateNode failed.[uiKey=%u] ", __func__, uiKey);
                return -1;
            }
            else
            {
                m_oUinMap[uiKey] = (char*)(pData) - m_pvMem;
            }

        }

        pData->uiUin = uiKey;
        memcpy(&(pData->stData ), &data, sizeof(DATA_TYPE));

        PetLog(0, 0, PETLOG_DEBUG, "%s|add [uiKey=%u] in map succ", __func__, uiKey);
        return 0;

    }

    //return:0 成功  -1没有数据  <0删除失败
    int erase(unsigned int uiKey, DATA_TYPE& data)
    {
        //在map中找不到，就不删除了

        typename hash_map::iterator it = m_oUinMap.find(uiKey);
        if (it == m_oUinMap.end())
        {
            PetLog(0, 0, PETLOG_WARN, "%s|cannot find [uiKey=%u] in map ", __func__, uiKey);
            return -1;
        }

        //移除m_pMemPool中的数据
        DATASAVE* pData = (DATASAVE*)(m_pvMem + m_oUinMap[uiKey]);
        memcpy(&data, &pData->stData, sizeof(data));
        m_pMemPool->ReleaseNode(pData);

        //移除map中数据
        m_oUinMap.erase(it);

        PetLog(0, 0, PETLOG_DEBUG, "%s|erase [uiKey=%u] from map succ", __func__, uiKey);
        return 0;
    }

 
    //return:0 成功  -1没有数据  <0删除失败
    int erase(unsigned int uiKey)
    {
        typename hash_map::iterator it = m_oUinMap.find(uiKey);
        if (it == m_oUinMap.end())
        {
            PetLog(0, 0, PETLOG_WARN, "%s|cannot find [uiKey=%u] in map ", __func__, uiKey);
            return -1;
        }

        return erase(it);

    }

    int erase(typename hash_map::iterator it)
    {
        //移除m_pMemPool中的数据
        DATASAVE* pData = (DATASAVE*)(m_pvMem + it->second);
        //memcpy(&data, &pData->stData, sizeof(data));
        m_pMemPool->ReleaseNode(pData);

        PetLog(0, 0, PETLOG_DEBUG, "%s|erase [uiKey=%u] from map succ", __func__,it->first);
        //移除map中数据
        m_oUinMap.erase(it);

        return 0;

    }

    template <typename CallBack>
    int foreach(CallBack cb, void* arg)
    {
        typename hash_map::iterator it = m_oUinMap.begin();
        for (; it != m_oUinMap.end(); it++)
            cb(it->first, ((DATASAVE*)(m_pvMem + it->second))->stData, arg);

        return 0;
    } 


    template < typename Key, typename Value, typename Comp, typename Alloc, typename Pred>
        void zorch_if(tr1::unordered_map<Key,Value,Comp,Alloc>& m , Pred p)
    {
            typedef typename tr1::unordered_map<Key,Value,Comp,Alloc>::iterator iter_t;

            for(iter_t i = m.begin(); i != m.end(); )
            {
                if(p(*i))
                {
                    iter_t dommed = i;
                    ++i;
                    m.erase(dommed);
                }
                else
                {
                    ++i;
                }
            }

    }


    template <typename Pred>
        void remove_if_m(Pred p)
    {
            typedef typename hash_map::iterator iter_t;

            for(iter_t i = m_oUinMap.begin(); i != m_oUinMap.end(); )
            {
                PetLog(0, 0, PETLOG_DEBUG, "%s|key:%u data:%ld", 
                        __func__, i->first,i->second);
                if(p(i->first,((DATASAVE*)(m_pvMem + i->second))->stData))
                {
                    PetLog(0, 0, PETLOG_DEBUG, "%s|%u|need erase",__func__, i->first);
                    iter_t dommed = i;
                    ++i;
                    erase(dommed);
                    //m.erase(dommed);
                }
                else
                {
                    ++i;
                }
            }

    }



    //return:0 查找到  -1 未找到记录
    int find(unsigned int uiKey, DATA_TYPE& data)
    {

        memset(&data, 0, sizeof(data));
        typename hash_map::iterator it = m_oUinMap.find(uiKey);

        if (it == m_oUinMap.end())
        {
            PetLog(0, 0, PETLOG_DEBUG, "%s|cannot find [uiKey=%u] in map ", __func__, uiKey);
            return -1;
        }
        else
        {

            DATASAVE* pData = (DATASAVE*)(m_pvMem + m_oUinMap[uiKey]);
            memcpy(&data, &pData->stData, sizeof(data));
            PetLog(0, 0, PETLOG_DEBUG, "%s|find [uiKey=%u] from map succ", __func__, uiKey);
        }
        return 0;
    }

    inline size_t size()
    {
        return m_oUinMap.size();
    }

public:

    CMapSerialize():m_pMemPool(NULL),m_pvMem(NULL)
    {

    }
    ~CMapSerialize()
    {
        if (m_pMemPool)
            delete m_pMemPool;

    }


private:
    hash_map m_oUinMap;
    CFixedsizeMemPool<DATASAVE>* m_pMemPool;
    char* m_pvMem; //各数据结点所在位置的偏移相对于此

};
}



#endif

/**
 * @file    timer_pool.h
 * @brief   存放timer数据节点的池，精度为1/10秒
 * @author  jamieli@tencent.com
 * @date    2010-04-16
 */

#ifndef _TIMER_POOL_H_
#define _TIMER_POOL_H_

#include <vector>
#include <sys/time.h>
#include <limits.h>

#include "comm/mem_pool/fixedsize_mem_pool.h"
#include "comm/util/pet_util.h"
#include "comm/log/pet_log.h"

namespace snslib
{
template<class DATATYPE>
class CTimerPool
{
public:
    const static int SUCCESS = 0;
    const static int ERROR = -1;

    const static int POOL_FULL = 101;
    const static int TIMER_NOT_EXIST = 102;

    const static int TB_SEC_NUM = 600;
    const static int TB_MIN_NUM = 60;
    const static int TB_HOUR_NUM = 24;
    const static int TBALL_NUM = TB_SEC_NUM + TB_MIN_NUM + TB_HOUR_NUM;

public:
    typedef struct tagTimerPoolHeader
    {
        unsigned int uiMagicNumber;
        unsigned int uiSecPt;   			// 秒针的偏移
        unsigned int uiMinPt;   			// 分针的偏移
        unsigned int uiHourPt;   			// 时针的偏移
        int iTimerNodeNum;         			// 定时器的个数
    }TimerPoolHeader;

    //双向链表头，或者叫定时器的桶，指针记录的都是偏移量
    //由于TimerBucket的前两个字段排序与TimerNode前两个字段排序一致，可以通过TimerNode的指针访问这两个字段
    typedef struct tagTimerBucket
    {
        unsigned int uiNext;
        unsigned int uiPrev;
    }TimerBucket;

    //双向链表节点，用于存放实际Timer的数据
    typedef struct tagTimerNode
    {
        unsigned int uiNext;
        unsigned int uiPrev;
        struct tm stExpire;  //到期时间
        unsigned int uiRandom;  //防止误删Timer的随机数
        DATATYPE TimerParam;
    }TimerNode;

private:
    TimerPoolHeader *m_pstTPH;  //TimerPoolHeader;

    TimerBucket *m_pstSec;    // 秒
    TimerBucket *m_pstMin;    // 分
    TimerBucket *m_pstHour;    // 时

    snslib::CFixedsizeMemPool<TimerNode> m_objTimerNodePool;

    char *m_pMem;   //指向存储空间的指针
    int m_iMemSize; //存储空间的大小

    char m_szErrMsg[256];
    bool m_bInitFlag;

public:
	CTimerPool()
	{
	    m_pstTPH = NULL;
	    m_pstSec = NULL;
	    m_pstMin = NULL;
	    m_pstHour = NULL;

	    memset(m_szErrMsg, 0x0, sizeof(m_szErrMsg));

	    m_bInitFlag = false;
	}

	~CTimerPool()
	{
	}

	const char *GetErrMsg()
	{
	    return m_szErrMsg;
	}

    /**
     * @brief 获取分配iNode需要的内存空间
     *
     * @param: iNodeNum
     *
     * @return MemSize
     */
    int GetNeedMemSize(int iNodeNum)
    {
        int iMemSize = sizeof(TimerPoolHeader) + (sizeof(TimerBucket) * TBALL_NUM) + m_objTimerNodePool.GetNeedMemSize(iNodeNum);
        return iMemSize;
    }

    /**
     * @brief 初始化整个TimerPool
     * @param pvMem 内存头结点
     * @param iMemSize 开辟空间的大小
     * @param iClearFlag 是否清空标志位
     *
     * @return 0-成功 其他-失败
     */
    int Init(void *pvMem, int iMemSize, int iClearFlag = 0)
    {
        int iRetVal = 0;
        if(iMemSize <= (int)((sizeof(TimerBucket) * TBALL_NUM) + sizeof(TimerPoolHeader))){
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "memsize is not enough, head+tbsize=%d, memsize=%d", (int)((sizeof(TimerBucket) * TBALL_NUM) + sizeof(TimerPoolHeader)), iMemSize);
            return ERROR;
        }

        m_pMem = (char*)pvMem;
        m_pstTPH = (TimerPoolHeader *)m_pMem;

        m_pstSec = (TimerBucket *)(m_pMem + sizeof(TimerPoolHeader));
        m_pstMin = m_pstSec+ TB_SEC_NUM ;
        m_pstHour = m_pstMin+TB_MIN_NUM;

        void *pFSMPMem = m_pMem + sizeof(TimerPoolHeader) + (sizeof(TimerBucket) * TBALL_NUM);
        int iFSMPMemSize = iMemSize - sizeof(TimerPoolHeader) - (sizeof(TimerBucket) * TBALL_NUM);

        iRetVal = m_objTimerNodePool.Init(pFSMPMem, iFSMPMemSize, 0, iClearFlag);
        if (iRetVal != 0){
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "init timer_node_pool failed, timer_node_memsize=%d, ret=%d", iFSMPMemSize, iRetVal);
            return ERROR;
        }

        if (iClearFlag){
            //初始化TimerPool
            memset(m_pstTPH, 0x0, sizeof(TimerPoolHeader));
            memset(m_pstSec, 0x0, sizeof(TimerBucket)*TBALL_NUM);

            struct timeval stTimeValue;
            gettimeofday( &stTimeValue, NULL );
            struct tm  stTime;
            localtime_r(&stTimeValue.tv_sec, &stTime);
            stTime.tm_sec = stTime.tm_sec * 10 + stTimeValue.tv_usec / 100000;
            PetLog(0, 0, PETLOG_DEBUG, "current time: %02d:%02d:%03d\n", stTime.tm_hour, stTime.tm_min, stTime.tm_sec );
            m_pstTPH->uiHourPt 			= stTime.tm_hour;
            m_pstTPH->uiMinPt			= stTime.tm_min;
            m_pstTPH->uiSecPt			= stTime.tm_sec;
            m_pstTPH->uiMagicNumber 	= 0x19820105;
            m_pstTPH->iTimerNodeNum		= 0;
        }

        m_iMemSize = iMemSize;
        m_bInitFlag = true;

        return SUCCESS;
    }

	/**
	 * @brief 添加一个定时器到TimerPool中。类似于手表，计算出到期时间，把节点放到对应桶里就行了
	 * @param uiInterval 定时器触发时间间隔，单位为毫秒
	 * @param TimerParam 定时器附带的参数，定时器触发时会返回该参数
	 * @param ullTimerID 定时器ID，该ID用于唯一确定一个定时器
	 *
	 * @return 0-成功 其他-失败
	 */
	int AddTimer(unsigned int uiInterval, const DATATYPE &TimerParam, unsigned long long *pullTimerID)
	{
	    if (!m_bInitFlag) {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer_pool not inited.");
	        return ERROR;
	    }

	    if ((uiInterval == 0)||(uiInterval > 24 * 60 * 60 * 1000)) {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer interval is not valid, interval=%d (0<value<%d)", uiInterval, 24 * 60 * 60 * 1000);
            return ERROR;
	    }

	    TimerNode *pstTimerNode = m_objTimerNodePool.AllocateNode();
	    if (pstTimerNode == NULL) {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "allocate timer_node failed, pool full.");
            return POOL_FULL;
	    }

	    // 计算到期时间
	    TimerBucket *pstTimerBucket = NULL;
	    struct timeval stTimeNow;
	    gettimeofday( &stTimeNow, NULL );
	    time_t iExpireTime = stTimeNow.tv_sec + uiInterval / 1000;
	    struct tm stExpire, stNow;
	    localtime_r(&stTimeNow.tv_sec, &stNow);
	    localtime_r(&iExpireTime, &stExpire);
	    stExpire.tm_sec = stExpire.tm_sec * 10 + stTimeNow.tv_usec / 100000 ;
	    stNow.tm_sec = stNow.tm_sec * 10 + stTimeNow.tv_usec / 100000;

	    PetLog(0, 0, PETLOG_DEBUG, "%s|interval=%d, now:%02d:%02d:%03d, scan:%02d:%02d:%03d, expire:%02d:%02d:%03d\n", __func__, uiInterval,
	    		stNow.tm_hour, stNow.tm_min, stNow.tm_sec,
	    	    m_pstTPH->uiHourPt, m_pstTPH->uiMinPt, m_pstTPH->uiSecPt,
	    	    stExpire.tm_hour, stExpire.tm_min, stExpire.tm_sec );

	    // 计算要放入哪个桶
	    if ( stExpire.tm_hour != (int)m_pstTPH->uiHourPt ) {
	    	pstTimerBucket = m_pstHour + stExpire.tm_hour;
	    }else if(stExpire.tm_min != (int)m_pstTPH->uiMinPt) {
	    	pstTimerBucket = m_pstMin + stExpire.tm_min;
	    }else{
	    	pstTimerBucket = m_pstSec + stExpire.tm_sec;
	    }

	    pstTimerNode->stExpire = stExpire;
	    pstTimerNode->uiRandom = snslib::CRandomTool::Instance()->Get(0, INT_MAX);
	    memcpy(&(pstTimerNode->TimerParam), &TimerParam, sizeof(DATATYPE));
	    pstTimerNode->uiPrev = ((char *)pstTimerBucket - m_pMem);
	    pstTimerNode->uiNext = pstTimerBucket->uiNext;

	    TimerNode *pstLastTimeNode = NULL;
	    if (pstTimerBucket->uiNext > 0){
	        pstLastTimeNode = (TimerNode *)(m_pMem + pstTimerBucket->uiNext);
	    }
	    pstTimerBucket->uiNext = ((char *)pstTimerNode - m_pMem);
	    if (pstLastTimeNode){
	        pstLastTimeNode->uiPrev = pstTimerBucket->uiNext;
	    }

	    memcpy(pullTimerID, &pstTimerBucket->uiNext, sizeof(int));
	    memcpy(((char *)pullTimerID) + 4, &pstTimerNode->uiRandom, sizeof(int));

	    m_pstTPH->iTimerNodeNum++;

	    PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s|timer_node_num=%d, expire=%02d:%02d:%03d, bucket=%ld\n", __func__,
	    		m_pstTPH->iTimerNodeNum, pstTimerNode->stExpire.tm_hour, pstTimerNode->stExpire.tm_min, pstTimerNode->stExpire.tm_sec,
	    		pstTimerBucket - m_pstSec );

	    return SUCCESS;
	}

    /**
     * @brief 删除一个定时器
     * @param ullTimerID 定时器ID
     *
     * @return 0-成功 其他-失败
     */
	int DelTimer(unsigned long long ullTimerID){
		if (!m_bInitFlag) {
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer_pool not inited.");
			return ERROR;
		}

		if (m_pstTPH->iTimerNodeNum == 0) {
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer pool is empty, timer_id=%llu", ullTimerID);
			return TIMER_NOT_EXIST;
		}

		int iRetVal = 0;

		int iRandom = 0;
		int iOffSet = 0;

		memcpy(&iOffSet, &ullTimerID, sizeof(int));
		memcpy(&iRandom, ((char *) &ullTimerID) + 4, sizeof(int));

		if ((iOffSet <= 0) || (iOffSet >= m_iMemSize)) {
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer_id is not valid, node offset=%d, memsize=%d.", iOffSet, m_iMemSize);
			return ERROR;
		}

		TimerNode *pstTimerNode = (TimerNode *) (m_pMem + iOffSet);
		if (pstTimerNode->uiPrev == 0) {
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer_id is not valid, node prev is 0, timer_id=%llu.", ullTimerID);
			return ERROR;
		}

		if (pstTimerNode->uiRandom != (unsigned int) iRandom) {
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer_id is not valid, random is not equ, node_random=%d, id_random=%d",
					pstTimerNode->uiRandom, iRandom);
			return TIMER_NOT_EXIST;
		}

		pstTimerNode->uiRandom = snslib::CRandomTool::Instance()->Get(0, INT_MAX);
		TimerNode *pstPrevTimerNode = (TimerNode *) (m_pMem + pstTimerNode->uiPrev);
		TimerNode *pstNextTimerNode = NULL;
		if (pstTimerNode->uiNext > 0) {
			pstNextTimerNode = (TimerNode *) (m_pMem + pstTimerNode->uiNext);
		}

		pstPrevTimerNode->uiNext = pstTimerNode->uiNext;

		if (pstNextTimerNode) {
			pstNextTimerNode->uiPrev = pstTimerNode->uiPrev;
		}

		pstTimerNode->uiNext = 0;
		pstTimerNode->uiPrev = 0;

		m_pstTPH->iTimerNodeNum--;
		if (m_pstTPH->iTimerNodeNum < 0) {
			//TODO 这个是非常致命的错误
			m_pstTPH->iTimerNodeNum = 0;
		}

		iRetVal = m_objTimerNodePool.ReleaseNode(pstTimerNode);
		if (iRetVal != 0) {
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "release timer_node failed, ret=%d", iRetVal);
			return ERROR;
		}

		return SUCCESS;
	}

	/**
	 * @brief 将pstSrcTimerBucket上的定时器节点，迁移到pstDstTimerBucket后面iDstTimerBucketNum个对应的定时器桶中
	 * @param pstDstTimerBucket目标定时器桶的首指针
	 * @param iDstTimerBucketNum目标定时器桶的个数
	 * @param pstSrcTimerBucket需要迁移的定时器桶
	 * @return 0-成功 其他-失败
	 */
	int Migrate(struct tm stNow, TimerBucket *pstDstTimerBucket, int iDstTimerBucketNum, TimerBucket *pstSrcTimerBucket)
	{
		PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s|migrate, num=%d, bucket=%ld\n", __func__, iDstTimerBucketNum, pstSrcTimerBucket - m_pstSec );

	    if (pstSrcTimerBucket->uiNext == 0){
	        // 这个链上面没有节点，不需要迁移
	    	PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s|migrate, num=%d, %ld, no node need migrate\n", __func__,
	    			iDstTimerBucketNum, pstSrcTimerBucket - m_pstSec );
	        return SUCCESS;
	    }

        TimerNode *pstTimerNode = (TimerNode *)(m_pMem + pstSrcTimerBucket->uiNext);
        TimerNode *pstPrevTimerNode = (TimerNode *)(m_pMem + pstTimerNode->uiPrev);
        TimerNode *pstNextTimerNode = (pstTimerNode->uiNext>0)?(TimerNode *)(m_pMem + pstTimerNode->uiNext):NULL;

        while (pstTimerNode != NULL){
            //转移节点
            unsigned int uiInterval = 0;

            //找到正确的TB
            if( pstDstTimerBucket == m_pstMin ){
            	uiInterval = pstTimerNode->stExpire.tm_min;
            }else if( pstDstTimerBucket == m_pstSec ) {
            	uiInterval = pstTimerNode->stExpire.tm_sec;
            }else{
            	PetLog(0, 0, PETLOG_ERR, "TimePool|%s|%d| dst pt is not valid", __func__, __LINE__ );
            	return ERROR;
            }

            PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s|EXPIRE=%02d:%02d:%03d|NOW=%02d:%02d:%03d|Scan=%02d:%02d:%03d|INTERVAL=%d\n", __func__,
            		pstTimerNode->stExpire.tm_hour, pstTimerNode->stExpire.tm_min, pstTimerNode->stExpire.tm_sec,
            		stNow.tm_hour, stNow.tm_min, stNow.tm_sec, m_pstTPH->uiHourPt, m_pstTPH->uiMinPt, m_pstTPH->uiSecPt, uiInterval);

            if (uiInterval >= (unsigned int)iDstTimerBucketNum){
                PetLog(0, 0, PETLOG_ERR, "TimePool|%s|%d|node interval is too big, num=%d, interval=%d\n", __func__, __LINE__, iDstTimerBucketNum, uiInterval);
                uiInterval = iDstTimerBucketNum - 1;
            }

            //删除节点
            pstPrevTimerNode->uiNext = pstTimerNode->uiNext;
            if (pstNextTimerNode){
                pstNextTimerNode->uiPrev = pstTimerNode->uiPrev;
            }

            //插入节点
            TimerBucket *pstTmpTB = pstDstTimerBucket + uiInterval;
            pstTimerNode->uiNext = pstTmpTB->uiNext;
            pstTimerNode->uiPrev = ((char *)pstTmpTB - m_pMem);
            if (pstTmpTB->uiNext > 0){
                TimerNode *pstTmpTN = (TimerNode *)(m_pMem +pstTmpTB->uiNext);
                pstTmpTN->uiPrev = ((char *)pstTimerNode - m_pMem);
            }
            pstTmpTB->uiNext = ((char *)pstTimerNode - m_pMem);

            pstTimerNode = pstNextTimerNode;
            pstNextTimerNode = NULL;
            if (pstTimerNode != NULL){
                pstNextTimerNode = (pstTimerNode->uiNext>0)?(TimerNode *)(m_pMem + pstTimerNode->uiNext):NULL;
            }
        }

	    return SUCCESS;
	}

	/**
	 * @brief 获得当前已经触发的所有定时器信息
	 * @param vAllTimerParam 所有当前需要出发的定时器信息vector
	 * @return 0-成功 其他-失败
	 *
	 * @note 由于该接口驱动了整个时间轮的运转，有空就调用下吧
	 */
	int GetTimer(std::vector<unsigned long long> &vullTimerID, std::vector<DATATYPE> &vAllTimerParam)
	{
	    int iRetVal = 0;
	    struct timeval stTimeNow;
	    gettimeofday(&stTimeNow, NULL);
	    struct tm stNowTm;
	    localtime_r(&stTimeNow.tv_sec, &stNowTm);
	    stNowTm.tm_sec = stNowTm.tm_sec * 10 + stTimeNow.tv_usec / 100000;

		// 如果没有节点，仅用更新扫描时间
	    if (m_pstTPH->iTimerNodeNum <= 0){
	    	m_pstTPH->iTimerNodeNum = 0;
	    	m_pstTPH->uiHourPt		= stNowTm.tm_hour;
	    	m_pstTPH->uiMinPt		= stNowTm.tm_min;
	    	m_pstTPH->uiSecPt		= stNowTm.tm_sec;
			return SUCCESS;
	    }

	    // 每次检查所有链，把符合条件的全部取出来
	    int iTimeGap = ( stNowTm.tm_hour * TB_MIN_NUM * TB_SEC_NUM + stNowTm.tm_min * TB_SEC_NUM + stNowTm.tm_sec ) -
					   ( m_pstTPH->uiHourPt * TB_MIN_NUM * TB_SEC_NUM + m_pstTPH->uiMinPt * TB_SEC_NUM + m_pstTPH->uiSecPt );
		if( iTimeGap < 0 ){
	    	iTimeGap = stNowTm.tm_hour * TB_MIN_NUM * TB_SEC_NUM + stNowTm.tm_min * TB_SEC_NUM + stNowTm.tm_sec;
	    	iTimeGap += ( TB_HOUR_NUM * TB_MIN_NUM * TB_SEC_NUM ) -
	    				( m_pstTPH->uiHourPt * TB_MIN_NUM * TB_SEC_NUM + m_pstTPH->uiMinPt * TB_SEC_NUM + m_pstTPH->uiSecPt );
	    }
	    //iTimeGap = iTimeGap>0?iTimeGap:0;
//	    PetLog(0, 0, PETLOG_DEBUG, "%s|H|%d| now:%02d:%02d:%03d, scan:%02d:%02d:%03d, gap=%d\n", __func__, m_pstTPH->iTimerNodeNum,
//	    		stNowTm.tm_hour, stNowTm.tm_min, stNowTm.tm_sec, m_pstTPH->uiHourPt, m_pstTPH->uiMinPt, m_pstTPH->uiSecPt, iTimeGap );

	    while( iTimeGap > 0 ){
//	    	PetLog(0, 0, PETLOG_DEBUG, "%s|%d| now:%02d:%02d:%03d, scan:%02d:%02d:%03d, gap=%d\n", __func__, m_pstTPH->iTimerNodeNum,
//	    		   		stNowTm.tm_hour, stNowTm.tm_min, stNowTm.tm_sec, m_pstTPH->uiHourPt, m_pstTPH->uiMinPt, m_pstTPH->uiSecPt, iTimeGap );

	    	// 扫描一条链
			TimerBucket *pstSec = m_pstSec + m_pstTPH->uiSecPt;

			if(pstSec->uiNext){
				TimerNode *pstTimerNode = (TimerNode *)(m_pMem + pstSec->uiNext);
				TimerNode *pstPrevTimerNode = (TimerNode *)(m_pMem + pstTimerNode->uiPrev);
				TimerNode *pstNextTimerNode = (pstTimerNode->uiNext>0)?(TimerNode *)(m_pMem + pstTimerNode->uiNext):NULL;

				while (pstTimerNode != NULL){
					vAllTimerParam.push_back(pstTimerNode->TimerParam);
					unsigned long long ullTimerID = 0;
					int iTimerNodeOffset = (char *)pstTimerNode - m_pMem;
					memcpy(&ullTimerID, &iTimerNodeOffset, sizeof(int));
					memcpy(((char *)&ullTimerID) + 4, &pstTimerNode->uiRandom, sizeof(int));
					vullTimerID.push_back(ullTimerID);

					PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s| ++++++ Get one node ++++++ |OFF|%d|ID|%llu|NOW|%02d:%02d:%03d|EXP|%02d:%02d:%03d|\n", __func__,
							iTimerNodeOffset, ullTimerID, stNowTm.tm_hour, stNowTm.tm_min, stNowTm.tm_sec,
							pstTimerNode->stExpire.tm_hour, pstTimerNode->stExpire.tm_min, pstTimerNode->stExpire.tm_sec );

					//删除节点
					pstPrevTimerNode->uiNext = pstTimerNode->uiNext;
					if (pstNextTimerNode){
						pstNextTimerNode->uiPrev = pstTimerNode->uiPrev;
					}

					m_pstTPH->iTimerNodeNum--;
					if (m_pstTPH->iTimerNodeNum < 0){
						PetLog(0, 0, PETLOG_ERR, "TimePool|%s|%d|timer node in header is below zero, chang to zero\n", __func__, __LINE__);
						m_pstTPH->iTimerNodeNum = 0;
					}

					pstTimerNode->uiNext = 0;
					pstTimerNode->uiPrev = 0;

					iRetVal = m_objTimerNodePool.ReleaseNode(pstTimerNode);
					if (iRetVal != 0){
						PetLog(0, 0, PETLOG_ERR, "TimePool|%s|%d|release node failed, ret=%d\n", __func__, __LINE__, iRetVal);
					}

					pstTimerNode = pstNextTimerNode;
					pstNextTimerNode = NULL;
					if(pstTimerNode != NULL){
						pstNextTimerNode = (pstTimerNode->uiNext>0)?(TimerNode *)(m_pMem + pstTimerNode->uiNext):NULL;
					}
				}

				pstSec->uiNext = 0;
			}

			//进行时间轮转动
			m_pstTPH->uiSecPt++;
			iTimeGap--;
			if (m_pstTPH->uiSecPt == (unsigned int)TB_SEC_NUM){
				// min->sec
				m_pstTPH->uiSecPt = 0;
				m_pstTPH->uiMinPt++;
				if (m_pstTPH->uiMinPt == (unsigned int)TB_MIN_NUM){

					// hour->min
					m_pstTPH->uiMinPt = 0;
					m_pstTPH->uiHourPt++;
					m_pstTPH->uiHourPt %= TB_HOUR_NUM;
					Migrate(stNowTm, m_pstMin, TB_MIN_NUM, m_pstHour + m_pstTPH->uiHourPt);

					Migrate(stNowTm, m_pstSec, TB_SEC_NUM,  m_pstMin + m_pstTPH->uiMinPt);
				}else{
					Migrate(stNowTm, m_pstSec, TB_SEC_NUM,  m_pstMin + m_pstTPH->uiMinPt);
				}
			}
	    }

        return SUCCESS;
	}

    void ShowInfo()
    {
        PetLog(0, 0, PETLOG_INFO, "=========TimerPoolHeader=========");
        PetLog(0, 0, PETLOG_INFO, "TB_CUR_PT|%d|%d|%d",
                m_pstTPH->uiHourPt, m_pstTPH->uiMinPt, m_pstTPH->uiSecPt);
        PetLog(0, 0, PETLOG_INFO, "MemInfo|MemSize|%d|NodeNum|%d|UsedNodeNum|%d",
                m_objTimerNodePool.GetMemSize(),
                m_objTimerNodePool.GetNodeNum(),
                m_objTimerNodePool.GetUsedNodeNum());
    }

    // for 测试
    int GetHMS(unsigned int &uiHour, unsigned int &uiMin, unsigned int &uiSec)
    {
        uiHour = m_pstTPH->uiHourPt;
        uiMin = m_pstTPH->uiMinPt;
        uiSec = m_pstTPH->uiSecPt;
        return 0;
    }
};

}

#endif

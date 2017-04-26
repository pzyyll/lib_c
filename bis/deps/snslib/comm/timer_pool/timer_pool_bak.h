/**
 * @file    timer_pool.h
 * @brief   存放timer数据节点的池
 * @author  jamieli@tencent.com
 * @date    2010-04-16
 */

#ifndef _TIMER_POOL_H_
#define _TIMER_POOL_H_

#include <vector>
#include <sys/time.h>

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

    const static int TB1_BIT = 8;
    const static int TB2_BIT = 6;
    const static int TB3_BIT = 6;
    const static int TB4_BIT = 6;
    const static int TB5_BIT = 6;

    const static int TB1_NUM = 1<<TB1_BIT;
    const static int TB2_NUM = 1<<TB2_BIT;
    const static int TB3_NUM = 1<<TB3_BIT;
    const static int TB4_NUM = 1<<TB4_BIT;
    const static int TB5_NUM = 1<<TB5_BIT;
    const static int TBALL_NUM = TB1_NUM + TB2_NUM + TB3_NUM + TB4_NUM + TB5_NUM;


public:
    typedef struct tagTimerPoolHeader
    {
        unsigned int uiMagicNumber;
        unsigned int uiTB1CurPt;   //第一个TB组的当前TB偏移
        unsigned int uiTB2CurPt;   //第二个TB组的当前TB偏移
        unsigned int uiTB3CurPt;   //第三个TB组的当前TB偏移
        unsigned int uiTB4CurPt;   //第四个TB组的当前TB偏移
        unsigned int uiTB5CurPt;   //第五个TB组的当前TB偏移
        int iTimerNodeNum;         //定时器的个数
        struct timeval stScanTime; //上次跳动时刻
        char szUnUsed[100];
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
        struct timeval tExpire; //到期时间，精确到微秒
        unsigned int uiRandom;  //防止误删Timer的随机数
        DATATYPE TimerParam;
    }TimerNode;

private:
    TimerPoolHeader *m_pstTPH;  //TimerPoolHeader;

    TimerBucket *m_pstTB1;    //256个桶
    TimerBucket *m_pstTB2;    //64个桶
    TimerBucket *m_pstTB3;    //64个桶
    TimerBucket *m_pstTB4;    //64个桶
    TimerBucket *m_pstTB5;    //64个桶

    snslib::CFixedsizeMemPool<TimerNode> m_objTimerNodePool;

    char *m_pMem;   //指向存储空间的指针
    int m_iMemSize; //存储空间的大小

    char m_szErrMsg[256];
    bool m_bInitFlag;

public:
	CTimerPool()
	{
	    m_pstTPH = NULL;

	    m_pstTB1 = NULL;
	    m_pstTB2 = NULL;
	    m_pstTB3 = NULL;
	    m_pstTB4 = NULL;
	    m_pstTB5 = NULL;

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

        m_pstTB1 = (TimerBucket *)(m_pMem + sizeof(TimerPoolHeader));
        m_pstTB2 = m_pstTB1+TB1_NUM;
        m_pstTB3 = m_pstTB2+TB2_NUM;
        m_pstTB4 = m_pstTB3+TB3_NUM;
        m_pstTB5 = m_pstTB4+TB4_NUM;

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
            memset(m_pstTB1, 0x0, sizeof(TimerBucket)*TBALL_NUM);

            m_pstTPH->uiMagicNumber = 0x1234ABCD;
            gettimeofday( &m_pstTPH->stScanTime, NULL );
        }

        m_iMemSize = iMemSize;
        m_bInitFlag = true;

        return SUCCESS;
    }

	/**
	 * @brief 添加一个定时器到TimerPool中
	 * @param uiInterval 定时器触发时间间隔，单位ms
	 * @param TimerParam 定时器附带的参数，定时器触发时会返回该参数
	 * @param ullTimerID 定时器ID，该ID用于唯一确定一个定时器
	 *
	 * @return 0-成功 其他-失败
	 */
	int AddTimer(unsigned int uiInterval, const DATATYPE &TimerParam, unsigned long long *pullTimerID)
	{
	    if (!m_bInitFlag)
	    {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer_pool not inited.");
	        return ERROR;
	    }

	    if (uiInterval == 0)
	    {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer interval is not valid, interval=%d", uiInterval);
            return ERROR;
	    }

	    TimerNode *pstTimerNode = m_objTimerNodePool.AllocateNode();
	    if (pstTimerNode == NULL)
	    {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "allocate timer_node failed, pool full.");
            return POOL_FULL;
	    }

	    TimerBucket *pstTimerBucket = NULL;
	    // 这里需要加入补偿值
	    struct timeval stTimeExpire, stTimeNow;
	    gettimeofday(&stTimeNow, NULL);
	    if(timercmp(&stTimeNow, &m_pstTPH->stScanTime, <)){
	    	PetLog(0, 0, PETLOG_ERR, "Now is less than scan time, use scan time instead" );
	    	stTimeNow = m_pstTPH->stScanTime;
		}
	    stTimeExpire =  stTimeNow;
	    PetLog(0, 0, PETLOG_DEBUG, "%s|interval=%d, gap=%d, CUR: %03d %03d %03d %03d %03d", __func__, uiInterval, (stTimeNow.tv_sec - m_pstTPH->stScanTime.tv_sec)*1000 + (stTimeNow.tv_usec - m_pstTPH->stScanTime.tv_usec)/1000,
	    		m_pstTPH->uiTB1CurPt, m_pstTPH->uiTB2CurPt, m_pstTPH->uiTB3CurPt, m_pstTPH->uiTB4CurPt, m_pstTPH->uiTB5CurPt );
	    unsigned int uiIntervalForTB = uiInterval + (stTimeNow.tv_sec - m_pstTPH->stScanTime.tv_sec)*1000 + (stTimeNow.tv_usec - m_pstTPH->stScanTime.tv_usec)/1000 ;
	    unsigned int uiIntervalTotal = uiIntervalForTB;

	    if ((uiIntervalForTB >> TB1_BIT) == 0){
	        //FOR TB1
	        pstTimerBucket = m_pstTB1 + ((m_pstTPH->uiTB1CurPt + uiIntervalForTB - 1)%TB1_NUM) ;
	        PetLog(0, 0, PETLOG_DEBUG, "%s|interval|%d|TB1|%d,%d", __func__, uiIntervalForTB, m_pstTPH->uiTB1CurPt, ((m_pstTPH->uiTB1CurPt + uiIntervalForTB - 1)%TB1_NUM) );
	    }else{
	        uiIntervalForTB += m_pstTPH->uiTB1CurPt;
	        if ((uiIntervalForTB >> (TB1_BIT+TB2_BIT)) == 0){
	            //FOR TB2
                pstTimerBucket = m_pstTB2 + ((m_pstTPH->uiTB2CurPt + (uiIntervalForTB >> TB1_BIT) - 1 )%TB2_NUM) ;
                PetLog(0, 0, PETLOG_DEBUG, "%s|interval|%d|TB2|%d,%d", __func__, uiIntervalForTB, m_pstTPH->uiTB2CurPt, ((m_pstTPH->uiTB2CurPt + (uiIntervalForTB >> TB1_BIT) - 1)%TB2_NUM) );
            }else{
	            uiIntervalForTB += (m_pstTPH->uiTB2CurPt << TB1_BIT) + m_pstTPH->uiTB1CurPt;
	            if ((uiIntervalForTB >> (TB1_BIT+TB2_BIT+TB3_BIT)) == 0){
	                //FOR TB3
                    pstTimerBucket = m_pstTB3 + ((m_pstTPH->uiTB3CurPt + (uiIntervalForTB >> (TB1_BIT+TB2_BIT)) - 1)%TB3_NUM)  ;
                    PetLog(0, 0, PETLOG_DEBUG, "%s|interval|%d|TB3|%d,%d", __func__, uiIntervalForTB, m_pstTPH->uiTB3CurPt, ((m_pstTPH->uiTB3CurPt + (uiIntervalForTB >> (TB1_BIT+TB2_BIT)) - 1)%TB3_NUM)   );
                }else{
	                uiIntervalForTB += (m_pstTPH->uiTB3CurPt << (TB1_BIT+TB2_BIT)) + (m_pstTPH->uiTB2CurPt << TB1_BIT) + m_pstTPH->uiTB1CurPt;
	                if ((uiIntervalForTB >> (TB1_BIT+TB2_BIT+TB3_BIT+TB4_BIT)) == 0){
	                    //FOR TB4
	                    pstTimerBucket = m_pstTB4 + ((m_pstTPH->uiTB4CurPt + (uiIntervalForTB >> (TB1_BIT+TB2_BIT+TB3_BIT)) - 1 )%TB4_NUM) ;
	                    PetLog(0, 0, PETLOG_DEBUG, "%s|interval|%d|TB4|%d,%d", __func__, uiIntervalForTB, m_pstTPH->uiTB4CurPt, ((m_pstTPH->uiTB4CurPt + (uiIntervalForTB >> (TB1_BIT+TB2_BIT+TB3_BIT)) - 1)%TB4_NUM)  );
	                }else{
	                    //FOR TB5
	                    uiIntervalForTB += (m_pstTPH->uiTB4CurPt << (TB1_BIT+TB2_BIT+TB3_BIT)) + (m_pstTPH->uiTB3CurPt << (TB1_BIT+TB2_BIT)) + (m_pstTPH->uiTB2CurPt << TB1_BIT) + m_pstTPH->uiTB1CurPt;
	                    pstTimerBucket = m_pstTB5 + ((m_pstTPH->uiTB5CurPt + (uiIntervalForTB >> (TB1_BIT+TB2_BIT+TB3_BIT+TB4_BIT)) - 1)%TB5_NUM);
	                    PetLog(0, 0, PETLOG_DEBUG, "%s|interval|%d|TB5|%d,%d", __func__, uiIntervalForTB, m_pstTPH->uiTB5CurPt, ((m_pstTPH->uiTB5CurPt + (uiIntervalForTB >> (TB1_BIT+TB2_BIT+TB3_BIT+TB4_BIT)) - 1 )%TB5_NUM)  );
	                }
	            }
	        }
	    }

	    stTimeExpire.tv_sec += (uiIntervalTotal / 1000);
	    if ((stTimeExpire.tv_usec + ((uiIntervalTotal % 1000) * 1000)) < 1000000){
	        stTimeExpire.tv_usec += ((uiIntervalTotal % 1000) * 1000);
	    }else{
	        stTimeExpire.tv_sec += 1;
	        stTimeExpire.tv_usec += ((uiIntervalTotal % 1000) * 1000) - 1000000;
	    }

	    PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s|TB_OFF=%d|NOW=%d.%d|EXPIRE=%d.%d", __func__, (int)(pstTimerBucket-m_pstTB1), (int)stTimeNow.tv_sec, (int)stTimeNow.tv_usec, (int)stTimeExpire.tv_sec, (int)stTimeExpire.tv_usec);

	    memcpy(&(pstTimerNode->tExpire), &stTimeExpire, sizeof(stTimeExpire));
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

	    PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s|timer_node_num=%d", __func__, m_pstTPH->iTimerNodeNum);

	    return SUCCESS;
	}

    /**
     * @brief 删除一个定时器
     * @param ullTimerID 定时器ID
     *
     * @return 0-成功 其他-失败
     */
	int DelTimer(unsigned long long ullTimerID)
	{
        if (!m_bInitFlag)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer_pool not inited.");
            return ERROR;
        }

        if (m_pstTPH->iTimerNodeNum == 0)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer pool is empty, timer_id=%llu", ullTimerID);
            return TIMER_NOT_EXIST;
        }

        int iRetVal = 0;

	    int iRandom = 0;
	    int iOffSet = 0;

	    memcpy(&iOffSet, &ullTimerID, sizeof(int));
	    memcpy(&iRandom, ((char *)&ullTimerID) + 4, sizeof(int));

        if ((iOffSet <= 0) || (iOffSet >= m_iMemSize))
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer_id is not valid, node offset=%d, memsize=%d.", iOffSet, m_iMemSize);
            return ERROR;
        }

	    TimerNode *pstTimerNode = (TimerNode *)(m_pMem + iOffSet);

	    if (pstTimerNode->uiPrev == 0)
	    {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer_id is not valid, node prev is 0, timer_id=%llu.", ullTimerID);
            return ERROR;
	    }

	    if (pstTimerNode->uiRandom != (unsigned int)iRandom)
	    {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "timer_id is not valid, random is not equ, node_random=%d, id_random=%d", pstTimerNode->uiRandom, iRandom);
            return TIMER_NOT_EXIST;
	    }

	    TimerNode *pstPrevTimerNode = (TimerNode *)(m_pMem + pstTimerNode->uiPrev);
	    TimerNode *pstNextTimerNode = NULL;
	    if (pstTimerNode->uiNext > 0)
	    {
	        pstNextTimerNode = (TimerNode *)(m_pMem + pstTimerNode->uiNext);
	    }

	    pstPrevTimerNode->uiNext = pstTimerNode->uiNext;

	    if (pstNextTimerNode)
	    {
	        pstNextTimerNode->uiPrev = pstTimerNode->uiPrev;
	    }

	    m_pstTPH->iTimerNodeNum--;
	    if (m_pstTPH->iTimerNodeNum < 0)
	    {
	        //TODO 这个是非常致命的错误
	        m_pstTPH->iTimerNodeNum = 0;
	    }

	    iRetVal = m_objTimerNodePool.ReleaseNode(pstTimerNode);
	    if (iRetVal != 0)
	    {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg), "release timer_node failed, ret=%d", iRetVal);
            return ERROR;
	    }

	    return SUCCESS;
	}

	/**
	 * @brief 将pstSrcTimerBucket上的定时器节点，迁移到pstDstTimerBucket后面iDstTimerBucketNum个对应的定时器桶中
	 * @param pstDstTimerBucket目标定时器桶的首指针
	 * @param iDstTimerBucketNum目标定时器桶的个数
	 * @param iDstMaskBits目标计算桶偏移的时候，末尾掩码的bit数量
	 * @param pstSrcTimerBucket需要迁移的定时器桶
	 * @return 0-成功 其他-失败
	 */
	int Migrate(TimerBucket *pstDstTimerBucket, int iDstTimerBucketNum, int iDstMaskBits, TimerBucket *pstSrcTimerBucket)
	{
	    if (pstSrcTimerBucket->uiNext == 0){
	        // 这个链上面没有节点，不需要迁移
	    	PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s|migrate, num=%d, mask=%d, %d, no node need migrate", __func__,
	    			iDstTimerBucketNum, iDstMaskBits, (char*)pstSrcTimerBucket - m_pMem );
	        return SUCCESS;
	    }

        TimerNode *pstTimerNode = (TimerNode *)(m_pMem + pstSrcTimerBucket->uiNext);
        TimerNode *pstPrevTimerNode = (TimerNode *)(m_pMem + pstTimerNode->uiPrev);
        TimerNode *pstNextTimerNode = (pstTimerNode->uiNext>0)?(TimerNode *)(m_pMem + pstTimerNode->uiNext):NULL;

        while (pstTimerNode != NULL){
            //转移节点
            struct timeval stTimeNow;
            gettimeofday(&stTimeNow, NULL);
            unsigned int uiInterval = 0;

            //找到正确的TB
            if(timercmp(&stTimeNow, &pstTimerNode->tExpire, <)){
                //计算出到期时间
                uiInterval = (pstTimerNode->tExpire.tv_sec - stTimeNow.tv_sec) * 1000 + ((pstTimerNode->tExpire.tv_usec - stTimeNow.tv_usec) / 1000);
            }

            PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s|Num|%d|Mask|%d|EXPIRE=%d.%d|NOW=%d.%d|INTERVAL=%d", __func__,
            		iDstTimerBucketNum, iDstMaskBits, (int)pstTimerNode->tExpire.tv_sec, (int)pstTimerNode->tExpire.tv_usec, (int)stTimeNow.tv_sec, (int)stTimeNow.tv_usec, uiInterval);

            uiInterval = uiInterval >> iDstMaskBits;
            if ((int)uiInterval >= iDstTimerBucketNum){
                PetLog(0, 0, PETLOG_ERR, "TimePool|%s|NUM|%d|Mask|%d|node interval is too big, interval=%d, scan=%d.%d, now=%d.%d, exp=%d.%d", __func__, iDstTimerBucketNum, iDstMaskBits,
                		uiInterval, m_pstTPH->stScanTime.tv_sec, m_pstTPH->stScanTime.tv_usec, stTimeNow.tv_sec, stTimeNow.tv_usec, pstTimerNode->tExpire.tv_sec, pstTimerNode->tExpire.tv_usec );
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
	 * @note 由于该接口驱动了整个时间轮的运转，必须保证每1ms调用一次
	 */
	int GetTimer(std::vector<unsigned long long> &vullTimerID, std::vector<DATATYPE> &vAllTimerParam)
	{
	    int iRetVal = 0;
	    struct timeval stTimeNow;
	    gettimeofday(&stTimeNow, NULL);
	    if(timercmp(&stTimeNow, &m_pstTPH->stScanTime, <)){
			PetLog(0, 0, PETLOG_ERR, "Now is less than scan time, use scan time instead" );
			stTimeNow = m_pstTPH->stScanTime;
	    }

	    int iTimeGap = (stTimeNow.tv_sec - m_pstTPH->stScanTime.tv_sec)*1000000 + (stTimeNow.tv_usec - m_pstTPH->stScanTime.tv_usec);

		// 如果没有节点，仅用更新扫描时间
	    if (m_pstTPH->iTimerNodeNum <= 0){
	    	int iTmpStep = iTimeGap / 1000;
	    	m_pstTPH->stScanTime.tv_usec += (iTmpStep % 1000)*1000;
	    	m_pstTPH->stScanTime.tv_sec += iTmpStep / 1000;
			return SUCCESS;
	    }

	    if (m_pstTPH->uiTB1CurPt >= (unsigned int)TB1_NUM){
	        PetLog(0, 0, PETLOG_ERR, "TimePool|%s|%d|tb1_cur_pt is not valid, tb1_cur_pt=%d", __func__, __LINE__, m_pstTPH->uiTB1CurPt);
            //TODO 这里可以考虑不退出，如果出现这种情况，保证系统还能继续，可以考虑先不退出，只是纠正
            m_pstTPH->uiTB1CurPt = 0;
	    }

	    // 每次检查TB1上所有链，把符合条件的全部取出来
	    PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s|H|NODE_NUM|%d|TBPT|%03u %03u %03u %03u %03u |SCAN|%d %d |GAP|%d| %d.%d - %d.%d", __func__, m_pstTPH->iTimerNodeNum, m_pstTPH->uiTB1CurPt, m_pstTPH->uiTB2CurPt, m_pstTPH->uiTB3CurPt, m_pstTPH->uiTB4CurPt, m_pstTPH->uiTB5CurPt,
	    	    		m_pstTPH->stScanTime.tv_sec, m_pstTPH->stScanTime.tv_usec, iTimeGap, stTimeNow.tv_sec,stTimeNow.tv_usec, m_pstTPH->stScanTime.tv_sec, m_pstTPH->stScanTime.tv_usec  );

	    while( (m_pstTPH->uiTB1CurPt < (unsigned int)TB1_NUM ) && (iTimeGap > 1000) ){
		    PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s|NODE_NUM|%d|TBPT|%03u %03u %03u %03u %03u |SCAN|%d %d |GAP|%d|", __func__, m_pstTPH->iTimerNodeNum, m_pstTPH->uiTB1CurPt, m_pstTPH->uiTB2CurPt, m_pstTPH->uiTB3CurPt, m_pstTPH->uiTB4CurPt, m_pstTPH->uiTB5CurPt,
		    	    		m_pstTPH->stScanTime.tv_sec, m_pstTPH->stScanTime.tv_usec, iTimeGap  );

	    	// 扫描一条链
			TimerBucket *pstTB1 = m_pstTB1 + m_pstTPH->uiTB1CurPt;

			//给扫描时间加1ms
			int iTmpUsec= m_pstTPH->stScanTime.tv_usec + 1000;
			m_pstTPH->stScanTime.tv_sec += iTmpUsec / 1000000;
			m_pstTPH->stScanTime.tv_usec = iTmpUsec % 1000000;

			//向前进1格
			m_pstTPH->uiTB1CurPt++;

			iTimeGap = (stTimeNow.tv_sec - m_pstTPH->stScanTime.tv_sec)*1000000 + (stTimeNow.tv_usec - m_pstTPH->stScanTime.tv_usec);

			if(pstTB1->uiNext == 0){
				//该链上面没有定时器节点
			}else{
				TimerNode *pstTimerNode = (TimerNode *)(m_pMem + pstTB1->uiNext);
				//TODO 这里需要对PREV进行检查
				TimerNode *pstPrevTimerNode = (TimerNode *)(m_pMem + pstTimerNode->uiPrev);
				TimerNode *pstNextTimerNode = (pstTimerNode->uiNext>0)?(TimerNode *)(m_pMem + pstTimerNode->uiNext):NULL;
				int iFirstNodeGap = (stTimeNow.tv_sec - pstTimerNode->tExpire.tv_sec)*1000000 + (stTimeNow.tv_usec - pstTimerNode->tExpire.tv_usec);

				while (pstTimerNode != NULL){
					vAllTimerParam.push_back(pstTimerNode->TimerParam);
					unsigned long long ullTimerID = 0;
					int iTimerNodeOffset = (char *)pstTimerNode - m_pMem;
					memcpy(&ullTimerID, &iTimerNodeOffset, sizeof(int));
					memcpy(((char *)&ullTimerID) + 4, &pstTimerNode->uiRandom, sizeof(int));
					vullTimerID.push_back(ullTimerID);

					PetLog(0, 0, PETLOG_DEBUG, "timer_pool|%s| ++++++ Get one node ++++++ |OFF|%d|ID|%llu|NOW|%d.%d|EXP|%d.%d|", __func__,
							iTimerNodeOffset, ullTimerID, stTimeNow.tv_sec, stTimeNow.tv_usec, pstTimerNode->tExpire.tv_sec, pstTimerNode->tExpire.tv_usec );

					//删除节点
					pstPrevTimerNode->uiNext = pstTimerNode->uiNext;
					if (pstNextTimerNode){
						pstNextTimerNode->uiPrev = pstTimerNode->uiPrev;
					}

					m_pstTPH->iTimerNodeNum--;
					if (m_pstTPH->iTimerNodeNum < 0){
						PetLog(0, 0, PETLOG_ERR, "TimePool|%s|%d|timer node in header is below zero, chang to zero", __func__, __LINE__);
						m_pstTPH->iTimerNodeNum = 0;
					}

					iRetVal = m_objTimerNodePool.ReleaseNode(pstTimerNode);
					if (iRetVal != 0){
						PetLog(0, 0, PETLOG_ERR, "TimePool|%s|%d|release node failed, ret=%d", __func__, __LINE__, iRetVal);
					}

					pstTimerNode = pstNextTimerNode;
					pstNextTimerNode = NULL;
					if(pstTimerNode != NULL){
						pstNextTimerNode = (pstTimerNode->uiNext>0)?(TimerNode *)(m_pMem + pstTimerNode->uiNext):NULL;
					}
				}
			}

			//进行时间轮转动
			if (m_pstTPH->uiTB1CurPt == (unsigned int)TB1_NUM)
			{
				//节点迁移
				//每次迁移保证TB2上的某条链全部迁移完毕，超过TB1最大时长的，按照TB1中最大时长计算
				if (m_pstTPH->uiTB2CurPt >= (unsigned int)TB2_NUM)
				{
					PetLog(0, 0, PETLOG_ERR, "TimePool|%s|%d|tb2_cur_pt is not valid, tb2_cur_pt=%d", __func__, __LINE__, m_pstTPH->uiTB2CurPt);
					//TODO 这里可以考虑不退出，如果出现这种情况，保证系统还能继续，可以考虑先不退出，只是纠正
					m_pstTPH->uiTB2CurPt = 0;
				}

				//TB2->TB1
				Migrate(m_pstTB1, TB1_NUM, 0, m_pstTB2 + m_pstTPH->uiTB2CurPt);
				m_pstTPH->uiTB1CurPt = 0;
				m_pstTPH->uiTB2CurPt++;

				if (m_pstTPH->uiTB2CurPt == (unsigned int)TB2_NUM)
				{
					if (m_pstTPH->uiTB3CurPt >= (unsigned int)TB3_NUM)
					{
                    PetLog(0, 0, PETLOG_ERR, "TimePool|%s|%d|tb3_cur_pt is not valid, tb3_cur_pt=%d", __func__, __LINE__, m_pstTPH->uiTB3CurPt);
						//TODO 这里可以考虑不退出，如果出现这种情况，保证系统还能继续，可以考虑先不退出，只是纠正
						m_pstTPH->uiTB3CurPt = 0;
					}

					//TB3->TB2
					Migrate(m_pstTB2, TB2_NUM, TB1_BIT, m_pstTB3 + m_pstTPH->uiTB3CurPt);
					m_pstTPH->uiTB2CurPt = 0;
					m_pstTPH->uiTB3CurPt++;

					if (m_pstTPH->uiTB3CurPt == (unsigned int)TB3_NUM)
					{
						if (m_pstTPH->uiTB4CurPt >= (unsigned int)TB4_NUM)
						{
                        PetLog(0, 0, PETLOG_ERR, "TimePool|%s|%d|tb4_cur_pt is not valid, tb4_cur_pt=%d", __func__, __LINE__, m_pstTPH->uiTB4CurPt);
							//TODO 这里可以考虑不退出，如果出现这种情况，保证系统还能继续，可以考虑先不退出，只是纠正
							m_pstTPH->uiTB4CurPt = 0;
						}

						//TB4->TB3
						Migrate(m_pstTB3, TB3_NUM, TB2_BIT+TB1_BIT, m_pstTB4 + m_pstTPH->uiTB4CurPt);
						m_pstTPH->uiTB3CurPt = 0;
						m_pstTPH->uiTB4CurPt++;

						if (m_pstTPH->uiTB4CurPt == (unsigned int)TB4_NUM)
						{
							if (m_pstTPH->uiTB5CurPt >= (unsigned int)TB5_NUM)
							{
                            PetLog(0, 0, PETLOG_ERR, "TimePool|%s|%d|tb5_cur_pt is not valid, tb5_cur_pt=%d", __func__, __LINE__, m_pstTPH->uiTB5CurPt);
								//TODO 这里可以考虑不退出，如果出现这种情况，保证系统还能继续，可以考虑先不退出，只是纠正
								m_pstTPH->uiTB5CurPt = 0;
							}

							//TB5->TB4
							Migrate(m_pstTB4, TB4_NUM, TB3_BIT+TB2_BIT+TB1_BIT, m_pstTB5 + m_pstTPH->uiTB5CurPt);
							m_pstTPH->uiTB4CurPt = 0;
							m_pstTPH->uiTB5CurPt++;

							m_pstTPH->uiTB5CurPt = m_pstTPH->uiTB5CurPt%TB5_NUM;
						}
					}
				}
			}
	    }

        return SUCCESS;
	}

	/**
	 * @brief 校验内存中数据的有效性
	 *
	 * @returen 0-成功 其他-失败
	 */
	int Verify()
	{
	    return SUCCESS;
	}

	void ShowInfo()
	{
	    printf("=========TimerPoolHeader=========");
	    printf("TB_CUR_PT|%d|%d|%d|%d|%d", m_pstTPH->uiTB1CurPt, m_pstTPH->uiTB2CurPt, m_pstTPH->uiTB3CurPt, m_pstTPH->uiTB4CurPt, m_pstTPH->uiTB5CurPt);
	    m_objTimerNodePool.Show();
	}

};

}

#endif

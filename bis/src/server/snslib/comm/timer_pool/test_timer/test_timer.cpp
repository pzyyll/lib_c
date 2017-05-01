/**
 * @file    test_timer.cpp
 * @brief   测试timer_pool
 * @author  smiyang@tencent.com
 * @date    2010-08-18
  */
//============================================================================
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <iostream>
#include <fstream>

#include "comm/timer_pool/timer_pool.h"

using namespace std;
using namespace snslib;

typedef struct tagTimerParamInfo
{
    unsigned int uiUin;
    unsigned int uiModuleID;    //根据模块名，根据该值，确定是那个部分设置的定时器(APP or TRANSACTION)
    unsigned int uiParam;       //4字节参数
    char szUnUsed[28];
}TimerParamInfo;

int main(int argc, char* argv[])
{
     //OpenPetLog("timer");

	//timer_pool内存初始化
    // 在这里并没有使用到共享内存
    int iTimerShmKey = 13333;
    int iTimerShmSize = 80*1024*1024;
    int iRetVal = 0;

    CTimerPool<TimerParamInfo> * m_pobjTimerPool = new CTimerPool<TimerParamInfo>();
    if (m_pobjTimerPool == NULL)
    {
        printf( "new CTimerPool failed, errno=%d, errmsg=%s", errno, strerror(errno));
        return -1;
    }

    //初始化定时器渠
    void *pvMem = malloc(iTimerShmSize);
    iRetVal = m_pobjTimerPool->Init(pvMem, iTimerShmSize, 1);
    if(iRetVal != 0){
    	printf("TimerPool Init failed, ret=%d\n", iRetVal);
    	return -1;
    }

    //测试1秒能够添加多少节点
    clock_t start = clock();
    double  dDuration = 0;
    unsigned long long ullCount = 0, ullLostCount = 0;
    while( dDuration < 1 ){
    	TimerParamInfo stTimerParamInfo;
    	memset(&stTimerParamInfo, 0x0, sizeof(TimerParamInfo));
    	stTimerParamInfo.uiUin = 0;
    	stTimerParamInfo.uiModuleID = 1;
    	stTimerParamInfo.uiParam = 2;

    	unsigned long long ullTimerID = 0;

    	iRetVal = m_pobjTimerPool->AddTimer( 100*10, stTimerParamInfo, &ullTimerID);
    	if(iRetVal != 0){
    		printf("add TimerNode failed, ret=%d\n", iRetVal);
    		++ullLostCount;
    	}else{
    		++ullCount;
    	}

    	clock_t end = clock();
    	dDuration = (double)(end-start)/CLOCKS_PER_SEC;
    }

    cout << "时间段长为： " << dDuration << "成功添加节点个数为： " << ullCount << "添加节点失败个数为："<< ullLostCount<< endl;


    //取出节点
    unsigned long long ullGetCount = 0;
    ofstream outfile("test_timer.txt", iostream::app);
    std::vector<TimerParamInfo> vstTimerParam;
    std::vector<unsigned long long> vullTimerID;
    while(ullCount > 0){
    	vstTimerParam.clear();
        vullTimerID.clear();
        m_pobjTimerPool->GetTimer(vullTimerID, vstTimerParam);
        unsigned int uiH, uiM, uiS;
        //此函数得到时间轮的3个指针偏移量，为在原来类的基础上后添加的函数。
        //编译是无法通过的，因为该函数已经删除，可以添加该函数解决问题，以便观察指针的偏移量
        m_pobjTimerPool->GetHMS(uiH, uiM, uiS);
        if(vstTimerParam.size() != 0){
           outfile << "时针偏移量：" << uiH << " 分针偏移量：" << uiM << " J秒针偏移量：" << uiS <<endl;
           outfile << "本次触发个数：" << vstTimerParam.size() << endl;
           ullCount -= vstTimerParam.size();
           ullGetCount += vstTimerParam.size();
        }
    }
    cout << "共取出节点个数为：" << ullGetCount << endl;

    return 0;
}

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "process_manager.h"

using namespace  snslib;
/*!==========================================================
//Function:    CProcess_Manager
//Description: constructor
//ParamInfo: 
//Date/Author: Ray.Xia				2006/03/13
//Modified:
//ReturnCode:
//=========================================================*/
CProcess_Manager::CProcess_Manager( unsigned int iChildMinNum, unsigned int iChildMaxNum )
{
	m_iChildMinNum		= iChildMinNum;
	m_iChildMaxNum		= iChildMaxNum;
}

/*!==========================================================
//Function:    ~CProcess_Manager
//Description: destructor
//ParamInfo: 
//Date/Author: Ray.Xia				2006/03/13
//Modified:
//ReturnCode:
//=========================================================*/
CProcess_Manager::~CProcess_Manager()
{
}

void CProcess_Manager::Init(unsigned int iChildMinNum, unsigned int iChildMaxNum)
{
	m_iChildMinNum = iChildMinNum;
	m_iChildMaxNum = iChildMaxNum;
}

/*!==========================================================
//Function:    Run
//Description: 开始Process Manager运行
//ParamInfo: 
//Date/Author: Ray.Xia				2006/03/13
//Modified:
//ReturnCode:
//=========================================================*/
int CProcess_Manager::Run( int argc, char *argv[] )
{
	//create process entity
	for ( int i = 0; i < m_iChildMinNum; )
	{
		int iPid = fork();
		if ( iPid < 0 )
		{
			printf( "ERR: CProcess_Manager::Run() fork failed! ERRMSG:%s\n", strerror( errno ) );
			continue;
		}
		else if ( iPid > 0 )
		{
			i++;
		}
		else if ( iPid == 0 )
		{
			int iRet = Entity( argc, argv );
			if ( iRet < 0 )
			{
				printf( "ERR: CProcess_Manager::Entity() return < 0. RetValue = %d\n", iRet );
			}

			_exit( iRet );
		}
	}

	while(1)
	{
		//monitor all child processes
		int iStatus = 0;
		int iPid = waitpid( -1, &iStatus, WUNTRACED );
		if ( WIFEXITED( iStatus ) != 0 )
		{
			printf( "NOTICE: -----PID: %d exited normally!\n", iPid );
		}
		else if ( WIFSIGNALED( iStatus ) != 0 )
		{
			printf( "NOTICE: -----PID: %d exited bacause of signal ID: [%d] has not been catched!\n", iPid, WTERMSIG( iStatus ) );			
		}
		else if ( WIFSTOPPED( iStatus ) != 0 )
		{
			printf( "NOTICE: -----PID: %d exited because of stoped! ID: [%d]\n", iPid, WSTOPSIG( iStatus ) );
			
		}
		else
		{
			printf( "NOTICE: -----PID: %d exited abnormally!\n", iPid );
		}

		//Add Entity
		AddEntity( argc, argv );
	}

	return SUCCESS;
}

/*!==========================================================
//Function:    Entity
//Description: 程序实体，实体异常推出或者需要新的处理实体时此函数会被调用
//ParamInfo: 
//Date/Author: Ray.Xia				2006/03/13
//Modified:
//ReturnCode:
//=========================================================*/
int CProcess_Manager::Entity( int argc, char *argv[] )
{
	printf( "PID[%d] ================ Entity is running ===============\n", getpid() );

	while( 1 )
	{
		printf( "PID[%d]----I am live----%ld-----\n", getpid(), time(NULL) );
		sleep( 5 );
	}
	
	return SUCCESS;
}

/*!==========================================================
//Function:    AddEntity
//Description: 补充运行实体
//ParamInfo: 
//Date/Author: Ray.Xia				2006/03/13
//Modified:
//ReturnCode:
//=========================================================*/
int CProcess_Manager::AddEntity( int argc, char *argv[] )
{
	int iPid = fork();
	if ( iPid < 0 )
	{
		printf( "ERR: CProcess_Manager::Run() fork failed! ERRMSG:%s\n", strerror( errno ) );
		return ERROR;
	}
	else if ( iPid == 0 )
	{
		int iRet = Entity( argc, argv );
		if ( iRet < 0 )
		{
			printf( "ERR: CProcess_Manager::Entity() return < 0. RetValue = %d\n", iRet );
		}

		printf( "NOTICE: ----Add Entity: %d----\n", getpid() );

		_exit( iRet );
	}

	return SUCCESS;
}


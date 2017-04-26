
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include "map_serialize.h"
#include <iostream>


#define PetLog(iLogID, uiUin, iLogLevel, szFormat, args...) printf(szFormat, ##args);

using namespace std;
using namespace snslib;


const char*  filename = "map_test.dat";
const int   filesize = 1024000;

typedef struct {
	int	id;
	char	 name[10];
}UserProfile;

typedef void(*TestFunc)(int argc, char *argv[]);

typedef struct tagCmdDef
{
	char szName[64];
	TestFunc pFunc;
	int iParamCount;
	char szParamsDesc[64];
} CMD_DEF;


//全局啊。。
static snslib::CMapSerialize<UserProfile>  obj;



void TestGetInfo(int argc, char* argv[])
{
	cout<<"===info==="<<endl;
	cout<<"total num = "<<obj.size()<<endl;
	cout<<"===info end==="<<endl;

}


void TestInsert(int argc, char* argv[])
{
	UserProfile	user;
	user.id = atoi(argv[2]);
	snprintf(user.name,sizeof(user.name),"%s",argv[3]);

	int iRet = obj.insert(user.id,user);
	if(0 != iRet)
	{
		cout<<"insert["<<user.id <<"] err."<< iRet<<endl;
	}
	else
	{	
		cout<<"insert["<<user.id <<"] succ!"<<endl;
	}

}


void TestFind(int argc, char* argv[])
{
	int id =  atoi(argv[2]);
	UserProfile	user;

	int iRet = obj.find(id,user);
	if(0 != iRet)
	{
		cout<<"find["<<id <<"] err."<< iRet<<endl;
	}
	else
	{	
		cout<<"find["<<id <<"] succ!"<<endl;
		cout<<"id = "<<user.id <<"\tname = "<<user.name<<endl;
	}

}

void TestDel(int argc, char* argv[])
{
	int id =  atoi(argv[2]);
	UserProfile	user;

	int iRet = obj.erase(id,user);
	if(0 != iRet)
	{
		cout<<"erase["<<id <<"] err."<< iRet<<endl;
	}
	else
	{
		cout<<"id = "<<user.id <<"\tname = "<<user.name<<endl;
		cout<<"erase["<<id <<"] succ!"<<endl;
	}

}

const CMD_DEF astCmdDef[] =
{
	{"查询全局数据", TestGetInfo, 0, ""},
	{"插入一条数据", TestInsert, 2, "id  name"},		
	{"查找数据    ", TestFind, 1, "id"},
	{"删除数据    ", TestDel, 1, "id"}

};

void PrintCmdInfo()
{
	cout<<"Usage:"<<endl;
	for(int i = 0;  i < sizeof(astCmdDef)/sizeof(CMD_DEF); i++)
	{
		cout<<"\t"<<i+1<<"\t"<<astCmdDef[i].szName <<"\t"<<astCmdDef[i].szParamsDesc<<endl;
	}
}


int main(int argc , char* argv[])
{

	cout<<"==begin=="<<endl;
	if(argc<2)
	{
		PrintCmdInfo();
		return -1;
	}
	

	int fd = open(filename, O_CREAT | O_RDWR ,  S_IRWXU);
	if(fd <0 )
	{
		printf("open %s failed. err=%d,msg=%s\n",filename,errno,strerror(errno));
		exit(-1);
	}

	//没有下面这句，出个总线错误，原因未知
	ftruncate(fd,filesize);

	cout<<"open "<<filename <<" succ!!"<<endl;

	void* pvMem = mmap(0, filesize, PROT_READ|PROT_WRITE, MAP_SHARED , fd, 0);

	if( MAP_FAILED == pvMem)
	{
		printf("mmap  failed. err=%d,msg=%s\n",errno,strerror(errno));
		exit(-1);
	}

	cout<<"mmap  succ!!"<<endl;

	
	int iRet =  obj.init((char*)pvMem, filesize, 0);
	if(0 != iRet)
	{
		printf("init(0) failed,ret=%d\n",iRet);
		
		//使用非清除的init失败了。再使用带清除的来初始化
		iRet =  obj.init((char*)pvMem, filesize, 1);
		if(0 != iRet)
		{
			printf("init(1) failed,ret=%d\n",iRet);
			exit(-1);
		}

	}

	cout<<"obj init  succ!!"<<endl;

	//检查输入的参数
	int iCmdParam = atoi(argv[1]);
	if( iCmdParam <0  || iCmdParam > sizeof(astCmdDef)/sizeof(CMD_DEF) )
	{
		cout<<"err cmd = "<<iCmdParam <<endl;
		return -1;
	}
	
	if(argc < astCmdDef[iCmdParam-1].iParamCount+2)
	{
		cout<<"err param  num  "<<endl;
		return -1;
	}

	cout<<"check input  succ!!"<<endl;
	
	//go
	astCmdDef[iCmdParam-1].pFunc(argc,argv);


	msync(pvMem,filesize,MS_SYNC);
	return 0;
}
	

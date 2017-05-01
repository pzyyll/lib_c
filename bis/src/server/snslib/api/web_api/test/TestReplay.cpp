#include<iostream>
#include"../webapi.h"
     
using namespace std;
using namespace snslib;
int main()
{
	snslib::CWebApi  webapi;
	webapi.Init("webapi.init");


	unsigned unUin = 979762787;
    int ret=0;
	//int ret = webapi.AddGrowth(unUin,1000,"add 1000");
	//if(0 != ret)
	//	cout<<"ret= "<<ret<<"  msg = "<<webapi.GetErrMsg()<<endl;
	//else
	//	cout<<"AddGrowth succ"<<endl;

/*
	kongfupet::GetReplayListResponse replayList;
	cout<<"test GetReplayList"<<endl;
	ret = webapi.GetReplayList(unUin,replayList);
	if(0 != ret)
	{
	  cout<<"GetReplayList failed ret = "<<ret<<endl;
	  cout<<webapi.GetErrMsg()<<endl;
	}
	else
	{
		//cout<<"Â¼ÏñÁÐ±í¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£"<<endl;
		//replayList.PrintDebugString();
		//cout<<"\n\n"<<endl;
		
		
		cout<<"test GetReplay"<<endl;
		unsigned replay_id = replayList.replays(0).replay_id();
		unsigned fight_time = replayList.replays(0).fight_time();
		kongfupet::GetReplayResponse stReplay;
		ret = webapi.GetReplay(unUin,replay_id,fight_time,stReplay);
		if(0 != ret)
		{
			cout<<"GetReplay failed ret = "<<ret<<endl;
			  cout<<webapi.GetErrMsg()<<endl;
		}
		else
		{
			cout<<"µÚÒ»¸öÂ¼Ïñ¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£¡£"<<endl;
			stReplay.PrintDebugString();
			cout<<"\n\n"<<endl;
		}

	}*/ 

	kongfupet::WebFeeds feeds;
	cout<<"test GetWebFeeds-----------"<<endl;
	ret = webapi.GetWebFeeds(unUin,feeds);
	cout<<"ret = "<<ret<<endl;
	feeds.PrintDebugString();


 return 0;
}


//
// Created by czllo on 2017/4/26.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <unordered_map>

#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <ctime>

#include "mq_mgr.h"
#include "server_config.h"
#include "singleton.h"
#include "timer_heap.h"
#include "application_base.h"
#include "proto/bs_msg.pb.h"
#include "proto/bs_cmd.pb.h"
#include "proto/bs_errcode.pb.h"
#include "db.h"

class Server : public tnt::ApplicationBase {
public:
    // ApplicationBase
    int OnInit(const char* conf_file) override;
    int OnProc() override;
    int OnTick() override;
    int OnExit() override;
    int OnStop() override;
    int OnIdle() override;

    void CloseAllListenSockets();

private:
    int ProcessMQRecv();
    void ProcessMQMsg(const std::string &data);

    void ProcessDbSet();
    void ProcessDbGetScore();
    void ProcessDbRankQuery();
    void ProcessDbRangeByRank();
    void ProcessDbRangeByScore();
    void ProcessDbTopQuery();

    int Parse(::google::protobuf::Message &msg);

    int Respone(const ::google::protobuf::Message &data);

    //TODO 移除长时间没有响应的连接
private:
    bs_czl::MsgTransApp transApp;
public:
    int WaitChilen();

    time_t uiLastSaveTime;
    pid_t chilld_pid;
    time_t uiForkStartTime;
    time_t uiSaveTimeSpan;
};

typedef Singleton<Server> SingletonSvr;
#define LPSGLSVR SingletonSvr::instance()

#endif //SERVER_SERVER_H

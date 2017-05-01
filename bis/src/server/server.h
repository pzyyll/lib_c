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

#include "server_config.h"
#include "cepoll.h"
#include "task.h"
#include "singleton.h"
#include "timer_heap.h"

class Server {
    static const unsigned int LISTEN_TASK_POS = 0;
    typedef std::unordered_map<unsigned int, TaskPtr> TaskMap;
    typedef TaskMap::iterator TaskMapItr;
public:
    Server();
    ~Server();

    int Run();

    void StopService();

private:
    int Init();
    int StartListen();
    int MakeNonBlocking(int fd);

    void HldAllEvs(struct epoll_event *evs, int nds);
    int AcceptTask();
    TaskPtr FindTask(unsigned int pos);
    void RemoveTask(unsigned int pos);
    void RemoveTask(TaskMapItr &itr);

    //TODO 定时器操作
    void AddTime(struct timeval &tvTime, int milliseconds);
    void AddTimeout(ExpireTimer& timer);
    int RemoveTimer(unsigned long long ullTimerID);
    void ExpireTimers(struct timeval *next_timeout);

private:
    bool bLoop_;

    //listen fd
    int iSock_;
    //epoll handle
    unsigned int iPosCnt;

    TaskMap objTaskMap;
    TimerHeap objTimerHeap;
};

typedef Singleton<Server> SingletonSvr;
#define LPSGLSVR SingletonSvr::instance()

#endif //SERVER_SERVER_H

//
// Created by czllo on 2017/5/3.
//

#ifndef PROXY_MQ_MGR_H
#define PROXY_MQ_MGR_H

#include <vector>
#include <unordered_map>
#include <zmq.h>
#include "server_config.h"
#include "log_def.h"
#include "singleton.h"
#include "proto/bs_msg.pb.h"

class mq_mgr {
    typedef std::unordered_map<std::string, void *> DestMap;
    typedef DestMap::iterator DestMapItr;
public:
    mq_mgr();
    ~mq_mgr();

    int Init();
    int SendMsg(const std::string &dest, ::google::protobuf::Message &msg);
    int RecvMsgs(std::vector<std::string> &msgs);
    int RecvMsgs(std::vector<std::string> &msgs, const int max_size);
    int RecvMsg(std::string &strMsg);

    int RemvConn(const std::string &dest);

    void RemvConn(const DestMapItr &itr);

    void CloseAll();

private:
    void *FindConn(const std::string &dest);
    void *AddDestConn(const std::string &dest);

private:
    void *vpContext;
    void *vpPull_sk;
    //void *vpPush_sk;
    DestMap objPushSks;
};

typedef Singleton<mq_mgr> SingletonMQMgr;
#define LPMQMGR SingletonMQMgr::instance()

#endif //PROXY_MQ_MGR_H

//
// Created by czllo on 2017/5/3.
//

#ifndef PROXY_MQ_MGR_H
#define PROXY_MQ_MGR_H

#include <vector>
#include <zmq.h>
#include "server_config.h"
#include "log_def.h"
#include "singleton.h"
#include "proto/bs_msg.pb.h"

class mq_mgr {
public:
    mq_mgr();
    ~mq_mgr();

    int Init();
    int SendMsg(const std::string &key, ::google::protobuf::Message &msg);
    int RecvMsgs(std::vector<std::string> &msgs);
    int RecvMsgs(std::vector<std::string> &msgs, const int max_size);
    int RecvMsg(std::string &strMsg);

private:
    int GetIndex(const std::string &key);
    unsigned int Hash(const std::string &strKey);
private:
    void *vpContext;
    void *vpPull_sk;
    std::vector<void *> push_sks;
};

typedef Singleton<mq_mgr> SingletonMQMgr;
#define LPMQMGR SingletonMQMgr::instance()

#endif //PROXY_MQ_MGR_H

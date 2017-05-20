//
// Created by czllo on 2017/5/3.
//

#include "mq_mgr.h"

using namespace std;

mq_mgr::mq_mgr() : vpContext(NULL), vpPull_sk(NULL) {

}

mq_mgr::~mq_mgr() {
    CloseAll();
}

int mq_mgr::Init() {
    int ret = SUCCSESS;

    vpContext = zmq_ctx_new();
    vpPull_sk = zmq_socket(vpContext, ZMQ_PULL);
    ret = zmq_bind(vpPull_sk, LPSVRCFG->pull_addr_.c_str());
    if (ret < 0) {
        LOG(WARNING) << "bind to mq pull fail|" << LPSVRCFG->pull_addr_
                     << "|errno " << errno <<  endl;
        return ret;
    }

    LOG(INFO) << "Init MQ SUCC" << endl;
    return 0;
}

int mq_mgr::SendMsg(const std::string &dest, ::google::protobuf::Message &msg) {
    int ret = SUCCSESS;

    //@CZL 先在Map里找已经建立连接的Proxy, 没有在添加
    void *push_sk = FindConn(dest);
    if (NULL == push_sk) {
        push_sk = AddDestConn(dest);
        if (NULL == push_sk)
            return FAIL;
    }

    LOG(INFO) << "pre to send msg : " << msg.ShortDebugString() << endl;

    string data = msg.SerializeAsString();

    zmq_msg_t zmq_msg;
    zmq_msg_init_size(&zmq_msg, data.size());
    memcpy(zmq_msg_data(&zmq_msg), data.c_str(), data.size());

    ret = zmq_msg_send(&zmq_msg, push_sk, ZMQ_DONTWAIT);
    if (ret < 0) {
        if (EAGAIN == errno) {
            LOG(INFO) << "Send Msg Buf is Full. Please Check Recv Svr is live" << endl;
        }
        LOG(INFO) << "Send Msg Fail. " << errno << endl;
        zmq_msg_close(&zmq_msg);
    }

    return ret;
}

int mq_mgr::RecvMsgs(std::vector<std::string> &msgs) {
    int ret = SUCCSESS;

    while (true) {
        string msg;
        if (RecvMsg(msg) <= 0) {
            break;
        }
        msgs.push_back(msg);
    }
    return msgs.size();
}

int mq_mgr::RecvMsgs(std::vector<std::string> &msgs, const int max_size) {
    int ret = SUCCSESS;
    int cnt  = max_size;
    while (cnt--) {
        string msg;
        if (RecvMsg(msg) <= 0) {
            break;
        }
        msgs.push_back(msg);
    }

    return msgs.size();
}

int mq_mgr::RecvMsg(std::string &strMsg) {
    int ret = SUCCSESS;

    zmq_msg_t msg;
    zmq_msg_init(&msg);
    int rv = zmq_msg_recv(&msg, vpPull_sk, ZMQ_DONTWAIT);
    if (rv < 0) {
        if (EAGAIN != errno) {
            LOG(ERROR) << "No Data Recv" << endl;
            ret = FAIL;
        }
        //LOG(INFO) << "no data" << endl;
        return 0;
    }

    LOG(INFO) << "Recv data len = " << rv << endl;

    strMsg.assign((char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
    zmq_msg_close(&msg);

    return strMsg.size();
}

void *mq_mgr::FindConn(const std::string &dest) {
    DestMapItr itr = objPushSks.find(dest);
    if (objPushSks.end() == itr) {
        return NULL;
    }
    LOG(INFO) << "FIND DEST : " << itr->first << endl;
    return itr->second;
}

void *mq_mgr::AddDestConn(const std::string &dest) {
    int ret = SUCCSESS;

    void *push_sk = zmq_socket(vpContext, ZMQ_PUSH);
    ret = zmq_connect(push_sk, dest.c_str());
    if (ret < 0) {
        LOG(WARNING) << "conn to " << dest << " Fail. err=" << errno << endl;
        return NULL;
    }
    LOG(INFO) << "Conn To " << dest << endl;

    objPushSks.insert(make_pair(dest, push_sk));

    LOG(INFO) << "Now Conn Size : " << objPushSks.size() << endl;

    return push_sk;
}

int mq_mgr::RemvConn(const std::string &dest) {
    DestMapItr itr = objPushSks.find(dest);
    if (objPushSks.end() == itr) {
        return FAIL;
    }
    RemvConn(itr);
    return 0;
}

void mq_mgr::RemvConn(const DestMapItr &itr) {
    if (itr->second)
        zmq_close(itr->second);
    //TODO 移除前先把队列里的信息发送出去
    objPushSks.erase(itr);
}

void mq_mgr::CloseAll() {
    if (vpPull_sk)
        zmq_close(vpPull_sk);

    //TODO Remv all conn
    for (DestMapItr itr = objPushSks.begin(); itr != objPushSks.end(); ++itr) {
        zmq_close(itr->second);
    }

    if (vpContext) {
        zmq_ctx_destroy(vpContext);
    }
    vpPull_sk = NULL;
    vpContext = NULL;
    objPushSks.clear();
}



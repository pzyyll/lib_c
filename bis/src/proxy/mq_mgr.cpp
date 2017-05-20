//
// Created by czllo on 2017/5/3.
//

#include "mq_mgr.h"

using namespace std;

mq_mgr::mq_mgr() : vpContext(NULL), vpPull_sk(NULL) {

}

mq_mgr::~mq_mgr() {
    if (vpPull_sk)
        zmq_close(vpPull_sk);
    for (int i = 0; i < push_sks.size(); ++i) {
        zmq_close(push_sks[i]);
    }
    if (vpContext) {
        zmq_ctx_destroy(vpContext);
    }
    vpPull_sk = NULL;
    vpContext = NULL;
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

    for (int i = 0; i < static_cast<int>(LPSVRCFG->push_addrs_.size()); ++i) {
        void *push_sk = zmq_socket(vpContext, ZMQ_PUSH);
        ret = zmq_connect(push_sk, LPSVRCFG->push_addrs_[i].c_str());
        if (ret < 0) {
            LOG(WARNING) << "conn to mq push fail|" << LPSVRCFG->push_addrs_[i] << endl;
            return ret;
        }
        push_sks.push_back(push_sk);
    }

    if (push_sks.size() <= 0) {
        LOG(WARNING) << "No One Push MQ Can Push!!" << endl;
        return FAIL;
    }

    LOG(INFO) << "Init MQ SUCC" << endl;
    return 0;
}

int mq_mgr::SendMsg(const std::string &key, ::google::protobuf::Message &msg) {
    int ret = SUCCSESS;

    if (push_sks.size() <= 0) {
        LOG(WARNING) << "No One Push Svr" << endl;
        return FAIL;
    }

    string data = msg.SerializeAsString();

    //根据键值哈希一个服务器
    int idx = GetIndex(key);
    void *push_sk = push_sks[idx];

    LOG(INFO) << "Get Idx " << idx << endl;

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

    LOG(INFO) << "Send To Que" << endl;
    return ret;
}

int mq_mgr::RecvMsgs(std::vector<std::string> &msgs) {
    int ret = SUCCSESS;

    while (true) {
        string msg;
        if (RecvMsg(msg) <= 0) {
            break;
        }
        LOG(INFO) << "One Recv Msg :" << msg << endl;
        msgs.push_back(msg);
    }
    //LOG(INFO) << "Recv size " << msgs.size() << endl;
    return static_cast<int>(msgs.size());
}

int mq_mgr::RecvMsgs(std::vector<std::string> &msgs, const int max_size) {
    int ret = SUCCSESS;
    int cnt  = max_size;
    while (cnt--) {
        string msg;
        if (RecvMsg(msg) <= 0) {
            break;
        }
        LOG(INFO) << "One Recv Msg :" << msg << endl;
        msgs.push_back(msg);
    }

    //LOG(INFO) << "Recv size " << msgs.size() << endl;
    return static_cast<int>(msgs.size());
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

    return static_cast<int>(strMsg.size());
}

int mq_mgr::GetIndex(const std::string &key) {
    int buck_num = LPSVRCFG->bucket_num_;
    int buck_val = Hash(key) % buck_num;
    int per = buck_num / static_cast<int>(push_sks.size());
    int idx = 0;
    int sum = per;
    for (; static_cast<unsigned int>(idx) < push_sks.size() - 1; ++idx) {
        if (buck_val < sum)
            break;
        sum += per;
    }
    return idx;
}

unsigned int mq_mgr::Hash(const std::string &strKey) {
    int len = static_cast<int>(strKey.size());
    const char *key = strKey.c_str();
    /* 'm' and 'r' are mixing constants generated offline.
     They're not really 'magic', they just happen to work well.  */
    uint32_t seed = 5381;
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    /* Initialize the hash to a 'random' value */
    uint32_t h = seed ^ len;

    /* Mix 4 bytes at a time into the hash */
    const unsigned char *data = (const unsigned char *)key;

    while(len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    /* Handle the last few bytes of the input array  */
    switch(len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0]; h *= m;
    };

    /* Do a few final mixes of the hash to ensure the last few
     * bytes are well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (unsigned int)h;
}

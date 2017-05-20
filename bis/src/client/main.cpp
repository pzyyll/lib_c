//
// Created by czllo on 2017/5/1.
//

#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <netinet/in.h>
#include <cassert>

#include "pet_util.h"
#include "epoll_client.h"
#include "tcp_client.h"
#include "proto/msg_head.h"
#include "proto/bs_errcode.pb.h"
#include "proto/bs_cmd.pb.h"
#include "proto/bs_msg.pb.h"
#include "proto/bs_db.pb.h"

using namespace std;
using namespace snslib;
//using namespace bs_czl;

TcpClient cli;
const unsigned int MAX_BUFF_LEN = 1024*1000;
char sendBuff[MAX_BUFF_LEN];
char rvBuf[MAX_BUFF_LEN];
char input[MAX_BUFF_LEN];

int InitClinetConn(int argc, char **argv) {

    if (cli.init(argv[1], atoi(argv[2])) < 0) {
        cout << cli.get_err_msg() << endl;
        return -1;
    }

    cli.set_rw_timeout(500);

    return 0;
}

int DelSpaceStr(vector<string> &strs) {
    for (vector<string>::iterator itr = strs.begin(); itr != strs.end(); ) {
        if (itr->empty()) {
            itr = strs.erase(itr);
        } else {
            ++itr;
        }
    }
    return 0;
}

int send_and_recv(unsigned short cmd, const ::google::protobuf::Message &req, ::google::protobuf::Message &rsp) {
    string str_req = req.SerializeAsString();
    MsgHead *head = (MsgHead *)sendBuff;
    head->cmd_id = htons(cmd);
    head->msg_len = htonl(sizeof(MsgHead) + str_req.size());
    memcpy(sendBuff + sizeof(MsgHead), req.SerializeAsString().c_str(), req.SerializeAsString().size());

    unsigned int rv = MAX_BUFF_LEN;
    if (cli.check_connect() < 0) {
        cli.reconnect();
    }
    if (cli.send_and_recv(sendBuff, sizeof(MsgHead) + str_req.size(), rvBuf, rv) < 0)
        return -1;

    int data_len = 0;
    if (rv > sizeof(MsgHead)) {
        head = (MsgHead *)rvBuf;
        data_len = ntohl(head->msg_len) - sizeof(MsgHead);
        if ((unsigned int)data_len > rv) {
            cout << "rv data_len invaild. " << data_len << endl;
            return -1;
        }
    }
    if (!rsp.ParseFromArray(rvBuf + sizeof(MsgHead), data_len)) {
        //cout << "rv data fail. " << rv << endl;
        return -1;
    }
    return 0;
}

int bs_sets(const vector<string> &argv) {
    bs_czl::MsgSetBatchReq req;
    bs_czl::MsgSetBatchRsp rsp;

    int kvsize = argv.size() - 2;

    if (kvsize <= 0 || (kvsize % 2)) {
        cout << "参数错误！" << endl;
        return -1;
    }

    req.set_key_id(argv[1]);
    for (int i = 2; i < argv.size(); i += 2) {
        bs_czl::MsgSetMemData *item = req.add_data_list();
        assert(item != NULL);
        item->set_score(stod(argv[i]));
        item->set_data(argv[i + 1]);
    }

    send_and_recv(bs_czl::BS_CMD::MSG_SET_BATCH_REQ, req, rsp);
    cout << rsp.DebugString() << endl;

    return 0;
}

int bs_gets(const vector<string> &argv) {
    bs_czl::MsgGetScoreBatchReq req;
    bs_czl::MsgGetScoreBatchRsp rsp;

    if (argv.size() <= 2) {
        cout << "参数错误！" << endl;
        return -1;
    }

    req.set_key_id(argv[1]);
    for (int i = 2; i < argv.size(); ++i) {
        req.add_mem_data_list(argv[i]);
    }

    send_and_recv(bs_czl::BS_CMD::MSG_GET_SCORE_BATCH_REQ, req, rsp);
    cout << rsp.DebugString() << endl;

    return 0;
}

int bs_ranks(const vector<string> &argv) {
    bs_czl::MsgRankBatchQueryReq req;
    bs_czl::MsgRankBatchQueryRsp rsp;

    if (argv.size() <= 2) {
        cout << "参数错误！" << endl;
        return -1;
    }

    req.set_key_id(argv[1]);
    for (int i = 2; i < argv.size(); ++i) {
        req.add_mem_info_list(argv[i]);
    }

    send_and_recv(bs_czl::BS_CMD::MSG_RANK_BATCH_QUERY_REQ, req, rsp);
    cout << rsp.DebugString() << endl;
}

int bs_range(const vector<string> &argv) {
    bs_czl::MsgRangeQueryReq req;
    bs_czl::MsgRangeQueryRsp rsp;

    if (argv.size() != 4) {
        cout << "参数错误！" << endl;
        return -1;
    }

    req.set_key_id(argv[1]);
    req.mutable_range()->set_min(stoull(argv[2]));
    req.mutable_range()->set_max(stoull(argv[3]));

    send_and_recv(bs_czl::BS_CMD::MSG_RANGE_QUERY_REQ, req, rsp);
    cout << rsp.DebugString() << endl;

    return 0;
}

int bs_rangebyscore(const vector<string> &argv) {
    bs_czl::MsgRangeByScoreQueryReq req;
    bs_czl::MsgRangeByScoreQueryRsp rsp;

    if (argv.size() != 4) {
        cout << "参数错误！" << endl;
        return -1;
    }

    req.set_key_id(argv[1]);
    req.mutable_range()->set_min(stoull(argv[2]));
    req.mutable_range()->set_max(stoull(argv[3]));

    send_and_recv(bs_czl::BS_CMD::MSG_RANGEBYSCORE_QUERY_REQ, req, rsp);
    cout << rsp.DebugString() << endl;

    return 0;
}

int bs_top(const vector<string> &argv) {
    bs_czl::MsgTopQueryReq req;
    bs_czl::MsgTopQueryRsp rsp;

    if (argv.size() != 3) {
        cout << "参数错误！" << endl;
        return -1;
    }

    req.set_key_id(argv[1]);
    req.set_top(stoull(argv[2]));

    send_and_recv(bs_czl::BS_CMD::MSG_TOP_QUERY_REQ, req, rsp);
    cout << rsp.DebugString() << endl;

    return 0;
}

int help() {
    cout << "   bs_sets <key_id> <score mem_data [score mem_data] ... >  :设置分数及数据." << endl;
    cout << "   bs_gets <key_id> <mem_data [mem_data] ... >              :通过成员数据查看分数." << endl;
    cout << "   bs_ranks <key_id> <mem_data [mem_data] ... >             :批量查询用户排行." << endl;
    cout << "   bs_range <key_id> <min max>                              :范围查询(位置)" << endl;
    cout << "   bs_rangebyscore <key_id> <min max>                       :范围查询(分数)" << endl;
    cout << "   更多请期待！！  --CaiZhiLi" << endl;
    return 0;
}

int test(const vector<string> &argv) {
    if (argv.size() < 4)
        return -1;
    vector<string> vec;
    vec.push_back("test");
    vec.push_back("key1");

    for (int i = 0; i < stoi(argv[1]); ++i) {
        vec.push_back(to_string(i+1));
        vec.push_back(to_string(i+1));

        //每秒请求
        if ((i + 1) % stoi(argv[2]) == 0) {
            sleep(1);
        }
        //多少个为一组
        if ((i + 1) % stoi(argv[3]) == 0) {
            bs_sets(vec);
            vec.clear();
            vec.push_back("test");
            vec.push_back("key1");
        }
    }
    //send_and_recv(bs_czl::BS_CMD::MSG_SET_BATCH_REQ, req, rsp);
    return 0;
}

int HandleReq(const vector<string> &argv) {
    if ("bs_sets" == argv[0]) {
        return bs_sets(argv);
    } else if ("bs_gets" == argv[0]) {
        return bs_gets(argv);
    } else if ("bs_ranks" == argv[0]) {
        return bs_ranks(argv);
    } else if ("bs_range" == argv[0]) {
        return bs_range(argv);
    } else if ("bs_rangebyscore" == argv[0]) {
        return bs_rangebyscore(argv);
    } else if ("bs_top" == argv[0]) {
        return bs_top(argv);
    } else if ("help" == argv[0]) {
        return help();
    } else if ("test" == argv[0]) {
        return test(argv);
    } else if ("quit" == argv[0]) {
        return -2;
    } else {
        cout << "命令不存在！查看帮助(help)" << endl;
    }
    return 0;
}

int main(int argc, char **argv) {

    if (argc < 3) {
        cout << "usage : client ip port" << endl;
        return 0;
    }

    if (InitClinetConn(argc, argv) < 0)
        return 0;

    cout << ">> ";
    string strInput;
    while(getline(cin, strInput)) {
        vector<string> vargs;
        CStrTool::SplitStringUsing(strInput, " ", &vargs);
        DelSpaceStr(vargs);

        try {
            if (HandleReq(vargs) == -2) {
                break;
            }
        } catch (...) {
            cout << "参数有误！" << endl;
        }
        cout << ">> ";
    }

    return 0;
}
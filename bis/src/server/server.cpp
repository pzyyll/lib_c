//
// @Created by czllo on 2017/4/26.
// @breif 代码重复率极高，以后再想办法优化优化
//

#include "server.h"

using namespace std;
using namespace tnt;

int Server::OnInit(const char *conf_file) {

    if (LPSVRCFG->Init(conf_file) < 0)
        return -1;

    if (LPMQMGR->Init() < 0)
        return -1;

    LPDB->CheckDbFileAndLoad();

    uiLastSaveTime = time(NULL);
    chilld_pid = -1;

    return 0;
}

int Server::OnProc() {
    int ret = 0;
    ret = ProcessMQRecv();
    return ret;
}

//@CZL 说一次循环处理不要太多信息，那就每次最多10个吧
int Server::ProcessMQRecv() {
    std::vector<string> msgs;
    if (LPMQMGR->RecvMsgs(msgs, 10) <= 0) {
        return -1;
    }

    LOG(INFO) << "hand msgs size " << msgs.size() << endl;

    for (int i = 0; i < msgs.size(); ++i) {
        ProcessMQMsg(msgs[i]);
    }

    return 0;
}

void Server::ProcessMQMsg(const std::string &data) {
    transApp.Clear();
    if (!transApp.ParseFromString(data)) {
        LOG(ERROR) << "Parse Data Fail. Data Len=" << data.size() << endl;
        return;
    }

    LOG(INFO) << "Cmd Data:" << transApp.ShortDebugString() << endl;
    switch (transApp.cmd_id()) {
        case bs_czl::BS_CMD::MSG_SET_BATCH_REQ:
            ProcessDbSet();
            break;
        case bs_czl::BS_CMD::MSG_GET_SCORE_BATCH_REQ:
            ProcessDbGetScore();
            break;
        case bs_czl::BS_CMD::MSG_RANK_BATCH_QUERY_REQ:
            ProcessDbRankQuery();
            break;
        case bs_czl::BS_CMD::MSG_RANGE_QUERY_REQ:
            ProcessDbRangeByRank();
            break;
        case bs_czl::BS_CMD::MSG_RANGEBYSCORE_QUERY_REQ:
            ProcessDbRangeByScore();
            break;
        case bs_czl::BS_CMD::MSG_TOP_QUERY_REQ:
            ProcessDbTopQuery();
            break;
        default:
            LOG(WARNING) << "Cmd Id(" << transApp.cmd_id() << ") is invaild." << endl;
            break;
    }
}

void Server::ProcessDbSet() {
    bs_czl::MsgSetBatchReq req;
    bs_czl::MsgSetBatchRsp rsp;

    if (Parse(req) < 0)
        return;
    LOG(INFO) << "Set data : " << req.ShortDebugString() << endl;

    for (int i = 0; i < req.data_list_size(); ++i) {
        if(LPDB->Insert(req.key_id(), req.data_list(i).score(), req.data_list(i).data())) {
            rsp.set_succ_num(rsp.succ_num() + 1);
        }
    }
    rsp.set_ret(0);
    //rsp.set_err("Succ.");

    LOG(INFO) << "rsp : " << rsp.ShortDebugString() << endl;
    Respone(rsp);
}

void Server::ProcessDbGetScore() {
    bs_czl::MsgGetScoreBatchReq req;
    bs_czl::MsgGetScoreBatchRsp rsp;

    if (Parse(req) < 0)
        return;
    LOG(INFO) << "Set data : " << req.ShortDebugString() << endl;

    for (int i = 0; i < req.mem_data_list_size(); ++i) {
        double score = 0;
        bs_czl::MsgSetMemData *item = rsp.add_mem_data_list();
        assert(item != NULL);
        if (!LPDB->GetMemScore(req.key_id(), req.mem_data_list(i).data(), score)) {
            item->set_data("No Find!");
            continue;
        }
        item->set_data(req.mem_data_list(i).data());
        item->set_score(score);
    }
    rsp.set_ret(0);
    rsp.set_err("Succ.");

    LOG(INFO) << "rsp : " << rsp.ShortDebugString() << endl;
    Respone(rsp);
}

void Server::ProcessDbRankQuery() {
    bs_czl::MsgRankBatchQueryReq req;
    bs_czl::MsgRankBatchQueryRsp rsp;

    if (Parse(req) < 0)
        return;
    LOG(INFO) << "Set data : " << req.ShortDebugString() << endl;

    for (int i = 0; i < req.mem_info_list_size(); ++i) {
        unsigned long long rank;
        double score;
        bs_czl::MsgRankInfo *item = rsp.add_rank_info();
        assert(item != NULL);
        if (!LPDB->GetMemRank(req.key_id(), req.mem_info_list(i), rank)) {
            item->mutable_data()->set_data("No Find!");
            continue;
        }
        LPDB->GetMemScore(req.key_id(), req.mem_info_list(i), score);
        item->set_rank(rank);
        item->mutable_data()->set_data(req.mem_info_list(i));
        item->mutable_data()->set_score(score);
    }

    rsp.set_ret(0);
    rsp.set_err("Succ.");

    LOG(INFO) << "rsp : " << rsp.ShortDebugString() << endl;
    Respone(rsp);
}

void Server::ProcessDbRangeByRank() {
    bs_czl::MsgRangeQueryReq req;
    bs_czl::MsgRangeQueryRsp rsp;

    if (Parse(req) < 0)
        return;
    LOG(INFO) << "Set data : " << req.ShortDebugString() << endl;

    do {
        db::node_itr_type itr = LPDB->GetMemsFirstByRank(req.key_id(), req.range().min(), req.range().max());
        if (itr == LPDB->end()) {
            rsp.set_ret(bs_czl::MSG_RET::FAIL);
            rsp.set_err(LPDB->GetErr());
            break;
        }
        unsigned long long cnt = 0, min = req.range().min(), max = req.range().max();
        if (min > max) {
            unsigned long long t = min;
            min = max;
            max = t;
        }
        unsigned long long num = max - min + 1;
        for (; itr != LPDB->end() && cnt < num; ++cnt, ++itr) {
            bs_czl::MsgRankInfo *item = rsp.add_rank_info();
            item->set_rank(min + cnt);
            item->mutable_data()->set_score(itr->score);
            item->mutable_data()->set_data(itr->data.val);

            LOG(INFO) << "Info " << item->ShortDebugString() << endl;
        }
        rsp.set_ret(0);
        rsp.set_err("Succ.");
    } while (false);

    rsp.set_ret(0);
    rsp.set_err("Succ.");
    LOG(INFO) << "rsp : " << rsp.ShortDebugString() << endl;
    Respone(rsp);
}

void Server::ProcessDbRangeByScore() {
    bs_czl::MsgRangeByScoreQueryReq req;
    bs_czl::MsgRangeByScoreQueryRsp rsp;

    if (Parse(req) < 0)
        return;
    LOG(INFO) << "Set data : " << req.ShortDebugString() << endl;

    do {
        db::node_pair_type pair = LPDB->GetMemsByScore(req.key_id(), req.range().min(), req.range().max());
        if (!LPDB->GetErr().empty()) {
            rsp.set_ret(bs_czl::MSG_RET::FAIL);
            rsp.set_err(LPDB->GetErr());
            break;
        }
        db::node_itr_type itr = pair.first;
        unsigned long long rank = 0;
        LPDB->GetMemRank(req.key_id(), itr->data.val, rank);
        for (; itr != pair.second; ++itr, ++rank) {
            auto item = rsp.add_rank_info();
            item->set_rank(rank);
            item->mutable_data()->set_score(itr->score);
            item->mutable_data()->set_data(itr->data.val);

            LOG(INFO) << "Info " << item->ShortDebugString() << endl;
        }
        auto item = rsp.add_rank_info();
        item->set_rank(rank);
        item->mutable_data()->set_score(pair.second->score);
        item->mutable_data()->set_data(pair.second->data.val);

        rsp.set_ret(0);
        rsp.set_err("Succ.");
    } while (false);

    LOG(INFO) << "rsp : " << rsp.ShortDebugString() << endl;
    Respone(rsp);
}

void Server::ProcessDbTopQuery() {
    bs_czl::MsgTopQueryReq req;
    bs_czl::MsgTopQueryRsp rsp;

    if (Parse(req) < 0)
        return;
    LOG(INFO) << "Set data : " << req.ShortDebugString() << endl;

    db::node_itr_type itr = LPDB->GetMemByRank(req.key_id(), req.top());
    if (itr == LPDB->end()) {
        rsp.set_ret(bs_czl::MSG_RET::FAIL);
        rsp.set_err(LPDB->GetErr());
    } else {
        rsp.set_ret(bs_czl::MSG_RET::SUCCESS);
        rsp.mutable_mem_data()->set_data(itr->data.val);
        rsp.mutable_mem_data()->set_score(itr->score);
    }

    LOG(INFO) << "rsp : " << rsp.ShortDebugString() << endl;
    Respone(rsp);
}

int Server::Parse(::google::protobuf::Message &msg) {
    if (!msg.ParseFromString(transApp.app_data())) {
        LOG(WARNING) << "Parse Req Data Fail.|" << transApp.app_data()
                     << "|" << transApp.ShortDebugString() << endl;
        return -1;
    }
    return 0;
}

int Server::Respone(const ::google::protobuf::Message &data) {
    bs_czl::MsgTransApp rsp;

    rsp.set_client_pos(transApp.client_pos());
    rsp.set_app_data(data.SerializeAsString());

    LOG(INFO) << "RSP:" << rsp.ShortDebugString() << endl;
    LPMQMGR->SendMsg(transApp.src(), rsp);
    return 0;
}

int Server::OnTick() {
    LOG(INFO) << "====Tick=====" << endl;
    WaitChilen();
    return ApplicationBase::OnTick();
}

int Server::OnExit() {
    LPDB->SaveToFile();
    LOG(INFO) << "======EXIT=========" << endl;
    return ApplicationBase::OnExit();
}

int Server::OnStop() {
    LOG(INFO) << "======Stop=========" << endl;
    return ApplicationBase::OnStop();
}

int Server::OnIdle() {
    time_t now = time(NULL);

    //空闲时保存
    if (static_cast<int>(now - uiLastSaveTime) > LPSVRCFG->persistend_time_) {
        LPDB->BgSaveDb();
    }

    return 0;
}

void Server::CloseAllListenSockets() {
    LPMQMGR->CloseAll();
}

int Server::WaitChilen() {
    //有BUG这里总是没有进来？？
    if (chilld_pid != -1) {
        int staloc;
        pid_t pid;
        if ((pid = wait3(&staloc, WNOHANG, NULL)) > 0) {
            int exitcode = WEXITSTATUS(staloc);
            int bysignal = 0;
            if (WIFSIGNALED(staloc)) bysignal = WTERMSIG(staloc);

            if (!bysignal && exitcode == 0) {
                LOG(INFO) << "Bg Save Succ." << endl;
                uiLastSaveTime = time(NULL);
            } else if (!bysignal && exitcode != 0) {
                LOG(WARNING) << "Bg Save Fail." << endl;
            } else {
                LOG(WARNING) << "Bg Save terminated by signal " << bysignal << endl;
                //rmfile
                string tmp = LPSVRCFG->db_file_ + ".tmp";
                unlink(tmp.c_str());
            }

            chilld_pid = -1;
            uiSaveTimeSpan = time(NULL) - uiForkStartTime;
            uiForkStartTime = -1;

            LOG(INFO) << "SaveTimeSpan : " << uiSaveTimeSpan << endl;
        }
    }

    return 0;
}




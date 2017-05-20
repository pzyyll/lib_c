//
// Created by czllo on 2017/5/4.
//

#include "req_factor.h"

::google::protobuf::Message *req_factor::Create(const std::string &str, const int cmd) {
    if (bs_czl::BS_CMD::MSG_GET_SCORE_BATCH_REQ == cmd) {
        ::google::protobuf::Message *msg = new bs_czl::MsgGetScoreBatchReq;
        if (!msg || !Parse(*msg, str)) {
            return NULL;
        }
        return msg;
    } else if (bs_czl::BS_CMD::MSG_SET_BATCH_REQ == cmd) {
        ::google::protobuf::Message *msg = new bs_czl::MsgSetBatchReq;
        if (!msg || !Parse(*msg, str)) {
            return NULL;
        }
        return msg;
    } else if (bs_czl::BS_CMD::MSG_TOP_QUERY_REQ == cmd) {
        ::google::protobuf::Message *msg = new bs_czl::MsgTopQueryReq;
        if (!msg || !Parse(*msg, str)) {
            return NULL;
        }
        return msg;
    } else if (bs_czl::BS_CMD::MSG_RANGE_QUERY_REQ == cmd) {
        ::google::protobuf::Message *msg = new bs_czl::MsgRangeQueryReq;
        if (!msg || !Parse(*msg, str)) {
            return NULL;
        }
        return msg;
    } else if (bs_czl::BS_CMD::MSG_RANGEBYSCORE_QUERY_REQ == cmd) {
        ::google::protobuf::Message *msg = new bs_czl::MsgRangeByScoreQueryReq;
        if (!msg || !Parse(*msg, str)) {
            return NULL;
        }
        return msg;
    }

    return NULL;
}

bool req_factor::Parse(::google::protobuf::Message &msg, const std::string &str) {
    if (msg.ParseFromString(str))
        return true;
    return false;
}

std::string req_factor::GetKeyId(const std::string &str, const int cmd) {
    if (bs_czl::BS_CMD::MSG_GET_SCORE_BATCH_REQ == cmd) {
        bs_czl::MsgGetScoreBatchReq msg;
        if (!Parse(msg, str)) {
            return std::string();
        }
        return msg.key_id();
    } else if (bs_czl::BS_CMD::MSG_SET_BATCH_REQ == cmd) {
        bs_czl::MsgSetBatchReq msg;
        if (!Parse(msg, str)) {
            return std::string();
        }
        return msg.key_id();
    } else if (bs_czl::BS_CMD::MSG_TOP_QUERY_REQ == cmd) {
        bs_czl::MsgTopQueryReq msg;
        if (!Parse(msg, str)) {
            return std::string();
        }
        return msg.key_id();
    } else if (bs_czl::BS_CMD::MSG_RANGE_QUERY_REQ == cmd) {
        bs_czl::MsgRangeQueryReq msg;
        if (!Parse(msg, str)) {
            return std::string();
        }
        return msg.key_id();
    } else if (bs_czl::BS_CMD::MSG_RANGEBYSCORE_QUERY_REQ == cmd) {
        bs_czl::MsgRangeByScoreQueryReq msg;
        if (!Parse(msg, str)) {
            return std::string();
        }
        return msg.key_id();
    } else if (bs_czl::BS_CMD::MSG_RANK_BATCH_QUERY_REQ == cmd) {
        bs_czl::MsgRankBatchQueryReq msg;
        if (!Parse(msg, str)) {
            return std::string();
        }
        return msg.key_id();
    }

    return std::string();
}

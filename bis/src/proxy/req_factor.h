//
// Created by czllo on 2017/5/4.
//

#ifndef PROXY_REQ_FACTOR_H
#define PROXY_REQ_FACTOR_H

#include <string>
#include "proto/bs_msg.pb.h"
#include "proto/bs_cmd.pb.h"

class req_factor {
public:
    static ::google::protobuf::Message *Create(const std::string &str, const int cmd);

    static bool Parse(::google::protobuf::Message &msg, const std::string &str);

    static std::string GetKeyId(const std::string &str, const int cmd);
};


#endif //PROXY_REQ_FACTOR_H

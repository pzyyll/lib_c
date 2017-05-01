//
// @Created by czllo on 2017/4/30.
// @breif 命令对象，用于解析客户端请求
//

#ifndef SERVER_CMD_H
#define SERVER_CMD_H

#include <string>
#include <cstring>
#include <sys/types.h>
#include <netinet/in.h>
#include "server_config.h"
#include "proto/msg_head.h"
#include <comm/util/pet_util.h>


class Cmd {
public:
    Cmd();
    ~Cmd();

    int Parse(const char *cszDataBuf, size_t uiDataLen);
    bool Serialize(std::string &strData);
    bool Serialize(BufferSequenceType &bs);
    bool Serialize(char *cszDataBuf, size_t &uiDataLen);
    void Clear();

public:
    unsigned get_cmd_id() const;
    unsigned get_check() const;
    unsigned get_msg_idx() const;
    std::string &get_app_data() const;

    void set_cmd_id(unsigned short uCmdId);
    void set_check(unsigned short uCheck);
    void set_msg_idx(unsigned uMsgIdx);
    void set_app_data(const char *cszDataBuff, size_t uiDataLen);
    void set_app_data(const std::string &strData);

private:
    unsigned short uCmdId_;
    unsigned short uCheck_;
    unsigned uMsgIdx_;
    std::string strAppData_;
};


#endif //SERVER_CMD_H

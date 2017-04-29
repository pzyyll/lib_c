//
// @Created by czl.
//

#ifndef SERVER_MSG_HEAD_H
#define SERVER_MSG_HEAD_H

#include <google/protobuf/message.h>

struct MsgHead {
    int msg_len;
    int cmd_id;
};

struct MsgApp {
    MsgHead head;
    google::protobuf::Message *pData;
};

#endif //SERVER_MSG_HEAD_H

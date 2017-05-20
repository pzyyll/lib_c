//
// @Created by czl.
//

#ifndef SERVER_MSG_HEAD_H
#define SERVER_MSG_HEAD_H

#include <google/protobuf/message.h>

#pragma pack(1)
struct MsgHead {
    unsigned int msg_len;
    unsigned short cmd_id;
    unsigned short check;
};
#pragma pack()

struct MsgApp {
    MsgHead head;
    google::protobuf::Message *pData;
};

#endif //SERVER_MSG_HEAD_H

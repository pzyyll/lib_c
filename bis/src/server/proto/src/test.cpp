//
// Created by czllo on 2017/4/29.
//
#include <string>
#include "bs_msg.pb.h"
#include "../msg_head.h"

using namespace std;
//using namespace bs_czl;

int main() {
    bs_czl::MsgHead appHead;
    bs_czl::MsgSetBatchReq set_req;
    set_req.set_key_id("czl");
    bs_czl::MsgSetMemData *info = set_req.add_data_list();
    info->set_score(12);
    info->set_data("12");
    info = set_req.add_data_list();
    info->set_score(14);
    info->set_data("14");

    string serialize = set_req.SerializeAsString();
    cout << "set size: " << serialize.size() << endl;
    appHead.set_msg_len(serialize.size());
    appHead.set_cmd_id(4096);

    cout << "serialize head size : " << appHead.SerializeAsString().size() << endl;
    cout << "head size (sizeof):" << sizeof(MsgHead) << endl;

    cout << set_req.DebugString() << endl;
    cout << set_req.ShortDebugString() << endl;

    MsgApp app;
    app.head.cmd_id = appHead.cmd_id();
    app.head.msg_len = appHead.msg_len();
    app.pData = &set_req;

    cout << app.pData->DebugString() << endl;
    return 0;
}
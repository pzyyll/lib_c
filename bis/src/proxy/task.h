//
// @Created by czllo.
//

#ifndef SERVER_TASK_H
#define SERVER_TASK_H

#include <string>
#include <memory>
#include <queue>

#include <ctime>
#include <cstdio>

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

#include "comm/buffer/recv_buffer.h"
#include "server_config.h"
#include "log_def.h"
#include "cepoll.h"
#include "cmd.h"
#include "proto/bs_msg.pb.h"
#include "req_factor.h"
#include "mq_mgr.h"

class Task {
    typedef std::deque<BufferSequenceType> SendQueue;
public:
    typedef bool (*sendcb)();
    //typedef std::shared_ptr<Cmd> CmdPtr;
public:
    Task(unsigned int uPos, int iSock, struct sockaddr_in &objAddr);
    virtual ~Task();

    //@kick client
    void Terminate(const char *cstrFormat, ...) __attribute__((format(printf, 2, 3)));
    bool IsTerminate();

    //@EPOLLIN CALL BACK
    virtual void OnSockReadHdl();
    virtual void OnSockWriteHdl();

    void OnSockRecv();
    void WriteRemain();

    void OnMQRecv(bs_czl::MsgTransApp &msg);

public:
    unsigned int get_pos() const;
    int get_sock() const;
    std::string get_ip() const;

    unsigned int get_last_act_time() const;
    void set_last_act_time(const unsigned int uiTime);

protected:
    void Recv();
    void Response(BufferSequenceType& bs);
    int FlushSendQueue();

    void HandleCmdMsg();
    virtual void DoCmd();

protected:
    unsigned int uPos_;
    int iSock_;
    struct sockaddr_in objAddr_;
    char cstrIP[16];
    unsigned int uLastActiveTime_;

    snslib::RecvBuffer objRecvBuff_;
    SendQueue objSendQueue_;

    Cmd objCmd_;
};

typedef std::shared_ptr<Task> TaskPtr;

#endif //SERVER_TASK_H

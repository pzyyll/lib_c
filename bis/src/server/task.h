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

class Task {
    typedef std::deque<BufferSequenceType> SendQueue;
    //typedef std::shared_ptr<Cmd> CmdPtr;
public:
    Task(unsigned int uPos, int iSock, struct sockaddr_in &objAddr);
    ~Task();

    //@kick client
    void Terminate(const char *cstrFormat, ...) __attribute__((format(printf, 1, 2)));
    bool IsTerminate();

    //@EPOLLIN CALL BACK
    void OnSockReadHdl();
    void OnSockWriteHdl();

    void OnSockRecv();
    void WriteRemain();


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

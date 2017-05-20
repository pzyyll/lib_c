// Copyright: Tencent Tech. Co., Ltd.
// Author: chadshao
// Date: 2014/03/28
#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class TcpClient
{
public:
    TcpClient();
    ~TcpClient();
    
    int init(const char* ip, unsigned short port, int rw_timeout = kRwTimeout, int connect_timeout = kConnectTimeout);
    void set_connect_timeout(int timeout) { connect_timeout_ = timeout; }
    void set_rw_timeout(int timeout) { rw_timeout_ = timeout; }

    int send(void* buffer, unsigned int length);
    int recv(void* buffer, unsigned int& length, unsigned int expect_len = 0);
    int send_and_recv(void* request, unsigned int request_len, void* response, unsigned int& response_len);

    int reconnect();

    int check_connect()       { return check_connect_; }
    void close()              { close_socket(); }
    const char* get_err_msg() { return err_msg_; }

private:
    int init_socket();
    void close_socket();
    int connect();
    int poll_wait(short events, int timeout);
    int conncet_wait(int timeout);
    int recv_wait(int timeout);
    int send_wait(int timeout);

public:
    char               ip_[16];
    unsigned short     port_;

private:
    char               err_msg_[1024];
    int                connect_timeout_;
    int                rw_timeout_;
    int                socket_;
    struct sockaddr_in sock_addr_;
    bool               socket_inited_;
    int                check_connect_;

    const static int   kConnectTimeout = 300;
    const static int   kRwTimeout = 500;
};

#endif


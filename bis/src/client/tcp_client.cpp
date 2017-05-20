// Copyright: Tencent Tech. Co., Ltd.
// Author: chadshao
// Date: 2014/03/28
#include "tcp_client.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <fcntl.h>

TcpClient::TcpClient()
    : port_(0), 
      connect_timeout_(kConnectTimeout),
      rw_timeout_(kRwTimeout),
      socket_(0), 
      socket_inited_(false)
{
    memset(err_msg_, 0, sizeof(err_msg_));
    memset(ip_, 0, sizeof(ip_));
    memset(&sock_addr_, 0, sizeof(sock_addr_));
    check_connect_ = -1;
}

TcpClient::~TcpClient()
{
    close_socket();
}

int TcpClient::init(const char* ip, unsigned short port, int rw_timeout, int connect_timeout)
{
    int ret = 0;

    snprintf(ip_, sizeof(ip_), "%s", ip);
    port_ = port;
    rw_timeout_ = rw_timeout;
    connect_timeout_ = connect_timeout;

    ret = connect();
    if (ret == 0) {
        check_connect_ = 0;
    }
    
    return ret;
}

int TcpClient::init_socket()
{
    if (!socket_inited_) {
        socket_ = socket(PF_INET, SOCK_STREAM, 0);
        if (-1 == socket_) {
            snprintf(err_msg_, sizeof(err_msg_), "%s", strerror(errno));
            return -1;
        }
        socket_inited_ = true;
    }

    return 0;
}

void TcpClient::close_socket()
{
    if (socket_inited_) {
        ::close(socket_);
        socket_inited_ = false;
    }
}

int TcpClient::connect()
{
    if (0 != init_socket())
        return -1;

    sock_addr_.sin_family = AF_INET;
    sock_addr_.sin_port = htons(port_);
    sock_addr_.sin_addr.s_addr = inet_addr(ip_);

    int arg;
    if ((arg = fcntl(socket_, F_GETFL, 0)) < 0) {
        snprintf(err_msg_, sizeof(err_msg_), "%s", "fcntl F_GETFL failed");
        return -1;
    }

    if (fcntl(socket_, F_SETFL, arg | O_NONBLOCK) < 0) {
        snprintf(err_msg_, sizeof(err_msg_), "%s", "fcntl F_SETFL noblock failed");
        return -1;
    }

    if (::connect(socket_, (struct sockaddr *)&sock_addr_, sizeof(sock_addr_)) < 0) {
        if (errno == EINPROGRESS) {
            if (0 != conncet_wait(connect_timeout_))
                return -1;

            int optval;       
            socklen_t len = sizeof(optval);
            if (getsockopt(socket_, SOL_SOCKET, SO_ERROR, (void*)(&optval), &len) < 0) {
               snprintf(err_msg_, sizeof(err_msg_), "getsockopt failed, %s", strerror(errno));
               return -1;
            }

            if (optval != 0) {
                snprintf(err_msg_, sizeof(err_msg_), "getsockopt SOL_SOCKET SO_ERROR, optval:%d\n", optval);
                return -1;
            }

        } else {
            snprintf(err_msg_, sizeof(err_msg_), "%s", strerror(errno));
            return -1;
        }
    }

//    if (fcntl(socket_, F_SETFL, arg) < 0) {
//        snprintf(err_msg_, sizeof(err_msg_), "%s", "fcntl F_SETFL failed");
//        return -1;
//   }

    return 0;
}

int TcpClient::reconnect()
{
    close_socket();
    return connect();
}

int TcpClient::conncet_wait(int timeout)
{
    return poll_wait(POLLOUT, timeout);
}

int TcpClient::recv_wait(int timeout)
{
    return poll_wait(POLLIN, timeout);
}

int TcpClient::send_wait(int timeout)
{
    return poll_wait(POLLOUT, timeout);
}

int TcpClient::poll_wait(short events, int timeout)
{
    struct pollfd poll_fd;

    poll_fd.fd = socket_;
    poll_fd.events = events;
    poll_fd.revents = 0;

    while (true) {
        errno = 0;
        
        switch (::poll(&poll_fd, 1, timeout)) {
        case -1:
            if (errno != EINTR) {
                snprintf(err_msg_, sizeof(err_msg_), "%s", strerror(errno));
                return -1;
            }
            continue;
        case 0:
            errno = ETIME;
            snprintf(err_msg_, sizeof(err_msg_), "poll timeout");
            return -1;

        default:
            if (poll_fd.revents & POLLHUP) {
                snprintf(err_msg_, sizeof(err_msg_), "poll returned POLLHUP.");
                return -1;
            }
            if (poll_fd.revents & POLLERR) {
                snprintf(err_msg_, sizeof(err_msg_), "poll returned POLLERR.");
                return -1;
            }
            if (poll_fd.revents & POLLNVAL) {
                snprintf(err_msg_, sizeof(err_msg_), "poll returned POLLNVAL.");
                return -1;
            }
            if (poll_fd.revents & events) {
                return 0;
            } else {
                snprintf(err_msg_, sizeof(err_msg_), "poll returned event error.");
                return -1;
            }
        }
    }
}

int TcpClient::send(void* buffer, unsigned int length)
{
    if (NULL == buffer) {
        snprintf(err_msg_, sizeof(err_msg_), "send error: send buffer is NULL");
        return -1;
    }

    unsigned int bytes_sent = 0;
    int write_bytes = 0;

    while (true) {
        if (0 != send_wait(rw_timeout_)) {
            check_connect_ = -1;
            return -1;
        }

        write_bytes = write(socket_, (unsigned char *)buffer + bytes_sent, length - bytes_sent);
        if (write_bytes > 0) {
            bytes_sent += write_bytes;
            if (bytes_sent < length) 
                continue;
            else 
                break;
        } else if (errno == EINTR) { 
            continue;
        } else {
            break;
        }
    }

    if (write_bytes < 0) {
        snprintf(err_msg_, sizeof(err_msg_), "send error: %s", strerror(errno));
        check_connect_ = -1;
        return -1;
    }

    return 0;
}

int TcpClient::recv(void* buffer, unsigned int& length, unsigned int expect_len)
{
    if (NULL == buffer) {
        snprintf(err_msg_, sizeof(err_msg_), "send error: recv buffer is NULL");
        return -1;
    }

    unsigned int bytes_received = 0;
    int read_bytes = 0;

    while (true) {
        if (0 != recv_wait(rw_timeout_)) {
            check_connect_ = -1;
            return -1;
        }

        if (expect_len > 0) {
            read_bytes = read(socket_, (unsigned char *)buffer + bytes_received, expect_len - bytes_received);
            if (read_bytes > 0) {
                length = (bytes_received += read_bytes);
                if (bytes_received < expect_len) 
                    continue;
                else 
                    break;
            } else if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        } else {
            read_bytes = read(socket_, buffer, length);
            if (read_bytes > 0) {
                length = read_bytes;
                return 0;
            } else if (errno == EINTR) {
                continue; 
            } else {
                break;
            }
        }
    }

    if (read_bytes <= 0) {
        check_connect_ = -1;
        snprintf(err_msg_, sizeof(err_msg_), "recv error: %s", strerror(errno));
        return -1;
    }

    return 0;
}

int TcpClient::send_and_recv(void* request, unsigned int request_len, void* response, unsigned int& response_len)
{
    if (0 != send(request, request_len)) {
        check_connect_ = -1;
        return -1;
    }

    if (0 != recv(response, response_len)) {
        check_connect_ = -1;
        return -2;
    }

    return 0;
}


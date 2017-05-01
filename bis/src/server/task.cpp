//
// Created by czllo on 2017/4/28.
//

#include "task.h"

Task::Task(unsigned int uPos, int iSock, struct sockaddr_in &objAddr) :
        uPos_(uPos),
        iSock_(iSock),
        objAddr_(objAddr),
        uLastActiveTime_(0),
        objRecvBuff_(MAX_CMD_LEN) {

        //snprintf(cstrIP, sizeof(cstrIP), "unkown ip");
        inet_ntop(AF_INET, &objAddr.sin_addr, cstrIP, sizeof(cstrIP));
}

Task::~Task() {
    if (!IsTerminate()) {
        ::close(iSock_);
        iSock_ = -1;
    }
}

unsigned int Task::get_pos() const {
    return uPos_;
}

int Task::get_sock() const {
    return iSock_;
}

std::string Task::get_ip() const {
    return std::string(cstrIP);
}

void Task::Terminate(const char *cstrFormat, ...) {
    if (IsTerminate()) {
        return;
    }

    close(iSock_);
    iSock_ = -1;

    static char buffer[1024];
    bzero(buffer, sizeof(buffer));

    va_list args;
    va_start(args, reason);
    vsnprintf(buffer, sizeof(buffer) - 1, cstrFormat, args);
    va_end(args);

    LOG_INFO("%s", buffer);
    //todo remove this task
}

bool Task::IsTerminate() {
    return iSock_ <= 0;
}

void Task::OnSockReadHdl() {
    OnSockRecv();
    HandleCmdMsg();
}

void Task::OnSockWriteHdl() {
    WriteRemain();
}

void Task::OnSockRecv() {
    Recv();
    set_last_act_time(time(NULL));
}

void Task::WriteRemain() {
    int iErr = FlushSendQueue();
    if (iErr < 0) {
        Terminate("flush send queue fail, ret=%d, closing conn", iErr);
        return;
    }

    if (objSendQueue_.empty()) {
        LPEPOLLHLD->EpollMod(iSock_, uPos_, EPOLLIN);
    }
}

void Task::Recv() {
    if (IsTerminate()) {
        return;
    }
    LOG_INFO("buf empty size|%u|full size|%u",
             static_cast<unsigned int>(objRecvBuff_.Remain()),
             static_cast<unsigned int>(objRecvBuff_.m_size));

    int rvsize = 0;
    while  (objRecvBuff_.Remain() > 0) {
        rvsize = ::recv(iSock_, objRecvBuff_.m_nptr, objRecvBuff_.Remain(), MSG_NOSIGNAL);
        if (rvsize < 0) {
            //In fact, EAGAIN = EWOULDBLOCK
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                Terminate("recv failed|%u|%s", errno, strerror(errno));
            }
            break;
        } else if (0 == rvsize) {
            Terminate("conn close by clien.");
            return;
        }

        objRecvBuff_.Advance(rvsize);
    }

}

void Task::Response(BufferSequenceType &bs) {
    //bool bIsSendQueueEmpty = objSendQueue_.empty();
    objSendQueue_.push_back(bs);

    int iErr = FlushSendQueue();
    if (iErr < 0) {
        Terminate("flush send queue fail, ret=%d, closing conn", iErr);
        return;
    }

    set_last_act_time(time(NULL));

    if (!objSendQueue_.empty()) {
        //@czl
        //网上说不能同时注册EPOLLIN和EPOLLOUT事件，因为每次EPOLLIN都会产生EPOLLOUT
        //但是我又想在下个循环中如果有EPOLLIN事件时同时处理EPOLLOUT事件。矛盾啊
        //LPEPOLLHLD->EpollMod(iSock_, uPos_, EPOLLOUT);
        LPEPOLLHLD->EpollMod(iSock_, uPos_, EPOLLIN | EPOLLOUT);

        LOG_WARN("solidify response buffer|remain=%u", objSendQueue_.back().Remain());
        objSendQueue_.back().Solidify();
    }
}

int Task::FlushSendQueue() {
    int ret = SUCCSESS;
    if (IsTerminate()) {
        ret = FAIL;
        return ret;
    }

    int writen, n = 0;
    struct iovec vec[BUFFER_SEQUCNE_COUNT];
    int iovec_num = 0;

    while (!objSendQueue_.empty()) {
        BufferSequenceType &bs = objSendQueue_.front();
        if (bs.Empty()) {
            objSendQueue_.pop_front();
            continue;
        }

        iovec_num = bs.InitIoVec(vec);
        LOG_INFO("iovec_num|%d", iovec_num);

        n = ::writev(iSock_, vec, iovec_num);
        if (n < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                LOG_WARN("writev fail, err=%d, msg=%s", errno, stderr(errno));
                return -1;
            }

            LOG_WARN("send is block in send queue.remain=%u", static_cast<unsigned>(bs.Remain()));
            return 0;
        } else if (0 == n) {
            LOG_WARN("conn close by client.");
            return -2;
        }

        LOG_INFO("Flus data len %d", n);
        writen += n;
        bs.Advance(n);
        if (bs.Empty()) {
            objSendQueue_.pop_front();
        }
    }
    return 0;
}

unsigned int Task::get_last_act_time() const {
    return uLastActiveTime_;
}

void Task::set_last_act_time(const unsigned int uiTime) {
    uLastActiveTime_ = uiTime;
}

void Task::HandleCmdMsg() {
    while(objRecvBuff_.UsedSize() > 0) {
        objCmd_.Clear();
        int rv = objCmd_.Parse(objRecvBuff_.m_base_p, objRecvBuff_.UsedSize());
        if (rv < 0) {
            //@czl 包有问题
            Terminate("objCmd parse fail.");
            return;
        } else if (0 == rv) {
            //@czl
            return;
        }

        //TODO 执行实际的数据处理
        //test
        LOG_INFO("handle cmd|%u|%s|", objCmd_.get_cmd_id(), objCmd_.get_app_data().c_str());
        objCmd_.set_cmd_id(999);
        BufferSequenceType bs;
        if (!objCmd_.Serialize(bs)) {
            return;
        }
        //TODO 添加请求频率限制

        Response(bs);
    }
}



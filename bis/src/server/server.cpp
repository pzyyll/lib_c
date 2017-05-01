//
// Created by czllo on 2017/4/26.
//

#include "server.h"

using namespace std;

Server::Server() : bLoop_(0), iSock_(-1), iPosCnt(0) {

}

Server::~Server() {

}

int Server::Run() {
    int ret = SUCCSESS;

    if ((ret = StartListen()) < 0) {
        LOG_ERR("start listen err.");
        return ret;
    }

    struct epoll_event evs[EPOLL_MAX_EVS];

    bLoop_ = 1;
    while(bLoop_) {
        //main loop
        int bWorking = false;
        errno = 0;
        int nds = LPEPOLLHLD->EpollWait(evs, EPOLL_MAX_EVS);
        if (nds > 0) {
            bWorking = true;
            LOG_INFO("epoll wait. recv(%d)", nds);
            HldAllEvs(evs, nds);

        } else {
            if (nds < 0 && EINTR != errno) {
                LOG_ERR("epoll_wait fail|%s", strerror(errno));
            }
            //LOG_INFO("sig intr or nothin can be do");
        }

        //TODO 处理其它事务
        if (!bWorking) {
            usleep(static_cast<__useconds_t>(LPSVRCFG->free_time_sleep_));
        }
    }

    return ret;
}

int Server::Init() {
    int ret = SUCCSESS;

    ret = LPEPOLLHLD->EpollInit();

    return ret;
}

int Server::StartListen() {
    int ret = SUCCSESS;

    //0.Init
    if ((ret = Init() ) < 0) {
        LOG_ERR("Init Server fail.");
        return ret;
    }

    //1.create listen socket fd;
    iSock_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (iSock_ < 0) {
        LOG_ERR("create socket fd fail.");
        ret = FAIL;
        return ret;
    }

    int nodelay = 1;
    if ((ret = ::setsockopt(iSock_, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) ) < 0) {
        LOG_ERR("setsockopt nodelay fail.");
        return ret;
    }

    int reuse = 1;
    if ((ret = ::setsockopt(iSock_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) ) < 0) {
        LOG_ERR("setsockopt addr reuse fail.");
        return ret;
    }

    if ((ret = MakeNonBlocking(iSock_) ) < 0) {
        return ret;
    }

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SingletonServerCfg::instance()->port_);
    if ("" == SingletonServerCfg::instance()->ip_) {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        ret = inet_pton(AF_INET, SingletonServerCfg::instance()->ip_.c_str(), &addr.sin_addr);
    }

    if (ret <  0) {
        LOG_ERR("IP INVAILD.");
        return ret;
    }

    //2.bind listen fd
    if ((ret = ::bind(iSock_, (struct sockaddr *)&addr, sizeof(addr)) ) < 0) {
        LOG_WARN("bind fail for fd(%d).", iSock_);
        return ret;
    }

    //3.listen fd.
    if ((ret = ::listen(iSock_, 4096)) < 0) {
        LOG_WARN("listen fail|%d|", iSock_);
        return ret;
    }

    if (0 != LPEPOLLHLD->EpollAdd(iSock_, LISTEN_TASK_POS, EPOLLIN)) {
        LOG_WARN("add to epoll fail for listen fd.");
        ret = FAIL;
        return ret;
    }

    LOG_INFO("=====Start Listen=====");
    return ret;
}

int Server::MakeNonBlocking(int fd) {
    int val = ::fcntl(fd, F_GETFL, 0);
    if (val < 0)
    {
        LOG_ERR("get stat for fd(%d) fail.", fd);
        return -1;
    }

    val |= O_NONBLOCK;
    if (::fcntl(fd, F_SETFL, val) < 0)
    {
        LOG_ERR("set blocking for fd(%d) fail.", fd);
        return -1;
    }

    return 0;
}

void Server::HldAllEvs(struct epoll_event *evs, int nds) {
    if (NULL == evs) {
        LOG_WARN("What hapen this null ptr?");
        return;
    }

    for(int i = 0; i < nds; ++i) {
        unsigned int cpos = evs[i].data.u32;
        if (LISTEN_TASK_POS == cpos) {
            if (evs[i].events & EPOLLIN) {
                AcceptTask();
            } else {
                LOG_WARN("happen event(%u) is not intre", evs[i].events);
            }
        } else {
            TaskPtr pTask = FindTask(cpos);
            if (!pTask) {
                LOG_WARN("not find task, pos=%u", cpos);
                continue;
            }

            if (evs[i].events & EPOLLOUT) {
                pTask->OnSockWriteHdl();
                if (pTask->IsTerminate()) {
                    RemoveTask(cpos);
                }
            }
            if (evs[i].events & EPOLLIN) {
                pTask->OnSockReadHdl();
                if (pTask->IsTerminate()) {
                    RemoveTask(cpos);
                }
            }

            if (evs[i].events & EPOLLERR)
            {
                pTask->Terminate("EPOLLERR");
                RemoveTask(cpos);
                continue;
            } else if (evs[i].events & EPOLLHUP)
            {
                pTask->Terminate("EPOLLHUP");
                RemoveTask(cpos);
                continue;
            }
        }
    }
}

int Server::AcceptTask() {

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t iLen = sizeof(addr);

    while (true) {
        int cli_fd = ::accept(iSock_, (struct sockaddr *)&addr, &iLen);
        if (cli_fd < 0) {
            if (EAGAIN != errno)
                LOG_WARN("accept fail.errmsg=%s", strerror(errno));
            break;
        }

        unsigned int cpos = (++iPosCnt == LISTEN_TASK_POS) ? ++iPosCnt : iPosCnt;
        LOG_INFO("new pos for accept cli|%u", cpos);

        if (MakeNonBlocking(cli_fd) < 0)
            return FAIL;

        int iNoDelay = 1;
        if (::setsockopt(cli_fd, IPPROTO_TCP, TCP_NODELAY, &iNoDelay, sizeof(iNoDelay)) < 0) {
            LOG_WARN("setsockopt fail.errmsg=%s", strerror(errno));
            return FAIL;
        }

        int iSendBuffSize = 4 * 32768;
        int iRecvBuffSize = 4 * 32768;

        if (::setsockopt(cli_fd, SOL_SOCKET, SO_SNDBUF, &iSendBuffSize, sizeof(iSendBuffSize)) < 0) {
            LOG_WARN("setsockopt fail.errmsg=%s", strerror(errno));
            return FAIL;
        }

        if (::setsockopt(cli_fd, SOL_SOCKET, SO_RCVBUF, &iRecvBuffSize, sizeof(iRecvBuffSize)) < 0) {
            LOG_WARN("setsockopt fail.errmsg=%s", strerror(errno));
            return FAIL;
        }

        TaskPtr task(new Task(cpos, cli_fd, addr));
        if (LPEPOLLHLD->EpollAdd(cli_fd, cpos, EPOLLIN) < 0) {
            task->Terminate("epoll add fail|errmsg=%s", strerror(errno));
            return FAIL;
        }

        objTaskMap.insert(make_pair(cpos, task));

        //TODO ADD Timer

        LOG_INFO("conn succ.cli ip=%s|pos=%u", task->get_ip().c_str(), task->get_pos());
    }

    return 0;
}

TaskPtr Server::FindTask(unsigned int pos) {
    TaskMapItr itr = objTaskMap.find(pos);
    if (objTaskMap.end() == itr)
        return TaskPtr();
    return itr->second;
}

void Server::RemoveTask(unsigned int pos) {
    LOG_INFO("close task, pos=%u", pos);

    TaskMapItr itr = objTaskMap.find(pos);
    if (itr != objTaskMap.end()) {
        RemoveTask(itr);
    }
}

void Server::RemoveTask(TaskMapItr &itr) {
    if (itr->second && !itr->second->IsTerminate()) {
        itr->second->Terminate("remove task");
    }
    objTaskMap.erase(itr);
}

void Server::AddTime(struct timeval &tvTime, int milliseconds) {
    tvTime.tv_sec += milliseconds / 1000;
    tvTime.tv_usec += milliseconds % 1000 * 1000;
    if (tvTime.tv_usec >= 1000000)
    {
        tvTime.tv_sec++;
        tvTime.tv_usec -= 1000000;
    }
}

void Server::AddTimeout(ExpireTimer &timer) {
    objTimerHeap.push(timer);
}

int Server::RemoveTimer(unsigned long long ullTimerID) {
    return 0;
}

void Server::ExpireTimers(struct timeval *next_timeout) {

}

void Server::StopService() {

}





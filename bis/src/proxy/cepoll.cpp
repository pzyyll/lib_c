//
// Created by czllo on 2017/4/28.
//

#include "cepoll.h"

int CEpoll::get_iEpfd() const {
    return iEpfd;
}

void CEpoll::set_iEpfd(int iEpfd) {
    CEpoll::iEpfd = iEpfd;
}

CEpoll::CEpoll() : iEpfd(-1), uTimeout(0) {

}

CEpoll::~CEpoll() {
    if (iEpfd > 0)
        close(iEpfd);
    iEpfd = -1;
}

int CEpoll::EpollInit(unsigned uTimeout) {
    this->uTimeout = uTimeout;
    iEpfd = epoll_create1(EPOLL_CLOEXEC);
    return iEpfd;
}

int CEpoll::EpollAdd(int fd, unsigned data, int epoll_mask) {
    struct epoll_event ev;
    ev.data.u32 = data;
    ev.events |= epoll_mask | EPOLLET | EPOLLRDHUP;
    return epoll_ctl(iEpfd, EPOLL_CTL_ADD, fd, &ev);
}

int CEpoll::EpollDel(int fd) {
    return epoll_ctl(iEpfd, EPOLL_CTL_DEL, fd, NULL);
}

int CEpoll::EpollMod(int fd, unsigned data, int epoll_mask) {
    struct epoll_event ev;
    ev.data.u32 = data;
    ev.events |= epoll_mask | EPOLLET;
    return epoll_ctl(iEpfd, EPOLL_CTL_MOD, fd, &ev);
}

int CEpoll::EpollWait(struct epoll_event *pEvs, unsigned int size) {
    int nds = epoll_wait(iEpfd, pEvs, size, uTimeout);
    return nds;
}

void CEpoll::set_uTimeout(unsigned uTimeout) {
    this->uTimeout = uTimeout;
}

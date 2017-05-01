//
// Created by czllo on 2017/4/28.
//

#ifndef SERVER_CEPOLL_H
#define SERVER_CEPOLL_H

#include <sys/epoll.h>
#include <unistd.h>
#include <singleton.h>

const unsigned int EPOLL_MAX_EVS = 2048;

class CEpoll {
public:
    CEpoll();
    ~CEpoll();

    int EpollInit(unsigned uTimeout = 0);
    int EpollAdd(int fd, unsigned data, int epoll_mask);
    int EpollDel(int fd);
    int EpollMod(int fd, unsigned data, int epoll_mask);
    int EpollWait(struct epoll_event *pEvs, unsigned int size);

    //int EpollAddInterest(struct epoll_event &pEv, int epoll_mask);
    //int EpollDelIntrest(struct epoll_event &pEv, int epoll_mask);

public:
    void set_iEpfd(int iEpfd);
    int get_iEpfd() const;
    void set_uTimeout(unsigned uTimeout);
private:
    int iEpfd;
    unsigned uTimeout;
    //struct epoll_event events[MAX_EPOLL_EVS];
};

typedef Singleton<CEpoll> SingletonEpoll;
#define LPEPOLLHLD SingletonEpoll::instance()

#endif //SERVER_CEPOLL_H

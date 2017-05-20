//
// Created by czllo on 2017/4/01.
//
#include <iostream>
#include <string>
#include <signal.h>

#include "server_config.h"
#include "server.h"

using namespace std;

static void TermSigHandler(int sig) {
    //LPSGLSVR->StopService();
}

int main (int argc, char **argv) {
    if (argc < 2) {
        cout << "use: server ${path}/xxx.ini" << endl;
        cout << "from chile." << endl;
        return -1;
    }

//    if (SingletonServerCfg::instance()->Init(argv[1]) != 0) {
//        return -1;
//    }

//    if (SingletonServerCfg::instance()->daemon_) {
//        daemon(1, 0);
//    }

    //hook sig
    struct sigaction sigact;
    sigact.sa_handler = TermSigHandler;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);
    sigaction(SIGABRT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGUSR1, &sigact, NULL);

    //默认的PIPE信号是关闭程序，当我们写一个被客户端关闭的套接字时会触发这个信号，需要忽视掉。
    sigact.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sigact, NULL);

    LPSGLSVR->Init(argc, argv);
    LPSGLSVR->Run();

    return 0;
}

//
// Created by czllo on 2017/4/01.
//
#include <iostream>
#include <string>
#include <signal.h>
#include <bits/sigaction.h>

#include "server_config.h"
#include "server.h"

using namespace std;

static void TermSigHandler(int sig) {
    LPSGLSVR->StopService();
}

static void SigpipeHandler(int sig) {
    LOG_INFO("pipe");
}

int main (int argc, char **argv) {
    if (argc < 2) {
        cout << "use: server ${path}/xxx.ini" << endl;
        cout << "from chile." << endl;
        return -1;
    }

    if (SingletonServerCfg::instance()->Init(argv[1]) != 0) {
        return -1;
    }

    if (SingletonServerCfg::instance()->daemon_) {
        daemon(1, 0);
    }

    //hook sig
    struct sigaction sigact;
    sigact.sa_handler = TermSigHandler;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);
    sigaction(SIGABRT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGUSR1, &sigact, NULL);
    
    sigact.sa_handler = SigpipeHandler;
    sigaction(SIGPIPE, &sigact, NULL);

    LPSGLSVR->Run();

    return 0;
}

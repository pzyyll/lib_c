//
// Created by czllo on 2017/4/01.
//
#include <iostream>
#include <string>

#include "server_config.h"
#include "server.h"

using namespace std;

int main (int argc, char **argv) {
    if (argc < 2) {
        cout << "use: server ${path}/xxx.ini" << endl;
        cout << "from chile." << endl;
        return -1;
    }

    if (SingletonServerCfg::instance()->Init("server.ini"/*argv[1]*/) != 0) {
        return -1;
    }

    if (SingletonServerCfg::instance()->daemon_) {
        daemon(1, 0);
    }

    return 0;
}

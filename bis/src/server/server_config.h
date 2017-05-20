//
// Created by czllo on 2017/4/26.
//

#ifndef SERVER_SERVER_CONFIG_H
#define SERVER_SERVER_CONFIG_H

#include <string>
#include <iostream>
#include <sys/stat.h>
#include <sys/errno.h>

#include "singleton.h"
#include "ini_file.h"
#include "log_def.h"
#include "comm/buffer/buffer_sequence.h"

const unsigned int MAX_CMD_LEN = 4096 * 8;
const int BUFFER_SEQUCNE_COUNT = 3;
const int SUCCSESS = 0;
const int FAIL = -1;

class ServerConfig {
public:
    int Init(const char *cstrFileName);

private:
    int InitLog();
public:
    //[SERVER]
    int daemon_;
    std::string ip_;
    int port_;
    int time_out_;
    int free_time_sleep_;

//    [MQ]
    std::string pull_addr_;

//    [DB]
    std::string db_file_;
    unsigned long long file_max_size_;
    int persistend_time_;

    //[LOG]
    std::string log_path_;
    int log_level_;
};

typedef Singleton<ServerConfig> SingletonServerCfg;
#define LPSVRCFG SingletonServerCfg::instance()
typedef snslib::BufferSequence<BUFFER_SEQUCNE_COUNT> BufferSequenceType;

#endif //SERVER_SERVER_CONFIG_H

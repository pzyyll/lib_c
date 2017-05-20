//
// @Created by czllo.
// @brief 执行DB操作，参考了Redis的实现方法
//

#ifndef SERVER_SLDB_H
#define SERVER_SLDB_H

#include <iostream>
#include <string>
#include <cstdio>
#include <cstddef>
#include <netinet/in.h>
#include "proto/bs_db.pb.h"
//#include "db.h"
#include "log_def.h"

const uint32_t LEN_ERR = UINT_MAX;

class sldb {
public:
    sldb();
    ~sldb();

    int Init(const char *file, const char *mode);
    size_t FileWrite(const void *buf, size_t len);
    size_t FileRead(void *buf, size_t len);
    off_t FileTell();
    int FileFlush();
    int FileFsync();
    int FileClose();

    int SaveType(unsigned char type);
    int LoadType();

    int SaveLen(uint32_t len);
    uint32_t LoadLen();

    ssize_t SaveRawString(const std::string &str);
    int LoadRawString(std::string &str);

    int SaveObj(const ::google::protobuf::Message &obj, unsigned char type);
    int LoadObj(::google::protobuf::Message &obj);

public:
    unsigned long long  buffered;
    FILE *fp;
};


#endif //SERVER_SLDB_H

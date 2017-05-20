//
// @Created by czllo .
// @brief 数据库操作
//

#ifndef SERVER_DB_H
#define SERVER_DB_H

#include <unordered_map>
#include <utility>
#include <memory>
#include <string>

#include <ctime>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/prctl.h>

#include "log_def.h"
#include "sort_set.h"
#include "singleton.h"
#include "server_config.h"
#include "sldb.h"
#include "server.h"

static inline long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec) * 1000000;
    ust += tv.tv_usec;
    return ust;
}

class db {
public:
    typedef std::shared_ptr<sort_set> SSetPtr;
    typedef std::unordered_map<std::string, SSetPtr> db_type;
    typedef sort_set::node_pair node_pair_type;
    typedef sort_set::iterator node_itr_type;
public:

    db() : is_loading(false) { }
    ~db() { }

    bool Insert(const std::string &key, const double score, const std::string &data);
    bool DelMem(const std::string &key, const std::string &data);
    bool GetMemScore(const std::string &key, const std::string &data, double &score);
    bool GetMemRank(const std::string &key, const std::string &data, unsigned long long &rank);
    node_pair_type GetMemsByScore(const std::string &key, const double min, const double max);
    node_itr_type GetMemsFirstByRank(const std::string &key, const unsigned long long min, const unsigned long long max);
    node_itr_type GetMemByRank(const std::string &key, const unsigned long long rank);

    //@CZL 请Fork一个后在去写文件！！！
    int SaveToFile();
    int CheckDbFileAndLoad();
    int BgSaveDb();

    const std::string &GetErr() { return strErr; }
    bool get_is_loading() const { return is_loading; }
    void set_is_loading(bool is_loading_) { is_loading = is_loading_; }

    node_itr_type end() { return node_itr_type(NULL, NULL); }
private:
    SSetPtr FindSSet(const std::string &key);
private:
    db_type db_dict;
    std::string strErr;
    bool is_loading;
};

typedef Singleton<db> SingletonDb;
#define LPDB SingletonDb::instance()


#endif //SERVER_DB_H

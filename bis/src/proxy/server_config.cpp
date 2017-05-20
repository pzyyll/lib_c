//
// Created by czllo on 2017/4/26.
//

#include "server_config.h"

using namespace std;

int ServerConfig::Init(const char *cstrFileName) {
    int ret = 0;

    CIniFile ini(cstrFileName);

    //LOG
    ini.GetString("LOG", "Path", "./log/", log_path_);
    ini.GetInt("LOG", "Level", 0, &log_level_);
    ret = InitLog();
    if (ret < 0) {
        cout << "Init Log Fail." << endl;
        return 0;
    }

    //SERVER
    ini.GetInt("SERVER", "Daemon", 1, &daemon_);
    ini.GetString("SERVER", "IP", "", ip_);
    ini.GetInt("SERVER", "Port", 20309, &port_);
    ini.GetInt("SERVER", "TimeOut", 0, &time_out_);
    ini.GetInt("SERVER", "FreeSleepTime", 0, &free_time_sleep_);
    ini.GetInt("SERVER", "BucketNum", 2048, &bucket_num_);

    //MQ
    LoadMQ(ini);

    cout << "==========SERVER============" << endl;
    cout << "Auth   : CaiZhiLi " << endl;
    cout << "Daemon : " << daemon_ << endl;
    cout << "IP     : " << ip_ << endl;
    cout << "Port   : " << port_ << endl;
    cout << "TimeOut: " << time_out_ << endl;
    cout << "FreeSleep : " << free_time_sleep_ << endl;
    cout << "BucketNum : " << bucket_num_ << endl;

    LOG_INFO("=======INIT SERVER=========");
    return ret;
}

int ServerConfig::LoadMQ(CIniFile &ini) {
    int svr_num = 0;
    ini.GetInt("MQ", "SvrNum", 0, &svr_num);

    for (int i = 0; i < svr_num; ++i) {
        string getaddr = "PushAddr" + to_string(i + 1);
        string addr;
        ini.GetString("MQ", getaddr.c_str(), "", addr);
        push_addrs_.push_back(addr);
        LOG(INFO) << getaddr << ":" << addr << endl;
    }
    ini.GetString("MQ", "PullAddr", "", pull_addr_);
    LOG(INFO) << "PullAddr:" << pull_addr_ << endl;
    return 0;
}

int ServerConfig::InitLog() {
    int ret = 0;
    ret = mkdir(log_path_.c_str(), S_IRWXU);
    if (ret < 0 && errno == ENOENT) {
        switch (errno) {
            case EEXIST:
                ret = 0;
                break;
            default:
                std::cout << "log path not exit!" << std::endl;
                return ret;
        }
    }

    if (log_path_.back() != '/')
        log_path_.push_back('/');
    string info_path = log_path_ + "info/";
    string warn_path = log_path_ + "warn/";
    string err_path = log_path_ + "error/";

    mkdir(info_path.c_str(), S_IRWXU);
    mkdir(warn_path.c_str(), S_IRWXU);
    mkdir(err_path.c_str(), S_IRWXU);

    info_path.append("log_");
    warn_path.append("log_");
    err_path.append("log_");

    iniglog("server");
    set_log_infopath(info_path.c_str());
    set_log_warnpath(warn_path.c_str());
    set_log_errorpath(err_path.c_str());
    set_logbufsecs(0);
    set_minloglevel(log_level_);

    return 0;
}
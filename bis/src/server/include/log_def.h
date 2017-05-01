//
// Created by czllo on 2017/4/26.
//

#ifndef SERVER_LOG_DEF_H
#define SERVER_LOG_DEF_H

#include <iostream>
#include <stdarg.h>
#include <glog/logging.h>

#ifdef LOG_DEF

inline void iniglog(const char *cstrArgv0) {
    google::InitGoogleLogging(cstrArgv0);
}

inline void deiniglog() {
    google::ShutdownGoogleLogging();
}

inline void set_logbufsecs(int secs) {
    FLAGS_logbufsecs = secs;
}

inline void set_minloglevel(int level) {
    FLAGS_minloglevel = level;
}

inline void set_log_infopath(const char *cstrPath) {
    google::SetLogDestination(google::GLOG_INFO, cstrPath);
}
inline void set_log_warnpath(const char *cstrPath) {
    google::SetLogDestination(google::GLOG_WARNING, cstrPath);
}
inline void set_log_errorpath(const char *cstrPath) {
    google::SetLogDestination(google::GLOG_ERROR, cstrPath);
}

inline void LOG_INFO(const char *cstrFormat, ...) {
    char buf[2048] = {0};
    va_list args;
    va_start(cstrFormat, s);
    vsnprintf(buf, sizeof(buf), s, args);
    va_end(args);

    google::LOG(google::INFO) << buf << std::endl;
}

inline void LOG_WARN(const char *cstrFormat, ...) {
    char buf[2048] = {0};
    va_list args;
    va_start(cstrFormat, s);
    vsnprintf(buf, sizeof(buf), s, args);
    va_end(args);

    google::LOG(google::WARNING) << buf << std::endl;
}

inline void LOG_ERR(const char *cstrFormat, ...) {
    char buf[2048] = {0};
    va_list args;
    va_start(cstrFormat, s);
    vsnprintf(buf, sizeof(buf), s, args);
    va_end(args);

    google::LOG(google::ERROR) << buf << std::endl;
}

#else

inline void iniglog(const char *cstrArgv0) {

}

inline void deiniglog() {

}

inline void set_logbufsecs(int secs) {

}

inline void set_minloglevel(int level) {

}

inline void set_log_infopath(const char *cstrPath) {

}
inline void set_log_warnpath(const char *cstrPath) {

}
inline void set_log_errorpath(const char *cstrPath) {

}

inline void LOG_INFO(const char *cstrFormat, ...) {

}

inline void LOG_WARN(const char *cstrFormat, ...) {

}

inline void LOG_ERR(const char *cstrFormat, ...) {

}

#endif //LOG_DEF

#endif //SERVER_LOG_DEF_H

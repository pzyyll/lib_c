#ifndef PAYSVR_PROTO_H_
#define PAYSVR_PROTO_H_

#include "api/paysvr_api/paysvr_def.h"

namespace snslib
{

class CPaySvrProto
{
public:
    static int Read(const char * buf, snslib::PayHeader & header);
    static int Write(char * buf, const snslib::PayHeader & header);
    static int Read(const char * buf, snslib::PayRequest & req);
    static int Write(char * buf, const snslib::PayRequest & req);
    static int Read(const char * buf, snslib::PayInnerRequest & req);
    static int Write(char * buf, const snslib::PayInnerRequest & req);
    static int Read(const char * buf, snslib::PayAttr & attr);
    static int Write(char * buf, const snslib::PayAttr & attr);
    static int Read(const char * buf, snslib::PayAns & ans);
    static int Write(char * buf, const snslib::PayAns & ans);
};

}

#endif /* PAYSVR_PROTO_H_ */

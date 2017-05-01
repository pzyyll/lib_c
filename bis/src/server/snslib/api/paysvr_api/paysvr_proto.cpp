#include <string.h>
#include "paysvr_proto.h"
#include "comm/util/pet_util.h"

using namespace snslib;

int CPaySvrProto::Read(const char * buf, PayHeader & header)
{
    int offset = 0;
    offset += CBuffTool::ReadShort(buf + offset, header.version);
    offset += CBuffTool::ReadShort(buf + offset, header.cmd);
    offset += CBuffTool::ReadShort(buf + offset, header.payID);
    offset += CBuffTool::ReadShort(buf + offset, header.length);

    return offset;
}

int CPaySvrProto::Write(char * buf, const PayHeader & header)
{
    int offset = 0;
    offset += CBuffTool::WriteShort(buf + offset, header.version);
    offset += CBuffTool::WriteShort(buf + offset, header.cmd);
    offset += CBuffTool::WriteShort(buf + offset, header.payID);
    offset += CBuffTool::WriteShort(buf + offset, header.length);

    return offset;
}

int CPaySvrProto::Read(const char * buf, PayRequest & req)
{
    int offset = 0;
    offset += CBuffTool::ReadByte(buf + offset, req.channel);
    offset += CBuffTool::ReadByte(buf + offset, req.itemType);
    offset += CBuffTool::ReadInt(buf + offset, req.itemCount);
    for (unsigned int i = 0; i < req.itemCount; ++i)
    {
        offset += CBuffTool::ReadLongLong(buf + offset, req.items[i].itemID);
        offset += CBuffTool::ReadShort(buf + offset, req.items[i].count);
    }
    offset += CBuffTool::ReadInt(buf + offset, req.price);
    unsigned short len = 0;
    offset += CBuffTool::ReadShort(buf + offset, len);
    offset += CBuffTool::ReadString(buf + offset, req.payInfo, len);
    req.payInfo[len] = 0;
    offset += CBuffTool::ReadInt(buf + offset, req.payUin);
    offset += CBuffTool::ReadInt(buf + offset, req.provideUin);
    offset += CBuffTool::ReadLongLong(buf + offset, req.providePetID);
    offset += CBuffTool::ReadByte(buf + offset, req.sessionType);
    offset += CBuffTool::ReadShort(buf + offset, len);
    offset += CBuffTool::ReadString(buf + offset, req.sessionKey, len);
    req.sessionKey[len] = 0;
    offset += CBuffTool::ReadShort(buf + offset, len);
    offset += CBuffTool::ReadString(buf + offset, req.clientIP, len);
    req.clientIP[len] = 0;
    offset += CBuffTool::ReadByte(buf + offset, req.vip);

    return offset;
}

int CPaySvrProto::Write(char * buf, const PayRequest & req)
{
    int offset = 0;
    offset += CBuffTool::WriteByte(buf + offset, req.channel);
    offset += CBuffTool::WriteByte(buf + offset, req.itemType);
    offset += CBuffTool::WriteInt(buf + offset, req.itemCount);
    for (unsigned int i = 0; i < req.itemCount; ++i)
    {
        offset += CBuffTool::WriteLongLong(buf + offset, req.items[i].itemID);
        offset += CBuffTool::WriteShort(buf + offset, req.items[i].count);
    }
    offset += CBuffTool::WriteInt(buf + offset, req.price);
    unsigned short len = strlen(req.payInfo);
    offset += CBuffTool::WriteShort(buf + offset, len);
    offset += CBuffTool::WriteString(buf + offset, req.payInfo, len);
    offset += CBuffTool::WriteInt(buf + offset, req.payUin);
    offset += CBuffTool::WriteInt(buf + offset, req.provideUin);
    offset += CBuffTool::WriteLongLong(buf + offset, req.providePetID);
    offset += CBuffTool::WriteByte(buf + offset, req.sessionType);
    len = strlen(req.sessionKey);
    offset += CBuffTool::WriteShort(buf + offset, len);
    offset += CBuffTool::WriteString(buf + offset, req.sessionKey, len);
    len = strlen(req.clientIP);
    offset += CBuffTool::WriteShort(buf + offset, len);
    offset += CBuffTool::WriteString(buf + offset, req.clientIP, len);
    offset += CBuffTool::WriteByte(buf + offset, req.vip);

    return offset;
}

int CPaySvrProto::Read(const char * buf, PayInnerRequest & req)
{
    int offset = 0;
    offset += CBuffTool::ReadLongLong(buf + offset, req.providePetID);
    offset += CBuffTool::ReadInt(buf + offset, req.price);
    offset += CBuffTool::ReadByte(buf + offset, req.itemType);
    offset += CBuffTool::ReadInt(buf + offset, req.itemCount);
    for (unsigned int i = 0; i < req.itemCount; ++i)
    {
        offset += CBuffTool::ReadLongLong(buf + offset, req.items[i].itemID);
        offset += CBuffTool::ReadShort(buf + offset, req.items[i].count);
    }

    return offset;
}

int CPaySvrProto::Write(char * buf, const PayInnerRequest & req)
{
    int offset = 0;
    offset += CBuffTool::WriteLongLong(buf + offset, req.providePetID);
    offset += CBuffTool::WriteInt(buf + offset, req.price);
    offset += CBuffTool::WriteByte(buf + offset, req.itemType);
    offset += CBuffTool::WriteInt(buf + offset, req.itemCount);
    for (unsigned int i = 0; i < req.itemCount; ++i)
    {
        offset += CBuffTool::WriteLongLong(buf + offset, req.items[i].itemID);
        offset += CBuffTool::WriteShort(buf + offset, req.items[i].count);
    }

    return offset;
}

int CPaySvrProto::Read(const char * buf, PayAttr & attr)
{
    int offset = 0;
    offset += CBuffTool::ReadInt(buf + offset, attr.yb);
    offset += CBuffTool::ReadInt(buf + offset, attr.growth);
    offset += CBuffTool::ReadInt(buf + offset, attr.starvation);
    offset += CBuffTool::ReadInt(buf + offset, attr.cleanness);
    offset += CBuffTool::ReadShort(buf + offset, attr.feeling);
    offset += CBuffTool::ReadInt(buf + offset, attr.strong);
    offset += CBuffTool::ReadInt(buf + offset, attr.iq);
    offset += CBuffTool::ReadInt(buf + offset, attr.charm);
    offset += CBuffTool::ReadInt(buf + offset, attr.love);

    return offset;
}

int CPaySvrProto::Write(char * buf, const PayAttr & attr)
{
    int offset = 0;
    offset += CBuffTool::WriteInt(buf + offset, attr.yb);
    offset += CBuffTool::WriteInt(buf + offset, attr.growth);
    offset += CBuffTool::WriteInt(buf + offset, attr.starvation);
    offset += CBuffTool::WriteInt(buf + offset, attr.cleanness);
    offset += CBuffTool::WriteShort(buf + offset, attr.feeling);
    offset += CBuffTool::WriteInt(buf + offset, attr.strong);
    offset += CBuffTool::WriteInt(buf + offset, attr.iq);
    offset += CBuffTool::WriteInt(buf + offset, attr.charm);
    offset += CBuffTool::WriteInt(buf + offset, attr.love);

    return offset;
}

typedef struct tagPayAns
{
    int retCode;
    char displayInfo[PAY_MAX_ANS_DISPLAY_INFO_LEN + 1];
    char serialNo[PAY_MAX_ANS_SERIALNO_LEN + 1];
    int money;
    int qb;
    int qpoint;

} PayAns;

int CPaySvrProto::Read(const char * buf, PayAns & ans)
{
    int offset = 0;
    offset += CBuffTool::ReadInt(buf + offset, ans.retCode);
    unsigned short len = 0;
    offset += CBuffTool::ReadShort(buf + offset, len);
    offset += CBuffTool::ReadString(buf + offset, ans.displayInfo, len);
    ans.displayInfo[len] = 0;
    offset += CBuffTool::ReadShort(buf + offset, len);
    offset += CBuffTool::ReadString(buf + offset, ans.serialNo, len);
    ans.serialNo[len] = 0;
    offset += CBuffTool::ReadInt(buf + offset, ans.money);
    offset += CBuffTool::ReadInt(buf + offset, ans.qb);
    offset += CBuffTool::ReadInt(buf + offset, ans.qpoint);

    return offset;
}

int CPaySvrProto::Write(char * buf, const PayAns & ans)
{
    int offset = 0;
    offset += CBuffTool::WriteInt(buf + offset, ans.retCode);
    unsigned short len = strlen(ans.displayInfo);
    offset += CBuffTool::WriteShort(buf + offset, len);
    offset += CBuffTool::WriteString(buf + offset, ans.displayInfo, len);
    len = strlen(ans.serialNo);
    offset += CBuffTool::WriteShort(buf + offset, len);
    offset += CBuffTool::WriteString(buf + offset, ans.serialNo, len);
    offset += CBuffTool::WriteInt(buf + offset, ans.money);
    offset += CBuffTool::WriteInt(buf + offset, ans.qb);
    offset += CBuffTool::WriteInt(buf + offset, ans.qpoint);

    return offset;
}

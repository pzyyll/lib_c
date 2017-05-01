//
// Created by czllo.
//

#include "cmd.h"

Cmd::Cmd() : uCmdId_(0), uCheck_(0), uMsgIdx_(0), strAppData_("") {

}

Cmd::~Cmd() {

}

int Cmd::Parse(const char *cszDataBuf, size_t uiDataLen) {
    if (uiDataLen < sizeof(MsgHead)) {
        LOG_WARN("pkg recv is not fin, len=%u", static_cast<unsigned int>(uiDataLen));
        return 0;
    }

    const MsgHead *pMsgHeader = reinterpret_cast<const MsgHead *>(cszDataBuf);
    unsigned int uiPkgLen = ntohl(pMsgHeader->msg_len);
    if (uiPkgLen > MAX_CMD_LEN) {
        LOG_WARN("pkglen(%u) is too long than MAX_CMD_LEN(%u).", uiPkgLen, MAX_CMD_LEN);
        return -1;
    }

    if (uiPkgLen > uiDataLen) {
        LOG_WARN("pkg is incomplete.pkglen=%u, DataLen=%u", uiPkgLen, static_cast<unsigned int>(uiDataLen));
        return  0;
    }

    uCmdId_ = ntohs(pMsgHeader->cmd_id);
    uCheck_ = ntohs(pMsgHeader->check);

    //无加密处理，最好还是加密数据！
    set_app_data(cszDataBuf + sizeof(MsgHead), uiPkgLen - sizeof(MsgHead));
    LOG_INFO("Body Data Hex|%s", snslib::CStrTool::Str2Hex(strAppData_.c_str(), strAppData_.size()));
    return uiPkgLen;
}

bool Cmd::Serialize(std::string &strData) {
    strData.clear();
    MsgHead msgHead;
    msgHead.cmd_id = htons(uCmdId_);
    msgHead.check = htons(uCheck_);
    msgHead.msg_len = htonl(sizeof(MsgHead) + strAppData_.size());
    strData.append(reinterpret_cast<char *>(&msgHead), sizeof(MsgHead));
    strData.append(strAppData_);
    return true;
}

bool Cmd::Serialize(BufferSequenceType &bs) {
    snslib::DynamicBuffer db1;
    MsgHead *pMsgHeader = reinterpret_cast<MsgHead *>(db1.Borrow(sizeof(MsgHead)));
    if (NULL == pMsgHeader)
        return false;

    memset(pMsgHeader, 0, sizeof(MsgHead));
    pMsgHeader->cmd_id = htons(uCmdId_);
    pMsgHeader->msg_len = htonl(sizeof(MsgHead) + strAppData_.size());

    bs += db1;
    snslib::StaticBuffer sb2(strAppData_.c_str(), strAppData_.size());
    bs += sb2;

    pMsgHeader->check = htons(bs.CheckSum());

    return true;
}

bool Cmd::Serialize(char *cszDataBuf, size_t &uiDataLen) {
    unsigned int uiPkgLen = sizeof(MsgHead) + strAppData_.size();
    if (NULL == cszDataBuf || uiDataLen < uiPkgLen) {
        return false;
    }
    MsgHead *pMsgHeader = reinterpret_cast<MsgHead *>(cszDataBuf);
    pMsgHeader->cmd_id = htons(uCmdId_);
    pMsgHeader->msg_len = htonl(uiPkgLen);

    memcpy(cszDataBuf + sizeof(MsgHead), strAppData_.c_str(), strAppData_.size());

    pMsgHeader->check = htons(snslib::CStrTool::CheckSum(cszDataBuf, uiPkgLen));
    return true;
}

void Cmd::Clear() {
    uCmdId_ = 0;
    uCheck_ = 0;
    uMsgIdx_ = 0;
    strAppData_.clear();
}

unsigned Cmd::get_cmd_id() const {
    return uCmdId_;
}

unsigned Cmd::get_check() const {
    return uCheck_;
}

unsigned Cmd::get_msg_idx() const {
    return uMsgIdx_;
}

const std::string &Cmd::get_app_data() const {
    return strAppData_;
}

void Cmd::set_cmd_id(unsigned short uCmdId) {
    uCmdId_ = uCmdId;
}

void Cmd::set_check(unsigned short uCheck) {
    uCheck_ = uCheck;
}

void Cmd::set_msg_idx(unsigned uMsgIdx) {
    uMsgIdx_ = uMsgIdx;
}

void Cmd::set_app_data(const char *cszDataBuff, size_t uiDataLen) {
    strAppData_.assign(cszDataBuff, uiDataLen);
}

void Cmd::set_app_data(const std::string &strData) {
    strAppData_ = strData;
}



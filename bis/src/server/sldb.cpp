//
// @Created by czllo.
//

#include "sldb.h"

sldb::sldb() : buffered(0), fp(NULL) {

}

sldb::~sldb() {
    if (NULL != fp) {
        FileFlush();
        fclose(fp);
        fp = NULL;
    }
}

int sldb::Init(const char *file, const char *mode) {
    if ((fp = fopen(file, mode)) == NULL) return -1;
    buffered = 0;
    return 0;
}

size_t sldb::FileWrite(const void *buf, size_t len) {
    size_t retval;

    retval = fwrite(buf,len,1,fp);
    if (retval <= 0) {
        return 0;
    }
    buffered += len;
    return retval;
}

size_t sldb::FileRead(void *buf, size_t len) {
    return fread(buf,len,1,fp);
}

off_t sldb::FileTell() {
    return ftello(fp);
}

int sldb::FileFlush() {
    return (fflush(fp) != 0) ? -1 : 0;
}

int sldb::FileFsync() {
    return (fsync(fileno(fp)) != 0) ? -1 : 0;
}

int sldb::FileClose() {
    if (NULL == fp)
        return EOF;
    int ret = fclose(fp);
    fp = NULL;
    LOG(INFO) << "Close Fp!" << std::endl;
    return ret;
}

int sldb::SaveType(unsigned char type) {
    return FileWrite(&type,1);
}

int sldb::LoadType() {
    unsigned char type;
    if (FileRead(&type,1) == 0) return -1;
    return type;
}

int sldb::SaveLen(uint32_t len) {
    unsigned char buf[2];
    size_t nwritten = 0;

    if (len < (1<<6)) {
        /* Save a 6 bit len */
        buf[0] = (len&0xFF)|(bs_czl::MsgLenType::BS_6BIT<<6);
        if (FileWrite(buf,1) == 0) return -1;
        nwritten = 1;
    } else if (len < (1<<14)) {
        /* Save a 14 bit len */
        buf[0] = ((len>>8)&0xFF)|(bs_czl::MsgLenType::BS_14BIT<<6);
        buf[1] = len&0xFF;
        if (FileWrite(buf,2) == 0) return -1;
        nwritten = 2;
    } else {
        /* Save a 32 bit len */
        buf[0] = (bs_czl::MsgLenType::BS_32BIT<<6);
        if (FileWrite(buf,1) == 0) return -1;
        len = htonl(len);
        if (FileWrite(&len,4) == 0) return -1;
        nwritten = 1+4;
    }
    return nwritten;
}

uint32_t sldb::LoadLen() {
    unsigned char buf[2];
    uint32_t len;
    int type;

    if (FileRead(buf,1) == 0) return LEN_ERR;
    type = (buf[0]&0xC0)>>6;
    if (type == bs_czl::MsgLenType::BS_6BIT) {
        /* Read a 6 bit len. */
        return buf[0]&0x3F;
    } else if (type == bs_czl::MsgLenType::BS_14BIT) {
        /* Read a 14 bit len. */
        if (FileRead(buf+1,1) == 0) return LEN_ERR;
        return ((buf[0]&0x3F)<<8)|buf[1];
    } else if (type == bs_czl::MsgLenType::BS_32BIT) {
        /* Read a 32 bit len. */
        if (FileRead(&len,4) == 0) return LEN_ERR;
        return ntohl(len);
    } else {
        LOG(WARNING) << "Lenth type err(" << type << ")"  << std::endl;
        return -1; /* Never reached. */
    }
}

ssize_t sldb::SaveRawString(const std::string &str) {
    ssize_t n =0, nwriten = 0;
    if (str.size() > 0) {
        if ((n = SaveLen(str.size())) <= 0) return -1;
        nwriten += n;
        if (FileWrite(str.c_str(), str.size()) <= 0) return -1;
        nwriten += str.size();
    }
    return nwriten;
}

int sldb::LoadRawString(std::string &str) {
    uint32_t len = 0;
    len = LoadLen();
    if (LEN_ERR == len) {
        LOG(ERROR) << "Load Len(" << len << ") err." << std::endl;
        return -1;
    }
    char *buf = new char[len];
    if (NULL != buf) {
        if (FileRead(buf, len) <= 0) {
            LOG(ERROR) << "Load Len(" << len << ") err." << std::endl;
            return -1;
        }
        str.assign(buf, len);
    }
    delete[] buf;
    return 0;
}

int sldb::SaveObj(const ::google::protobuf::Message &obj, unsigned char type) {
    ssize_t n = 0, nwriten = 0;
    if ((n = SaveType(type) ) <= 0) return -1;
    nwriten += n;
    if ((n = SaveRawString(obj.SerializeAsString()) ) <= 0) return -1;
    nwriten += n;

    return nwriten;
}

int sldb::LoadObj(::google::protobuf::Message &obj) {
    /*
    if ((type = LoadType()) < 0) {
        LOG(ERROR) << "Load Type Err|" << type << std::endl;
        return -1;
    }
    */
    std::string strBuf;
    if (LoadRawString(strBuf) < 0) {
        LOG(ERROR) << "Load String Err|" << std::endl;
        return -1;
    }

    if (!obj.ParseFromString(strBuf)) {
        LOG(ERROR) << "Load Parse To Ojb Err|"<< strBuf << std::endl;
        return -1;
    }
    /*
    switch (type) {
        case bs_czl::MsgNodeType::VERSION: {

            break;
        }
        case bs_czl::MsgNodeType::DB: {
            break;
        }
        case bs_czl::MsgNodeType::TABLE: {
            break;
        }
        case bs_czl::MsgNodeType::KEY: {
            break;
        }
        default: {
            LOG(ERROR) << "Never Go Here!!" << std::endl;
            return -1;
        }
    }
    */
    return 0;
}





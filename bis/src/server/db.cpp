//
// Created by czllo on 2017/5/1.
//

#include "db.h"

using namespace std;

bool db::Insert(const std::string &key, const double score, const std::string &data) {
    //strErr.clear();
    SSetPtr sset = FindSSet(key);
    if (NULL == sset) {
        sset = SSetPtr(new sort_set);
        assert(sset != NULL);
        db_dict.insert(make_pair(key, sset));
    }
    LOG(INFO) << "Insert score " << score << " data: " << data << endl;
    return sset->Update(score, data);
}

bool db::DelMem(const std::string &key, const std::string &data) {
    strErr.clear();
    SSetPtr sset = FindSSet(key);
    if (NULL == sset) {
        strErr = "Key is not exist!";
        LOG(WARNING) << key << " " << strErr << endl;
        return false;
    }

    if (!sset->DelMem(data)) {
        strErr = "Data is not exist!";
        LOG(WARNING) << data << " " << strErr << endl;
        return false;
    }
    return true;
}

bool db::GetMemScore(const std::string &key, const std::string &data, double &score) {
    strErr.clear();
    SSetPtr sset = FindSSet(key);
    if (NULL == sset) {
        strErr = "Key is not exist!";
        LOG(WARNING) << key << " " << strErr << endl;
        return false;
    }
    if (!sset->GetMemScore(data, score)) {
        strErr = "Data is not exist!";
        LOG(WARNING) << data << " " << strErr << endl;
        return false;
    }
    LOG(INFO) << score << " get socre succ." << endl;
    return true;
}

bool db::GetMemRank(const std::string &key, const std::string &data, unsigned long long &rank) {
    strErr.clear();
    SSetPtr sset = FindSSet(key);
    if (NULL == sset) {
        strErr = "Key is not exist!";
        LOG(WARNING) << key << " " << strErr << endl;
        return false;
    }

    if (!sset->GetMemRank(data, rank)) {
        strErr = "Data is not exist!";
        LOG(WARNING) << data << " " << strErr << endl;
        return false;
    }

    LOG(INFO) << rank << " get rank succ." << endl;
    return true;
}

db::node_pair_type db::GetMemsByScore(const std::string &key, const double min, const double max) {
    strErr.clear();
    SSetPtr sset = FindSSet(key);
    if (NULL == sset) {
        strErr = "Key is not exist!";
        LOG(WARNING) << key << " " << strErr << endl;
        return make_pair(end(), end());
    }

    node_pair_type pq = sset->GetMemsByScore(min, max);
    if (pq.first == sset->end()) {
        strErr = "Data is not exist.";
        LOG(INFO) << "Data is not exist" << endl;
        return make_pair(end(), end());
    }

    return sset->GetMemsByScore(min, max);
}

db::node_itr_type
db::GetMemsFirstByRank(const std::string &key, const unsigned long long min, const unsigned long long max) {
    strErr.clear();
    SSetPtr sset = FindSSet(key);
    if (NULL == sset) {
        strErr = "Key is not exist!";
        LOG(WARNING) << key << " " << strErr << endl;
        return end();
    }
    node_itr_type itr = sset->GetMemsFirstByRank(min, max);
    if (sset->end() == itr) {
        strErr = "Data is not exist";
        LOG(INFO) << "Data is not exist" << endl;
        return end();
    }
    return itr;
}

db::node_itr_type db::GetMemByRank(const std::string &key, const unsigned long long rank) {
    strErr.clear();
    SSetPtr sset = FindSSet(key);
    if (NULL == sset) {
        strErr = "Key is not exist!";
        LOG(WARNING) << key << " " << strErr << endl;
        return end();
    }

    node_itr_type itr = sset->GetMemByRank(rank);
    if (itr == sset->end()) {
        strErr = "Data is not exist";
        LOG(INFO) << "Data is not exist" << endl;
        return end();
    }

    return itr;
}

db::SSetPtr db::FindSSet(const std::string &key) {
    db_type::iterator itr = db_dict.find(key);
    if (db_dict.end() == itr) {
        return SSetPtr();
    }
    return itr->second;
}

int db::SaveToFile() {
    sldb hld_save_db;
    string tmp_file = LPSVRCFG->db_file_ + ".tmp";
    if (hld_save_db.Init(tmp_file.c_str(), "w") < 0) {
        LOG(INFO) << "Open Tmp File Fial! " << strerror(errno) << endl;
        return -1;
    }

    time_t now = time(NULL);
    char *cszNow = ctime(&now);
    bs_czl::MsgDbVersion db_ver;
    db_ver.set_version("BIS_V1");
    db_ver.set_auth("CaiZhiLi");
    db_ver.set_save_time(cszNow);
    LOG(INFO) << db_ver.ShortDebugString() << endl;

    if (hld_save_db.SaveObj(db_ver, bs_czl::MsgNodeType::VERSION) < 0) {
        LOG(ERROR) << "Save Db Version Info Fail" << endl;
        return -1;
    }

    bs_czl::MsgDb db1;
    db1.set_num(1);
    LOG(INFO) << db1.ShortDebugString() << endl;
    if (hld_save_db.SaveObj(db1, bs_czl::MsgNodeType::DB) < 0) {
        LOG(ERROR) << "Save Db Num Info Fail" << endl;
        return -1;
    }

    //TODO 嵌套层数太多了，优化优化
    for (db_type::iterator itr = db_dict.begin(); itr != db_dict.end(); ++itr) {
        bs_czl::MsgDbTable table;
        table.set_key_id(itr->first);
        table.set_length(itr->second->size());
        LOG(INFO) << table.ShortDebugString() << endl;

        if (hld_save_db.SaveObj(table, bs_czl::MsgNodeType::TABLE) < 0) {
            LOG(ERROR) << "Save Db Table Info Fail.(" << table.key_id() << ")" << endl;
            continue;
        }
        //50个key-value为一组存吧
        int cnt = 0;
        bs_czl::MsgDbKey kvs;
        for (node_itr_type sl_itr = itr->second->begin(); sl_itr != itr->second->end(); ++sl_itr) {
            kvs.add_key(sl_itr->score);
            kvs.add_val(sl_itr->data.val);
            LOG(INFO) << kvs.ShortDebugString() << endl;
            if (++cnt % 50 == 0) {
                if (hld_save_db.SaveObj(kvs, bs_czl::MsgNodeType::KEY) < 0) {
                    LOG(ERROR) << "Save Db KVS to Table Fail.(" << kvs.ShortDebugString() << ")" << endl;
                }
                cnt = 0;
                kvs.Clear();
            }
        }
        //不满50
        if (kvs.key_size() > 0 && kvs.val_size() > 0) {
            if (hld_save_db.SaveObj(kvs, bs_czl::MsgNodeType::KEY) < 0) {
                LOG(ERROR) << "Save Db KVS to Table Fail.(" << kvs.ShortDebugString() << ")" << endl;
            }
            kvs.Clear();
        }
    }

    //确保所有内存中的数据都写到磁盘上
    if (hld_save_db.FileFlush() < 0 || hld_save_db.FileFsync() < 0 || hld_save_db.FileClose() < 0 ) {
        LOG(ERROR) << "Write Db File Err : " << strerror(errno) << endl;
        return -1;
    }

    char cwd[MAXPATHLEN] = {0};
    if (rename(tmp_file.c_str(), LPSVRCFG->db_file_.c_str()) < 0) {
        char *cwdp = getcwd(cwd, MAXPATHLEN);
        LOG(ERROR) << "Err Moving Tmp Db File Fail. "
                 << "|tmp=" << tmp_file
                 << "|new_file=" << LPSVRCFG->db_file_
                 << "|dest=" << (cwdp ? cwdp : "unknown")
                 << "|err=" << strerror(errno) << endl;
        unlink(tmp_file.c_str());
        return -1;
    }

    LOG(INFO) << "Save Db Done." << endl;

    return 0;
}

int db::CheckDbFileAndLoad() {
    sldb hld_load_db;
    if (hld_load_db.Init(LPSVRCFG->db_file_.c_str(), "r") < 0) {
        LOG(INFO) << "No Db File!" << endl;
        return -1;
    }
    set_is_loading(true);
    SSetPtr cur_sset = NULL;
    while (true) {
        int type = 0;
        if ((type = hld_load_db.LoadType()) < 0) {
            LOG(ERROR) << "Load Done" << type << std::endl;
            break;
        }

        //TODO 嵌套层数太多了，优化优化
        switch (type) {
            case bs_czl::MsgNodeType::VERSION: {
                bs_czl::MsgDbVersion obj;
                if (hld_load_db.LoadObj(obj) < 0)
                    break;
                LOG(INFO) << obj.ShortDebugString() << endl;
                break;
            }
            case bs_czl::MsgNodeType::DB: {
                bs_czl::MsgDb obj;
                if (hld_load_db.LoadObj(obj) < 0)
                    break;
                //暂时没有什么用，以后说不定有用呢
                LOG(INFO) << obj.ShortDebugString() << endl;
                break;
            }
            case bs_czl::MsgNodeType::TABLE: {
                bs_czl::MsgDbTable obj;
                if (hld_load_db.LoadObj(obj) < 0)
                    break;
                LOG(INFO) << obj.ShortDebugString() << endl;
                if ((cur_sset = FindSSet(obj.key_id())) == NULL) {
                    cur_sset = SSetPtr(new sort_set);
                    assert(cur_sset != NULL);
                    db_dict.insert(make_pair(obj.key_id(), cur_sset));
                }
                break;
            }
            case bs_czl::MsgNodeType::KEY: {
                bs_czl::MsgDbKey obj;
                if (hld_load_db.LoadObj(obj) < 0)
                    break;
                LOG(INFO) << obj.ShortDebugString() << endl;
                if (cur_sset == NULL) {
                    //db文件没有问题的话，这里是应该不会进来的！
                    LOG(ERROR) << "set table is null" << endl;
                    break;
                }
                for (int i = 0; i < obj.key_size() && i < obj.val_size(); ++i) {
                    cur_sset->Insert(obj.key(i), obj.val(i));
                }
                break;
            }
            default: {
                LOG(ERROR) << "Never Go Here!!" << std::endl;
                set_is_loading(false);
                return -1;
            }
        }
    }
    set_is_loading(false);
    return 0;
}

int db::BgSaveDb() {
    pid_t childpid;
    //long long start;

    if (LPSGLSVR->chilld_pid != -1) {
        LOG(WARNING) << LPSGLSVR->chilld_pid << " Child Process ing..." << endl;
        return -1;
    }

    //start = ustime();
    if ((childpid = fork()) == 0) {
        //child
        int retval = 0;
        //close listen sock;
        LPSGLSVR->CloseAllListenSockets();
        prctl(PR_SET_NAME, "server_dbsave", NULL, NULL, NULL);
        retval = SaveToFile();

        _exit((retval < 0 ? 1 : 0));
    } else {
        //parent
        if (childpid < 0) {
            LOG(ERROR) << "BgSave Fork Fail|" << strerror(errno) << endl;
            return -1;
        }
        LPSGLSVR->chilld_pid = childpid;
        LPSGLSVR->uiForkStartTime = time(NULL);
        return 0;
    }
    return 0;
}



//
// Created by czllo on 2017/5/4.
//

#ifndef SKIPLIST_SORT_SET_H
#define SKIPLIST_SORT_SET_H

#include <string>
#include <unordered_map>
#include <utility>
#include "skiplist.h"

class sort_set {
public:
    typedef SkipList<std::string> sl_type;
    typedef sl_type::iterator iterator;
    typedef std::pair<iterator, iterator> node_pair;
    typedef std::unordered_map<std::string, double> dict_type;
    typedef dict_type::iterator dict_itr;

    bool Insert(const double key, const std::string &data);
    //更新数据，无则添加
    bool Update(const double score, const std::string &data);
    bool DelMem(const std::string &data);
    bool GetMemScore(const std::string &data, double &score);
    bool GetMemRank(const std::string &data, unsigned long long &rank);
    node_pair GetMemsByScore(const double min, const double max);
    iterator GetMemsFirstByRank(const unsigned long long min, const unsigned long long max);
    iterator GetMemByRank(const unsigned long long rank);

    unsigned long long size() { return sl.Lenth(); };
    iterator begin() { return sl.being(); }
    iterator end() { return sl.end(); }

private:
    sl_type sl;
    dict_type dict;
};


#endif //SKIPLIST_SORT_SET_H

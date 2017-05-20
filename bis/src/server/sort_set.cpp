//
// Created by czllo on 2017/5/4.
//

#include "sort_set.h"

using namespace std;

bool sort_set::Insert(const double key, const std::string &data) {

    //data is exist not to insert.
    if (!dict.insert(make_pair(data, key)).second) {
        return false;
    }

    sl.Insert(key, data);

    return true;
}

bool sort_set::Update(const double score, const std::string &data) {
    dict_itr itr = dict.find(data);
    if (itr == dict.end()) {
        return Insert(score, data);
    } else if (itr->second != score) {
        DelMem(itr->first);
        return Insert(score, data);
    }
    return true;
}

bool sort_set::DelMem(const std::string &data) {
    dict_itr itr = dict.find(data);
    if (itr == dict.end()) {
        return false;
    }
    sl.Delete(itr->second, itr->first);
    dict.erase(itr);
    return true;
}

bool sort_set::GetMemScore(const std::string &data, double &score) {
    dict_itr itr =  dict.find(data);
    if (itr == dict.end()) {
        return false;
    }

    score = itr->second;

    return true;
}

bool sort_set::GetMemRank(const std::string &data, unsigned long long &rank) {
    dict_itr itr = dict.find(data);
    if (itr == dict.end())
        return false;

    rank = sl.GetRank(itr->second, itr->first);

    return true;
}

sort_set::node_pair sort_set::GetMemsByScore(const double min, const double max) {
    iterator begin = sl.FirstInRangeByScore(min, max);
    iterator end = sl.LastInRangeByScore(min, max);

    return make_pair(begin, end);
}

sort_set::iterator sort_set::GetMemsFirstByRank(const unsigned long long min, const unsigned long long max) {
    return sl.FirstInRangeByIdx(Range(min, max));
}

sort_set::iterator sort_set::GetMemByRank(const unsigned long long rank) {
    return GetMemsFirstByRank(rank, rank);
}

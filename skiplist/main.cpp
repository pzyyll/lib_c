#include <iostream>
#include <cstdlib>
#include <string>
#include <cstdio>
#include "src/sort_set.h"
#include <unordered_map>

using namespace std;

int main() {

    double score;
    sort_set sset;
    int iSize = 10;
    for (int i = 1; i < iSize; ++i) {
        sset.Insert(i, to_string(i));
    }

    sset.Insert(1, "w");
    sset.Insert(1, "az");
    sset.Insert(12, "1234");
    sset.Insert(12, "12345677");

    for (sort_set::iterator itr = sset.begin(); itr != sset.end(); ++itr) {
        cout << itr->score << "|" << itr->data.val << endl;
    }
    cout << "size :" << sset.size() << endl;
    sset.GetMemScore("33", score);
    cout << "33 score :" << score << endl;

    auto itr = sset.GetMemsFirstByRank(12, 44);
    if (itr != sset.end()) {
        cout << itr->score << "|" << itr->data.val << endl;
    }
    itr = sset.GetMemByRank(12);
    if (itr != sset.end()) {
        cout << itr->score << "|" << itr->data.val << endl;
    }
    itr = sset.GetMemByRank(500);
    if (itr != sset.end()) {
        cout << itr->score << "|" << itr->data.val << endl;
    }

    sset.Update(100, "41");

    sset.DelMem("44");
    cout << sset.size() << endl;

    auto pair = sset.GetMemsByScore(12, 123);
    for (itr = pair.first; itr != sset.end() && itr != pair.second; ++itr) {
        cout << itr->score << "|" << itr->data.val << endl;
    }
    if (itr != sset.end()) {
        cout << itr->score << "|" << itr->data.val << endl;
    }

    unsigned long long rank = 0;
    if (sset.GetMemRank("1", rank))
        cout << "rank : " << rank << endl;



    SkipList<string> sl_is;
    int max_size = 100000;
    for (int i = 1; i < max_size; ++i) {
        sl_is.Insert(i, to_string(i));
    }
    sl_is.Search(99999);

    return 0;
}
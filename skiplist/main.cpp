#include <iostream>
#include <cstdlib>
#include <string>
#include "src/skiplist.h"

using namespace std;

int main() {
    SkipList<string> sl;

    int iSize = 4000;
    for (int i = 1; i < iSize; ++i) {
        int r = i;//random() % 100;
        sl.Insert(r, to_string(r));
    }
    for (int i = 0; i < 400; ++i) {
        int r = random() % 4000;
        cout << sl.Search(r) << endl;
    }

    string str;
    if (str == "")
        cout << "yes" << endl;


    if (is_same<int, int>::value)
        cout << "yes" << endl;

    cout << sl.Lenth() << endl;
    cout << sl.GetRank(13, "13") << endl;
    cout << sl.GetRank(44, "44") << endl;
    cout << sl.GetRank(444, "444") << endl;
    cout << sl.GetRank(13, "14") << endl;


    return 0;
}
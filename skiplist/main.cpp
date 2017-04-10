#include <iostream>
#include <cstdlib>
#include <string>
#include "src/skiplist.h"

using namespace std;

int main() {
    SkipList sl;

    int iSize = 200;
    for (int i = 0; i < iSize; ++i) {
        int r = i;//random() % 100;
        if (sl.Search(r) == "")
            sl.insert(r, to_string(r));
    }

    cout << sl.Search(1) << endl;
    cout << sl.Search(12) << endl;
    cout << sl.Search(33) << endl;

    string str;
    if (str == "")
        cout << "yes" << endl;

    return 0;
}
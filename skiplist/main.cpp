#include <iostream>
#include <cstdlib>

#include "src/skiplist.h"

using namespace std;

int main() {

    vector<int> vint;
    cout << vint.size() << endl;
    vint.resize(5);
    cout << vint.size() << endl;
    cout << vint[2] << endl;


    SkipListNode::data_type data = {21, 0};
    cout << data.val << endl;

    return 0;
}
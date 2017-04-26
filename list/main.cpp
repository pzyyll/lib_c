#include <iostream>

#include "list.h"

using namespace std;
using namespace cz;

int main() {
    List<int > stN;
    stN.push(1);
    stN.push(2);
    stN.push(3);

    for (auto itr = stN.begin(); itr != stN.end(); ++itr) {
        cout << itr->val << endl;
    }

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
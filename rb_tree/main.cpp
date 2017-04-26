#include <iostream>

#include "src/rb_tree.h"

using namespace std;

int main() {
    RBNode node;
    node.data = "12345678901234567890";
    cout << (sizeof(node) * 50000000)/1024/1024 << endl;
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
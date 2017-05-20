#include <iostream>
#include <string>

#include "src/rb_tree.h"

using namespace std;

int main() {

    rb_tree<int, string> rb;

    int max_size = 1000000;
    for (int i = 0; i < max_size; ++i) {
        rb.Insert(i, to_string(i));
    }

    rb.Insert(1, "12");
    rb.Insert(3, "34");
    rb.Insert(9, "54");
    rb.Insert(5, "56");

    //rb.InorderWalk();

    cout << rb.get_high() << endl;
    cout << rb.get_size() << endl;
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
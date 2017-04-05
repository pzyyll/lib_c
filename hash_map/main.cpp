#include <iostream>
#include <cstdlib>
#include <memory>

using namespace std;

int main() {
    std::cout << "Hello, World!" << std::endl;

    int *p = NULL;
    cout << sizeof(*p) << endl;

    allocator<int> alloc;

    typedef typename allocator<double>::template rebind<int>::other new_allocc;

    new_allocc allocc;
    int *i = allocc.allocate(1);
    allocc.construct(i, 3);
    cout << *i << endl;
    return 0;
}
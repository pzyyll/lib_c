#include <iostream>
#include <cstdlib>
#include <memory>
#include <string>
using namespace std;

class A {
public:
    A(const char *pcstr) : str_(pcstr) { }

    string str_;
};

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

    A ab("abc");
    cout << ab.str_ << endl;

    equal_to<char *> a;
    cout << a("abc", "abcd") << endl;
    return 0;
}
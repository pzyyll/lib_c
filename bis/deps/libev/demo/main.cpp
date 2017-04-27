#include <iostream>
#include "ev++.h"
#include <unistd.h>

class Task {
    void operator ()() {
        std::cout << "call back for task" << std::endl;
    }
};

int main() {
    ev::loop_ref main_loop = ev::get_default_loop();
    ev::io ioev(main_loop);
    Task t1;
    ioev.set(&t1);
    ioev.start();

    ioev.start(STDIN_FILENO, EV_READ);

    main_loop.run();=

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
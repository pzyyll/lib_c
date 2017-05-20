#include <iostream>
#include "ev++.h"
#include <unistd.h>

struct Task {
    void operator ()(ev::io &w, int revents) {
        std::cout << "call back for task" << std::endl;
        std::cout << revents << std::endl;
    }
};

int main() {
    ev::loop_ref main_loop = ev::get_default_loop();
    ev::io ioev(main_loop);
    ev::sig sig_(main_loop);

    Task t1;
    ioev.set(&t1);
    ioev.start();

    ioev.start(STDIN_FILENO, EV_READ);

    main_loop.run();
    ioev.

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
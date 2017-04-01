#include <iostream>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>

using namespace std;

int main() {

    int fd = 0;
    if ((fd = open("m.tch", O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) < 0) {
        cout << strerror(errno) << endl;
        return 0;
    }

    //write(fd, "abc\n", strlen("abc\n"));
    //write(fd, &fd, sizeof(fd));
    ftruncate(fd, 32);

    struct stat st;
    fstat(fd, &st);
    cout << st.st_size << endl;
    cout << "end" << endl;

    int *ptr = (int *)mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (ptr == MAP_FAILED)
        cout << "failed" << endl;
    cout << ptr << endl;

    string str;
    str = "abcefg";

    memcpy(ptr, str.c_str(), str.length());
    //snprintf((char *)ptr, 8, "abcd\n");

    msync(ptr, st.st_size, MS_SYNC);

    cout << (void *)(-1) << endl;

    return 0;
}
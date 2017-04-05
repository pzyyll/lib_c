#include <iostream>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>

using namespace std;

struct SaveData {
    int key;
    //union {
        //void *ptr;
        unsigned  int a;
    //} val;
};

const int MAXIITEM = 1000000;
const int MAXNTHREADS = 10;
struct SShared {
    pthread_mutex_t mutex;
    int buff[MAXIITEM];
    int nput;
    int nval;
} sShared = {
        PTHREAD_MUTEX_INITIALIZER
};

void *run(void *) {
    for (;;) {
        pthread_mutex_lock(&sShared.mutex);
        if (sShared.nput >= MAXIITEM) {
            pthread_mutex_unlock(&sShared.mutex);
            return (NULL);
        }
        sShared.buff[sShared.nput++] = sShared.nval++;
        pthread_mutex_unlock(&sShared.mutex);
    }
}

int main() {

    int fd = 0;
    if ((fd = open("m.tch", O_RDWR | O_CREAT, S_IRWXU)) < 0) {
        cout << strerror(errno) << endl;
        return 0;
    }

    ftruncate(fd, 1024 << 2);

    struct stat st;
    fstat(fd, &st);
    cout << st.st_size << endl;
    cout << "end" << endl;

    char *ptr = (char *)mmap(NULL, 1024000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (ptr == MAP_FAILED)
        cout << "failed" << endl;
    cout << (void *)ptr << endl;

    cout << "==========" << endl;
    SaveData buf;
    memcpy(&buf, ptr, sizeof(SaveData));
    cout << buf.key << endl;
    cout << buf.a << endl;

    memcpy(&buf, ptr+sizeof(SaveData), sizeof(buf));
    cout << buf.key << endl;
    cout << buf.a << endl;
    cout << "==========" << endl;

    int offset = 0;

    SaveData ssa;
    ssa.key = 12;
    ssa.a = 13;

    memcpy(ptr, &ssa, sizeof(ssa));
    offset += sizeof(ssa);

    SaveData ssb;
    ssb.key = 1;
    ssb.a = 2;

    memcpy(ptr + offset, &ssb, sizeof(ssb));
    offset += sizeof(ssb);

    cout << ((SaveData *)ptr)->key;
    cout << offset << endl;

    msync(ptr, st.st_size, MS_SYNC);

    cout << sysconf(_SC_PAGESIZE) << endl;

    return 0;
}
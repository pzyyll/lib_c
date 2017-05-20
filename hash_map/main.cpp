#include <iostream>
#include <cstdlib>
#include <memory>
#include <string>
#include <map>
#include "hash_table.h"

using namespace std;

class A {
public:
    A(const char *pcstr) : str_(pcstr) { }

    string str_;
};

dict<int, nodes> NodeGroups;
unsigned long slot = 2048;

nodes GetNode(string key) {
    int key = MurmurHash(key);
    return NodeGroups[key % slot];

}

int MurmurHash(string str)
{
    const void *key = str.c_str();
    /* 'm' and 'r' are mixing constants generated offline.
      They're not really 'magic', they just happen to work well.  */
    uint32_t seed = dict_hash_function_seed;
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    int len = str.length();
    /* Initialize the hash to a 'random' value */
    uint32_t h = seed ^ len;

    /* Mix 4 bytes at a time into the hash */
    const unsigned char *data = (const unsigned char *)key;

    while(len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    /* Handle the last few bytes of the input array  */
    switch(len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0]; h *= m;
    };

    /* Do a few final mixes of the hash to ensure the last few
     * bytes are well-incorporated. */
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return (unsigned int)h;
}

}


int main() {

    HashTable<int, int> ht;
    ht.hset(1, 1);
    ht.hset(2, 3);

    int loop = 50000000;
    for (int i = 0; i < loop; ++i) {
        ht.hset(i, i);
    }

    int a;
    ht.hget(2, a);
    cout << a << endl;

    return 0;
}
//
// Created by p_zhilicai on 2017/4/5.
//

#ifndef HASH_MAP_HASH_TABLE_H
#define HASH_MAP_HASH_TABLE_H

#include <new>
#include <memory>
#include <cstddef>
#include <cstdint>

static const double LOADFACTOR = 0.72;
static const unsigned int INIBUCKETSIZE = 64;

class HashFunc {
public:
    static uint32_t dict_hash_function_seed = 5381;

    //MurmurHash2
    unsigned int operator ()(void *key, int len)  {
        /* 'm' and 'r' are mixing constants generated offline.
         They're not really 'magic', they just happen to work well.  */
        uint32_t seed = dict_hash_function_seed;
        const uint32_t m = 0x5bd1e995;
        const int r = 24;

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
};

template <typename Tp1, typename Tp2>
struct HTNode {
    typedef Tp1 key_type;
    typedef Tp2 value_type;

    key_type key;
    value_type val;
    struct HTNode *next;
};

template <typename Tp1, typename Tp2>
class HTable {
public:
    typedef Tp1 key_type;
    typedef Tp2 value_type;
    typedef HTNode<key_type, value_type> * htnode_pointer;
    typedef unsigned int size_type;

    HTable(size_type bucket_size = INIBUCKETSIZE)
            : bucket_size_(bucket_size),
              sizemask_(bucket_size - 1),
              size_used_(0),
              htable_(NULL) {
        htable_ = new(std::nothrow) htnode_pointer[bucket_size_];
    }
    ~HTable() {
        for (size_type i = 0; i < bucket_size_; ++i) {
            while (NULL != htable_[i]) {
                htnode_pointer itr = htable_[i];
                htable_[i] = itr->next;
                delete itr;
            }
        }
        delete [] htable_;
        htable_ = NULL;
    }

    int Resize(size_type size) {
        //bucket_size_ = size;
        //sizemask_ = size - 1;
        size_used_ = 0;
        htnode_pointer *new_htable_ = static_cast<htnode_pointer *>(realloc(htable_, size));
        for (int i = 0; i < bucket_size_; ++i) {
            new_htable_[i] = htable_[i];
            if (NULL != htable_[i])
                ++size_used_;
        }
        this->~HTable();
        bucket_size_ = size;
        sizemask_ = size - 1;
        htable_ = new_htable_;
    }

public:
    size_type bucket_size_;
    size_type sizemask_;
    size_type size_used_;
    htnode_pointer *htable_;
};

template <typename KeyType, typename ValType, typename HF = HashFunc,
           typename EqualKey = std::equal_to<KeyType>,
           typename Alloc = std::allocator<KeyType> >
class HashTable {
private:
    typedef typename Alloc::template rebind<HTNode<KeyType, ValType>>::other node_allocator;
public:
    typedef unsigned int size_t;
    typedef KeyType key_type;
    typedef ValType value_type;
    typedef HF hash_fun;
    typedef EqualKey equal_type;
    typedef HTable<key_type, value_type> htable;

    int hset(const key_type &key, const value_type &val) {

    }

    value_type &hget(const key_type &key);
    int hdel(const key_type &key);

private:
    bool isNeedToExpand() {
        return false;
    }
    size_t genHashKey(const key_type &key);
private:
    htable tables[2];

    hash_fun hash;
    node_allocator alloc;
    equal_type equal_key;
};

#endif //HASH_MAP_HASH_TABLE_H

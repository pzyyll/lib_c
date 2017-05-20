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
static const unsigned int INIBUCKETSIZE = 1 << 27;

class HashFunc {
public:
    const static uint32_t dict_hash_function_seed = 5381;

    //MurmurHash2
    unsigned int operator ()(const void *key, const int len1)  {
        /* 'm' and 'r' are mixing constants generated offline.
         They're not really 'magic', they just happen to work well.  */
        uint32_t seed = dict_hash_function_seed;
        const uint32_t m = 0x5bd1e995;
        const int r = 24;

        int len = len1;
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
        htnode_pointer *new_htable_ = new(std::nothrow) htnode_pointer[size];;
        for (int i = 0; i < bucket_size_; ++i) {
            new_htable_[i] = htable_[i];
            if (NULL != htable_[i])
                ++size_used_;
        }
        delete [] htable_;
        //this->~HTable();
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
    typedef HTNode<key_type, value_type> htnode;

    int hset(const key_type &key, const value_type &val) {
        if (isNeedToExpand()) {
            tables[0].Resize(tables[0].bucket_size_ << 1);//
        }
        size_t index = genHashKey(key);
        //htnode *pnode = new(std::nothrow) htnode;//alloc.allocate(1);
        htable *curr = tables;
        htnode *itr = curr->htable_[index];

        if (NULL == itr) tables[0].size_used_++;
        bool key_exsit = false;
        while (NULL != itr) {
            if (equal_key(itr->key, key)) {
                key_exsit = true;
                break;
            }
            itr = itr->next;
        }

        if (key_exsit) {
            itr->val = val;
        } else {
            itr = new(std::nothrow) htnode;
            itr->key = key;
            itr->val = val;
            itr->next = curr->htable_[index];
            curr->htable_[index] = itr;
        }
        return 0;
    }

    int hget(const key_type &key, value_type &val) {
        size_t index = genHashKey(key);
        //htnode *pnode = new(std::nothrow) htnode;//alloc.allocate(1);
        htable *curr = tables;
        htnode *itr = curr->htable_[index];

        while (NULL != itr) {
            if (equal_key(itr->key, key)) {
                val = itr->val;
                return 0;
            }
            itr = itr->next;
        }
        return -1;
    }
    int hdel(const key_type &key);

private:
    bool isNeedToExpand() {
        return ((tables[0].size_used_ * 1.0 / tables[0].bucket_size_) > 0.72);
    }
    size_t genHashKey(const key_type &key) {
        return (hash(&key, sizeof(key)) & tables[0].sizemask_);
    }
private:
    htable tables[2];

    hash_fun hash;
    node_allocator alloc;
    equal_type equal_key;
};

#endif //HASH_MAP_HASH_TABLE_H

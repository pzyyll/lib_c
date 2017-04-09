//
// Created by czllo on 2017/4/9.
//

#ifndef SKIPLIST_SKIPLIST_H
#define SKIPLIST_SKIPLIST_H

#include <ctime>
#include <cstdlib>

#include <vector>
//namespace czl {

static const float SKIPLIST_P = 0.25;
static const int MAXLEVEL = 32;

int skRandomLevel();

template <typename T>
struct Data {
    T val;
    unsigned lru;
};

template struct Data<int>;

struct SkipListNode {
    typedef Data<int> data_type;

    SkipListNode() {}
    SkipListNode(int level, double score, data_type &data) {
        this->score = score;
        this->data = data;
        this->level.resize(level);
    }
    ~SkipListNode() {}

    data_type data;
    double score;
    struct SkipListNode *backward;
    struct SkipListLevel {
        struct SkipListNode *forward;
        unsigned span;
    } /*level[]*/;
    std::vector<SkipListLevel> level;
};

class SkipList {
public:
    SkipList()
            : head_(NULL),
              tail_(NULL),
              lenth_(0),
              level_(1)
    {
        InitHead();
    };
    ~SkipList();

    int Search(int sorce) {

    }

private:
    void InitHead() {
        SkipListNode::data_type data = {0, 0};
        head_ = new(std::nothrow) SkipListNode(MAXLEVEL, -1, data);
        for (int i = 0; i < MAXLEVEL; ++i) {
            head_->level[i].forward = NULL;
            head_->level[i].span = 0;
        }
    }

private:
    SkipListNode *head_, *tail_;
    unsigned long lenth_;
    int level_;
};

//}
#endif //SKIPLIST_SKIPLIST_H

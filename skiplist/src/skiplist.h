//
// Created by czllo on 2017/4/9.
//

#ifndef SKIPLIST_SKIPLIST_H
#define SKIPLIST_SKIPLIST_H

#include <ctime>
#include <cstdlib>
#include <climits>

#include <string>
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

template struct Data<std::string>;

struct SkipListNode {
    typedef Data<std::string> data_type;

    SkipListNode() {}
    SkipListNode(int level, double score, data_type &data)
            : score(score), data(data) {
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
    typedef std::string data_type;

    SkipList()
            : head_(NULL),
              tail_(NULL),
              lenth_(0),
              level_(1)
    {
        InitHead();
    };
    ~SkipList() { };

    data_type Search(int sorce) {
        SkipListNode *x = head_;
        for (int i = level_ - 1; i >= 0; --i) {
            while (NULL != x->level[i].forward
                   && x->level[i].forward->score < sorce) {
                x = x->level[i].forward;
            }
        }
        x = x->level[0].forward;
        if (x != NULL && x->score == sorce) return x->data.val;
        else return "";
    }

    int insert(int sore, const std::string &val) {
        SkipListNode *update[MAXLEVEL], *x;
        int ranks[MAXLEVEL] = {0};

        x = head_;
        for (int i = level_ - 1; i >= 0; --i) {
            ranks[i] = i == (level_ - 1) ? 0 : ranks[i + 1];
            while (NULL != x->level[i].forward
                   && x->level[i].forward->score < sore) {
                ranks[i] += x->level[i].span;
                x = x->level[i].forward;
            }
            update[i] = x;
        }
        x = x->level[0].forward;
        if (NULL != x && x->score == sore) {
            x->data.val = val;
        } else {
            int lvl = skRandomLevel();
            if (lvl > level_) {
                for (int i = level_; i < lvl; ++i) {
                    ranks[i] = 0;
                    update[i] = head_;
                    update[i]->level[i].span = lenth_;
                }
                level_ = lvl;
            }
            SkipListNode::data_type data = {val, 0};
            x = new(std::nothrow) SkipListNode(lvl, sore, data);
            for (int i = 0; i < level_; ++i) {
                x->level[i].forward = update[i]->level[i].forward;
                update[i]->level[i].forward = x;

                x->level[i].span = update[i]->level[i].span - (ranks[0] - ranks[i]);
                update[i]->level[i].span = (ranks[0] - ranks[i]) + 1;
            }

            for (int i = lvl; i < level_; ++i) {
                update[i]->level[i].span++;
            }

            x->backward = (update[0] == head_ ) ? NULL : update[0];
            if (x->level[0].forward)
                x->level[0].forward->backward = x;
            else
                tail_ = x;
            ++lenth_;
        }
    }

    unsigned lenth() { return lenth_; }
private:
    void InitHead() {
        SkipListNode::data_type data = {"", 0};
        head_ = new(std::nothrow) SkipListNode(MAXLEVEL, -1, data);
        for (int i = 0; i < MAXLEVEL; ++i) {
            head_->level[i].forward = NULL;
            head_->level[i].span = 0;
        }
        head_->backward = NULL;
    }

private:
    SkipListNode *head_, *tail_;
    unsigned long lenth_;
    int level_;
};

//}
#endif //SKIPLIST_SKIPLIST_H

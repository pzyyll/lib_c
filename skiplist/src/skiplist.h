//
// Created by czllo on 2017/4/9.
//

#ifndef SKIPLIST_SKIPLIST_H
#define SKIPLIST_SKIPLIST_H

#include <string>
#include <vector>
#include <type_traits>
#include <memory>

#include <ctime>
#include <cstdlib>
#include <climits>

//namespace czl {

static const float SKIPLIST_P = 0.25;
static const int MAXLEVEL = 32;

int skRandomLevel();

template <typename T>
struct Data {
    T val;
    unsigned lru;
};

template <typename T>
struct SkipListNode {
    typedef T val_type;
    typedef Data<val_type> data_type;

    SkipListNode(int lvl, double score, const val_type &val, unsigned lru = 0)
            : score(score),
              backward(NULL) {
        data.val = val;
        data.lru = lru;
        level.resize(lvl);
    }
    ~SkipListNode() { }

    struct SkipListLevel {
        SkipListNode *forward;
        unsigned span;
    } /*level[]*/;

    data_type data;
    double score;
    SkipListNode *backward;
    std::vector<SkipListLevel> level;
};

template <typename T>
class DefaultValCmp {
public:
    bool operator()(const T &a, const T &b) {
        return (a == b);
    }
};

template struct SkipListNode<std::string>;

template <typename Tp, typename ValCmp = DefaultValCmp<Tp>, typename Alloc = std::allocator<Tp>>
class SkipList {
public:
    typedef std::string data_type;
    typedef SkipListNode<Tp> sl_node;
    typedef typename Alloc::template rebind<sl_node>::other node_alloc;

    SkipList()
            : head_(NULL),
              tail_(NULL),
              lenth_(0),
              level_(1)
    {
        InitHead();
    };
    ~SkipList() { };

    data_type Search(int score) {
        sl_node *x = head_;
        for (int i = level_ - 1; i >= 0; --i) {
            while (NULL != x->level[i].forward
                   && x->level[i].forward->score < score) {
                x = x->level[i].forward;
            }
        }
        x = x->level[0].forward;
        if (x != NULL && x->score == score) return x->data.val;
        else return "";
    }

    int Insert(int sore, const std::string &val) {
        sl_node *update[MAXLEVEL], *x;
        int ranks[MAXLEVEL] = {0};

        x = head_;
        for (int i = level_ - 1; i >= 0; --i) {
            ranks[i] = i == (level_ - 1) ? 0 : ranks[i + 1];
            while ((NULL != x->level[i].forward)
                   && (x->level[i].forward->score < sore)) {
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

            x = make_node(lvl, sore, val);
            for (int i = 0; i < lvl; ++i) {
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

    unsigned Lenth() { return lenth_; }

private:
    void InitHead() {
        head_ = make_node(MAXLEVEL, -1, "");
        for (int i = 0; i < MAXLEVEL; ++i) {
            head_->level[i].forward = NULL;
            head_->level[i].span = 0;
        }
        head_->backward = NULL;
    }

    sl_node *make_node(int lvl, double score, const std::string &val) {
        sl_node *pNode = alloc_.allocate(1);
        alloc_.construct(pNode, lvl, score, val);

        return pNode;
    }

private:
    sl_node *head_, *tail_;
    node_alloc alloc_;
    unsigned long lenth_;
    int level_;
};

//}
#endif //SKIPLIST_SKIPLIST_H

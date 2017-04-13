//
// Created by czllo on 2017/4/9.
//

#ifndef SKIPLIST_SKIPLIST_H
#define SKIPLIST_SKIPLIST_H

#include <string>
#include <vector>
#include <type_traits>
#include <memory>
#include <iostream>

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
    typedef Tp data_type;
    typedef SkipListNode<Tp> sl_node;
    typedef ValCmp cmp_type;
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
        //int cnt = 0;
        for (int i = level_ - 1; i >= 0; --i) {
            while (NULL != x->level[i].forward
                   && x->level[i].forward->score < score) {
                x = x->level[i].forward;
                //++cnt;
            }
            //++cnt;
        }
        //std::cout << "cnt:" << cnt << std::endl;
        x = x->level[0].forward;
        if (x != NULL && x->score == score) return x->data.val;
        else return "";
    }

    unsigned long GetRank(double score, const data_type &val) {
        unsigned long rank = 0;
        sl_node *x = head_;
        for (int i = level_ - 1; i >= 0; --i) {
            while (NULL != x->level[i].forward
                    && (x->level[i].forward->score < score
                        || (x->level[i].forward->score == score))) {
                rank += x->level[i].span;
                x = x->level[i].forward;
            }

            if (cmp_(x->data.val, val)) {
                return rank;
            }
        }
        return 0;
    }

    int Insert(int score, const data_type &val) {
        sl_node *update[MAXLEVEL], *x;
        int ranks[MAXLEVEL] = {0};

        x = head_;
        for (int i = level_ - 1; i >= 0; --i) {
            ranks[i] = i == (level_ - 1) ? 0 : ranks[i + 1];
            while (NULL != x->level[i].forward
                   && (x->level[i].forward->score < score
                       || (x->level[i].forward->score == score
                           && !cmp_(x->level[i].forward->data.val, val)))) {
                ranks[i] += x->level[i].span;
                x = x->level[i].forward;
            }
            update[i] = x;
        }

        //
        //分值可重复，需保证调用此函数时该成员不存在
        //
        int lvl = skRandomLevel();
        if (lvl > level_) {
            for (int i = level_; i < lvl; ++i) {
                ranks[i] = 0;
                update[i] = head_;
                update[i]->level[i].span = lenth_;
            }
            level_ = lvl;
        }

        x = MakeNode(lvl, score, val);
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

    int Delete(double score, const data_type &val) {
        sl_node *update[MAXLEVEL], *x;
        x = head_;
        for (int i = level_ - 1; i >= 0; --i) {
            while (NULL != x->level[i].forward
                   && (x->level[i].forward->score < score
                       || (x->level[i].forward->score == score
                           && !cmp_(x->level[i].forward->data.val, val)))) {
                x = x->level[i].forward;
            }
            update[i] = x;
        }
        x = x->level[0].forward;
        if (NULL != x && score == x->score && cmp_(x->data.val, val)) {
            DeleteNode(x, update);
            FreeNode(x);
            return 0;
        }

        return -1; //not found
    }

    unsigned Lenth() { return lenth_; }

private:
    void InitHead() {
        head_ = MakeNode(MAXLEVEL, -1, "");
        for (int i = 0; i < MAXLEVEL; ++i) {
            head_->level[i].forward = NULL;
            head_->level[i].span = 0;
        }
        head_->backward = NULL;
    }

    //节点脱离
    void DeleteNode(sl_node *x, sl_node *update[]) {
        for (int i = 0; i < level_; ++i) {
            if (update[i]->level[i].forward == x) {
                update[i]->level[i].span += x->level[i].span - 1;
                update[i]->level[i].forward = x->level[i].forward;
            } else {
                update[i]->level[i].span -= 1;
            }
        }

        if (NULL != x->level[0].forward) {
            x->level[0].forward->backward = x->backward;
        } else {
            tail_ = x->backward;
        }

        while (level_ > 1 && NULL == head_->level[level_-1].forward)
            --level_;
        ++lenth_;
    }

    sl_node *MakeNode(int lvl, double score, const std::string &val) {
        sl_node *pNode = alloc_.allocate(1);
        alloc_.construct(pNode, lvl, score, val);

        return pNode;
    }

    int FreeNode(sl_node *node) {
        alloc_.destroy(node);
        alloc_.deallocate(node, 1);
        node = NULL;
        return 0;
    }

private:
    sl_node *head_, *tail_;
    unsigned long lenth_;
    int level_;

    node_alloc alloc_;
    cmp_type cmp_;
};

//}
#endif //SKIPLIST_SKIPLIST_H

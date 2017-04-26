//
// Created by czllo on 2017/4/24.
//

#ifndef RB_TREE_RB_TREE_H
#define RB_TREE_RB_TREE_H

#include <string>

struct RBNode {
    typedef int key_type;
    typedef std::string data_type;
    typedef unsigned long size_t;

    enum {
        RED,
        BLACK,
    };

    key_type key;
    data_type data;
    size_t size;

    bool color;
    RBNode *p, *l, *r;
};

class rb_tree {
public:
    typedef int key_type;
    typedef std::string val_type;
    typedef unsigned long size_t;
    typedef RBNode node_type;

private:
    /*
    int Insert(node_type *z) {
        node_type *y = NULL;
        node_type *x = root_;

        while (NULL != x) {
            y = x;
            if (z->key < y->key)
                x = x->l;
            else
                x = x->r;
        }
    }
     */


    void left_rotate(node_type *x) {
        node_type *y = x->r;
        x->r = y->l;
        if (NULL != y->l) {
            y->l->p = x;
        }
        y->p = x->p;
        if (NULL == x->p)
            root_ = y;
        else if (x == x->p->l)
            x->p->l = y;
        else
            x->p->r = y;
        y->l = x;
        x->p = y;
    }
    void right_rotate(node_type *y) {
        node_type *x = y->l;
        y->l = x->r;
        if (NULL != x->r) {
            x->r->p = y;
        }
        x->p = y->p;
        if (NULL == y->p) {
            root_ = x;
        } else if (y == y->p->r)
            y->p->r = x;
        else
            y->p->l = x;
        x->r = y;
        y->p = x;
    }


    node_type *MakeNode(const key_type &key, const val_type &val) {
        node_type *pNew = new node_type;
        pNew->l = NULL;
        pNew->r = NULL;
        pNew->p = NULL;
        pNew->key = key;
        pNew->data = val;
        pNew->color = node_type::RED;
        pNew->size = 0;

        return pNew;
    }

private:
    node_type *root_;
    size_t size;
};


#endif //RB_TREE_RB_TREE_H

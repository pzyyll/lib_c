//
// Created by czllo on 2017/4/24.
//

#ifndef RB_TREE_RB_TREE_H
#define RB_TREE_RB_TREE_H

#include <string>
#include <functional>
#include <memory>
#include <cassert>

#include <iostream>

#define return_fail(x) if (!(x)) {return;}
#define return_val_fail(x, ret) if (!(x)) {return (ret);}

enum Color {
    RED,
    BLACK,
};

template <typename Key, typename T>
struct RBNode {
    typedef Key key_type;
    typedef T val_type;
    typedef unsigned long size_type;

    RBNode() : key(),
               val(),
               size(0),
               color(BLACK),
               p(NULL),
               left(NULL),
               right(NULL) { }
    ~RBNode() { }

    key_type key;
    val_type val;
    size_type size;

    Color color;
    RBNode *p, *left, *right;
};

template <typename Key, typename T, typename Compare = std::less<Key>, typename Allocator = std::allocator<T>>
class rb_tree {
public:
    typedef Key key_type;
    typedef T val_type;
    typedef unsigned long size_type;
    typedef RBNode<key_type, val_type> node_type;
    typedef node_type* node_pointer;
    typedef Compare key_compare;
    typedef typename Allocator::template rebind<node_type>::other alloc_type;

public:
    rb_tree() : root_(NULL), nil_(NULL), size_(0), high(0) {
        nil_ = MakeNode(key_type(), val_type());
        root_ = nil_;
    }
    ~rb_tree() {
        //todo:free all node
    }

    void Insert(const key_type &key, const val_type &val) {
        node_pointer z = MakeNode(key, val);
        return_fail(z);
        Insert(z);
        ++size_;
    }

    //@just return the first
    node_pointer Search(const key_type &key) {
        node_pointer x = root_;
        while (x != nil_ && x->key != key) {
            if (key_cmp_(key, x->key))
                x = x->left;
            else
                x = x->right;
        }
        return x;
    }

    void InorderWalk() {
        InorderWalk(root_);
    }

private:
    void InorderWalk(node_pointer x) {
        if (x != nil_) {
            InorderWalk(x->left);
            std::cout << x->key << ":" << x->val << std::endl;
            InorderWalk(x->right);
        }
    }

    node_pointer MakeNode(const key_type &key, const val_type &val) {
        node_pointer x = alloc_.allocate(1);
        assert(x != NULL);
        alloc_.construct(x);
        x->key = key;
        x->val = val;
        return x;
    }

    void LeftRorate(node_pointer x) {
        //make sure of x is not null
        return_fail(x);
        node_pointer y = x->right;
        return_fail(y);
        x->right = y->left;
        if (y->left != nil_)
            y->left->p = x;
        y->p = x->p;
        if (x->p == nil_)
            root_ = y;
        else if (x == x->p->left)
            x->p->left = y;
        else
            x->p->right = y;
        y->left = x;
        x->p = y;
    }

    void RightRorate(node_pointer y) {
        return_fail(y);
        node_pointer x = y->left;
        return_fail(x);
        y->left = x->right;
        if (x->right != nil_)
            x->right->p = y;
        x->p = y->p;
        if (y->p == nil_)
            root_ = x;
        else if (y == y->p->left)
            y->p->left = x;
        else
            y->p->right = x;
        x->right = y;
        y->p = x;
    }

    void Insert(node_pointer z) {
        //make sure of z is not null
        node_pointer y = nil_;
        node_pointer x = root_;

        size_type cnt = 0;
        while (x != nil_) {
            y = x;
            if (key_cmp_(z->key, x->key))
                x = x->left;
            else
                x = x->right;
            ++cnt;
        }
        if (cnt > high) high = cnt;
        z->p = y;
        if (y == nil_)
            root_ = z;
        else if (key_cmp_(z->key, y->key))
            y->left = z;
        else
            y->right = z;
        z->left = nil_;
        z->right = nil_;
        z->color = RED;
        //fix color
        InsertFixup(z);
    }

    void InsertFixup(node_pointer z) {
        //make sure of z is not null
        while (z->p->color == RED) {  //父节点为红，违反孩子节点必须为黑的规则
            if (z->p == z->p->p->left) {
                node_pointer y = z->p->p->right;      //找到叔节点
                if (y->color == RED) {                //情况1，叔节点红色
                    z->p->color = BLACK;              //将父节点，
                    y->color = BLACK;                 //叔节点染为黑色，
                    z->p->p->color = RED;             //父节点的父节点染为红色, 现在为红(z)-黑(父，叔)-红(爷)
                    z = z->p->p;                      //转移到爷节点（爷节点什么鬼）
                } else if (z == z->p->right) {        //情况2，叔节点黑色并且z是父亲右孩子
                    z = z->p;
                    LeftRorate(z);
                } else {                              //情况3，叔节点黑色并且z是父亲左孩子
                    z->p->color = BLACK;
                    z->p->p->color = RED;
                    RightRorate(z->p->p);
                }
            } else {                                  //与上面的对称
                node_pointer y = z->p->p->left;
                if (y->color == RED) {
                    z->p->color = BLACK;
                    y->color = BLACK;
                    z->p->p->color = RED;
                    z = z->p->p;
                } else if (z == z->p->left) {
                    z = z->p;
                    RightRorate(z);
                } else {
                    z->p->color = BLACK;
                    z->p->p->color = RED;
                    LeftRorate(z->p->p);
                }
            }
        }
        root_->color = BLACK;
    }

    node_pointer Max(node_pointer x) {
        while (x->right != nil_)
            x = x->right;
        return x;
    }

    node_pointer Min(node_pointer x) {
        while (x->left != nil_)
            x = x->left;
        return x;
    }

    void Transplant(node_pointer u, node_pointer v) {
        //make sure u and v are not null
        if (u->p == nil_)
            root_ = v;
        else if (u == u->p->left)
            u->p->left = v;
        else
            u->p->right = v;
        v->p = u->p;
    }

    void Delete(node_pointer z) {
        node_pointer y = z;
        node_pointer x = nil_;
        Color y_ori_color = y->color;
        if (z->left == nil_) {
            x = z->right;
            Transplant(z, z->right);
        } else if (z->right == nil_) {
            x = z->left;
            Transplant(z, z->left);
        } else {
            y = Min(z->right);
            y_ori_color = y->color;
            x = y->right;
            if (y->p == z) {
                x->p = y;
            } else {
                Transplant(y, y->right);
                y->right = z->right;
                y->right->p = y;
            }
            Transplant(z, y);
            y->left = z->left;
            y->left->p = y;
            y->color = z->color;
        }

        if (y_ori_color == BLACK)
            DeleteFixup(x);
    }

    void DeleteFixup(node_pointer x) {
        while (x != root_ && x->color == BLACK) {
            if (x == x->p->left) {
                node_pointer w = x->p->right;
                if ()
            }
        }
        x->color = BLACK;
    }

public:
    size_type get_high() { return high; }
    size_type get_size() { return size_; }
private:
    node_pointer root_;
    node_pointer nil_;
    size_type size_;

    alloc_type alloc_;
    key_compare key_cmp_;

    size_type high;
};


#endif //RB_TREE_RB_TREE_H

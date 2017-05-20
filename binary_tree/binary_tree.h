//
// Created by czllo on 2017/5/19.
//

#ifndef BINARY_TREE_BINARY_TREE_H
#define BINARY_TREE_BINARY_TREE_H

#include <iostream>
#include <string>
#include <cassert>

template <typename T>
struct Node {
    typedef T data_type;

    Node() : data(), left(NULL), right(NULL), parent(NULL) { }
    ~Node() { }

    data_type data;
    struct Node *left, *right, *parent;
};

class BinaryTree {
public:
    typedef int   data_type;
    typedef struct Node<data_type>   node_type;
    typedef node_type*  node_pointer;

    BinaryTree() : root(NULL) { }
    ~BinaryTree() { }

    void Insert(const data_type &data) {
        node_pointer new_node = MakeNode(data);
        assert(new_node != NULL);
        Insert(new_node);
    }

    node_pointer Search(const data_type &data) {
        node_pointer x = root;
        while (x != NULL && x->data != data) {
            if (data < x->data)
                x = x->left;
            else
                x = x->right;
        }
        return x;

//        if (x == NULL || data == x->data)
//            return x;
//        if (data < x->data)
//            return Search(x->left);
//        else
//            return Search(x->right);
    }

    void Delete(const data_type &data) {
        node_pointer x = Search(data);
        if (x)
            Delete(x);
        FreeNode(x);
    }

    node_pointer Max() {
        if (root)
            return Max(root);
        return NULL;
    }
    node_pointer Min() {
        if (root)
            return Min(root);
        return NULL;
    }

    void InorderWalk() {
        InorderWalk(root);
    }

private:
    node_pointer MakeNode(const data_type &data) {
        node_pointer node = new node_type;
        assert(node != NULL);
        node->data = data;
        return node;
    }
    void FreeNode(node_pointer x) {
        if (x)
            delete x;
    }

    void InorderWalk(node_pointer x) {
        if (x) {
            InorderWalk(x->left);
            std::cout << x->data << std::endl;
            InorderWalk(x->right);
        }
    }

    node_pointer Max(node_pointer x) {
        assert(NULL != x);
        while (NULL != x->right) {
            x = x->right;
        }
        return x;
    }

    node_pointer Min(node_pointer x) {
        assert(NULL != x);
        while (NULL != x->left) {
            x = x->left;
        }
        return x;
    }

    node_pointer Pre(node_pointer z) {
        if (NULL == z)
            return NULL;

        if (NULL != z->left)
            return Max(z->left);
        node_pointer y = z->parent;
        while (NULL != y && y->left == z) {
            z = y;
            y = y->parent;
        }
        return y;
    }

    node_pointer Next(node_pointer z) {
        if (NULL == z)
            return NULL;

        if (NULL != z->right)
            return Min(z->right);
        node_pointer y = z->parent;
        while(NULL != y && y->right == z) {
            z = y;
            y = y->parent;
        }
        return y;
    }

    void Insert(node_pointer z) {
        node_pointer x = root;
        node_pointer y = NULL;
        while (NULL != x) {
            y = x;
            (z->data < x->data) ? (x = x->left) : (x = x->right);
        }
        z->parent = y;
        if (y == NULL) {
            root = z;
        } else if (z->data > y->data) {
            y->right = z;
        } else {
            y->left = z;
        }
    }
    void Delete(node_pointer z) {
        assert(z != NULL);

        if (NULL == z->left) {
            Transplant(z, z->right);
        } else if (NULL == z->right) {
            Transplant(z, z->left);
        } else {
            node_pointer y = Min(z->right);
            if (y->parent != z) {
                Transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            Transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
        }
    }

    void Transplant(node_pointer u, node_pointer v) {
        assert(u != NULL);
        //v can be nil
        if (NULL == u->parent)
            root = v;
        else if (u->parent->left == u)
            u->parent->left = v;
        else
            u->parent->right = v;
        if (NULL != v)
            v->parent = u->parent;
    }

private:
    node_pointer root;
};

#endif //BINARY_TREE_BINARY_TREE_H
